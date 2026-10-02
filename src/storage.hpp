#pragma once

namespace aren {

class Storage {
public:
    static void init();
    static void deinit();

    static void setString(const char *key, const std::string &value);
    static void setInt(const char *key, int value);
    static void setDouble(const char *key, double value);
    static void setBool(const char *key, bool value);
    static void setList(const char *key, const std::vector<std::string> &value);

    static std::string getString(const char *key);
    static int getInt(const char *key, int defaultValue = 0);
    static double getDouble(const char *key, double defaultValue = 0.0);
    static bool getBool(const char *key, bool defaultValue = false);
    static std::vector<std::string> getList(const char *key);
};

} // namespace aren
