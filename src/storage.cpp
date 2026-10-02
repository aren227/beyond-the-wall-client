#include "storage.hpp"
#include "raylib.h"
#include <cstdio>
#include <cstdlib>

#ifdef PLATFORM_WEB
#include <emscripten.h>
#endif

namespace aren {

enum class StorageType {
    String,
    Int,
    Double,
    Bool,
    List
};

#ifdef PLATFORM_WEB
constexpr const char *STORAGE_KEY = PROJECT_NAME ".save";

EM_JS(char *, web_storage_get, (const char *key), {
    var v = localStorage.getItem(UTF8ToString(key));
    if (v === null) return 0;
    var n = lengthBytesUTF8(v) + 1;
    var p = _malloc(n);
    stringToUTF8(v, p, n);
    return p;
});

EM_JS(void, web_storage_set, (const char *key, const char *text), {
    localStorage.setItem(UTF8ToString(key), UTF8ToString(text));
});

static char *persistentLoad() {
    return web_storage_get(STORAGE_KEY);
}

static void persistentUnload(char *t) {
    free(t);
}

static void persistentSave(const char *t) {
    web_storage_set(STORAGE_KEY, t);
}
#else

constexpr const char *STORAGE_PATH = PROJECT_NAME ".dat";

static char *persistentLoad() {
    return LoadFileText(STORAGE_PATH);
}

static void persistentUnload(char *t) {
    UnloadFileText(t);
}

static void persistentSave(const char *t) {
    SaveFileText(STORAGE_PATH, (char *)t);
}
#endif

struct Entry {
    StorageType type;
    std::string s;
    int i = 0;
    double d = 0.0;
    bool b = false;
    std::vector<std::string> l;

    Entry() = default;
    explicit Entry(const std::string &v) : type(StorageType::String), s(v) {}
    explicit Entry(int v) : type(StorageType::Int), i(v) {}
    explicit Entry(double v) : type(StorageType::Double), d(v) {}
    explicit Entry(bool v) : type(StorageType::Bool), b(v) {}
    explicit Entry(const std::vector<std::string> &v) : type(StorageType::List), l(v) {}
};

static std::unordered_map<std::string, Entry> s_entries;

static std::string escape(const std::string_view &s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        switch (c) {
            case '\\':
                out += "\\\\";
                break;
            case '\n':
                out += "\\n";
                break;
            case '\r':
                out += "\\r";
                break;
            // XXX(aren): comma escaping is not necessary for string values, but for list values.
            case ',':
                out += "\\,";
                break;
            default:
                out += c;
                break;
        }
    }
    return out;
}

static std::string unescape(const std::string_view &s) {
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); i++) {
        char c = s[i];
        if (c == '\\' && i + 1 < s.size()) {
            switch (s[i + 1]) {
                case '\\':
                    out += '\\';
                    i++;
                    break;
                case 'n':
                    out += '\n';
                    i++;
                    break;
                case 'r':
                    out += '\r';
                    i++;
                    break;
                case ',':
                    out += ',';
                    i++;
                    break;
                default:
                    ASSERT(false);
                    i++;
                    break;
            }
        } else {
            out += c;
        }
    }
    return out;
}

static std::vector<std::string> splitList(const std::string_view &s) {
    std::vector<std::string> out;
    if (s.empty()) return out;

    size_t prev = 0;
    size_t idx = 0;
    while (idx < s.size()) {
        if ((idx == 0 || s[idx-1] != '\\') && s[idx] == ',') {
            out.push_back(unescape(s.substr(prev, idx - prev)));
            prev = ++idx;
        } else {
            ++idx;
        }
    }
    out.push_back(unescape(s.substr(prev, s.size() - prev)));

    return out;
}

static std::string joinList(const std::vector<std::string> &v) {
    std::string s;

    for (size_t i = 0; i < v.size(); i++) {
        s.append(escape(v[i]));
        if (i + 1 < v.size()) {
            s.append(",");
        }
    }

    return s;
}

// Parses "key=(type)value". Returns false on malformed input — caller skips.
// static bool parseLine(const std::string &line, std::string &key, std::string &type, std::string &value) {
//     size_t eq = line.find('=');
//     if (eq == std::string::npos) return false;
//     if (eq + 1 >= line.size() || line[eq + 1] != '(') return false;
//     size_t rp = line.find(')', eq + 2);
//     if (rp == std::string::npos) return false;
//     key   = line.substr(0, eq);
//     type  = line.substr(eq + 2, rp - eq - 2);
//     value = line.substr(rp + 1);
//     return true;
// }

