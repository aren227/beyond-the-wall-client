#include "audio.hpp"
#include "pch.hpp"
#include "random.hpp"

namespace aren {

constexpr int POLY_POOL_SIZE = 4;

struct SfxVariant {
    Sound base = {};
    std::array<Sound, POLY_POOL_SIZE> pool;
    int nextIndex = 0;
};

struct SfxSlot {
    std::vector<SfxVariant> variants;
    int lastIdx = -1;
};

static std::unordered_map<int, SfxSlot> s_sfxSlotMap;

static Music s_music = {};
static bool s_musicLoaded = false;

static float s_musicVolume = 1.0f;
static float s_sfxVolume = 1.0f;

void Audio::init() {
    InitAudioDevice();
}

void Audio::update() {
    if (s_musicLoaded) UpdateMusicStream(s_music);
}

void Audio::registerSfxVariants(int id, const std::vector<std::string> &paths) {
    ASSERT(s_sfxSlotMap.count(id) == 0);
    ASSERT(paths.size() > 0);

    SfxSlot slot;
    slot.variants.reserve(paths.size());
    for (const auto &path : paths) {
        SfxVariant v;
        v.base = LoadSound(path.c_str());
        for (int i = 0; i < POLY_POOL_SIZE; i++) {
            v.pool[i] = LoadSoundAlias(v.base);
        }
        slot.variants.push_back(std::move(v));
    }
    s_sfxSlotMap[id] = std::move(slot);
}

void Audio::registerMusic(const std::string &path) {
    s_music = LoadMusicStream(path.c_str());
    s_music.looping = true;
    s_musicLoaded = true;
    SetMusicVolume(s_music, s_musicVolume);
    PlayMusicStream(s_music);
}

void Audio::setMusicVolume(float volume) {
    s_musicVolume = volume;
    if (s_musicLoaded) SetMusicVolume(s_music, volume);
}

void Audio::setSfxVolume(float volume) {
    s_sfxVolume = volume;
}

void Audio::deinit() {
    if (s_musicLoaded) {
        UnloadMusicStream(s_music);
        s_musicLoaded = false;
    }
    for (auto &[_, slot] : s_sfxSlotMap) {
        for (auto &v : slot.variants) {
            for (auto &a : v.pool) UnloadSoundAlias(a);
            UnloadSound(v.base);
        }
        slot.variants.clear();
    }
    CloseAudioDevice();
}

void Audio::play(int id, const AudioPlayParams &p) {
    auto it = s_sfxSlotMap.find(id);
    if (it == s_sfxSlotMap.end()) {
        ASSERT(false);
        return;
    }

    auto &slot = it->second;
    if (slot.variants.empty()) {
        ASSERT(false);
        return;
    }

    int n = (int)slot.variants.size();
    int next = 0;
    if (n > 1) {
        next = (slot.lastIdx + 1 + Random::rangeInt(0, n - 1)) % n;
    }
    slot.lastIdx = next;
    auto &var = slot.variants[next];

    auto &sound = var.pool[var.nextIndex];
    var.nextIndex = (var.nextIndex + 1) % POLY_POOL_SIZE;

    SetSoundVolume(sound, p.volume * s_sfxVolume);
    SetSoundPitch(sound, p.pitch);
    SetSoundPan(sound, p.pan);
    PlaySound(sound);
}

} // namespace aren