static bool parse(std::string_view &feed, std::string &key, char &type, std::string &value) {
    size_t eq = feed.find('=');
    if (eq == std::string::npos) return false;
    if (eq == 0) return false;
    if (feed[eq - 1] == '\\') return false;
    if (eq + 4 >= feed.size()) return false;
    if (feed[eq + 1] != '(' || feed[eq + 3] != ')') return false;
    size_t newl = feed.find('\n', eq + 4);
    if (newl == std::string::npos) return false;

    key = std::string(feed.substr(0, eq));
    type = feed[eq + 2];
    value = std::string(feed.substr(eq + 4, newl - (eq+4)));

    feed = feed.substr(newl+1, feed.size() - (newl+1));

    return true;
}

static void flush() {
    std::string out;
    out.reserve(s_entries.size() * 32);
    char numbuf[32];

    for (const auto &kv : s_entries) {
        out += kv.first;
        out += '=';
        const Entry &e = kv.second;
        switch (e.type) {
            case StorageType::String:
                out += "(s)";
                out += escape(e.s);
                break;
            case StorageType::Int:
                std::snprintf(numbuf, sizeof(numbuf), "%d", e.i);
                out += "(i)";
                out += numbuf;
                break;
            case StorageType::Double:
                std::snprintf(numbuf, sizeof(numbuf), "%.17g", e.d);
                out += "(d)";
                out += numbuf;
                break;
            case StorageType::Bool:
                out += "(b)";
                out += e.b ? "true" : "false";
                break;
            case StorageType::List:
                out += "(l)";
                out += joinList(e.l);
                break;
        }
        out += '\n';
    }

    persistentSave(out.empty() ? "" : out.c_str());
}

void Storage::init() {
    s_entries.clear();

    char *text = persistentLoad();
    if (!text) return;
    std::string content(text);
    persistentUnload(text);

    std::string_view view = content;

    std::string key;
    char type;
    std::string value;
    while (parse(view, key, type, value)) {
        printf("LOAD %s %c %s\n", key.c_str(), type, value.c_str());
        // std::cout << key << ' ' << type << ' ' << value << std::endl;
        Entry e;
        if (type == 's') {
            e = Entry(value);
        } else if (type == 'i') {
            e = Entry(int(std::strtol(value.c_str(), nullptr, 10)));
        } else if (type == 'd') {
            e = Entry(double(std::strtod(value.c_str(), nullptr)));
        } else if (type == 'b') {
            e = Entry(bool(value == "true"));
        } else if (type == 'l') {
            e = Entry(splitList(value));
        } else {
            ASSERTF(false, "Unsupported type: %c", type);
            continue;
        }
        s_entries[std::move(key)] = std::move(e);
    }
}

void Storage::deinit() {
    s_entries.clear();
}

void Storage::setString(const char *key, const std::string &value) {
    auto it = s_entries.find(key);
    if (it != s_entries.end() && it->second.type == StorageType::String && it->second.s == value) return;
    s_entries[key] = Entry(value);
    flush();
}

void Storage::setInt(const char *key, int value) {
    auto it = s_entries.find(key);
    if (it != s_entries.end() && it->second.type == StorageType::Int && it->second.i == value) return;
    s_entries[key] = Entry(value);
    flush();
}

void Storage::setDouble(const char *key, double value) {
    auto it = s_entries.find(key);
    if (it != s_entries.end() && it->second.type == StorageType::Double && it->second.d == value) return;
    s_entries[key] = Entry(value);
    flush();
}

void Storage::setBool(const char *key, bool value) {
    auto it = s_entries.find(key);
    if (it != s_entries.end() && it->second.type == StorageType::Bool && it->second.b == value) return;
    s_entries[key] = Entry(value);
    flush();
}

void Storage::setList(const char *key, const std::vector<std::string> &value) {
    auto it = s_entries.find(key);
    if (it != s_entries.end() && it->second.type == StorageType::List && it->second.l == value) return;
    s_entries[key] = Entry(value);
    flush();
}

std::string Storage::getString(const char *key) {
    auto it = s_entries.find(key);
    if (it == s_entries.end() || it->second.type != StorageType::String) return "";
    return it->second.s;
}

int Storage::getInt(const char *key, int defaultValue) {
    auto it = s_entries.find(key);
    if (it == s_entries.end() || it->second.type != StorageType::Int) return defaultValue;
    return it->second.i;
}

double Storage::getDouble(const char *key, double defaultValue) {
    auto it = s_entries.find(key);
    if (it == s_entries.end() || it->second.type != StorageType::Double) return defaultValue;
    return it->second.d;
}

bool Storage::getBool(const char *key, bool defaultValue) {
    auto it = s_entries.find(key);
    if (it == s_entries.end() || it->second.type != StorageType::Bool) return defaultValue;
    return it->second.b;
}

std::vector<std::string> Storage::getList(const char *key) {
    auto it = s_entries.find(key);
    if (it == s_entries.end() || it->second.type != StorageType::List) return {};
    return it->second.l;
}

} // namespace aren
