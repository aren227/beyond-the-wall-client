#include "settings.hpp"
#include "audio.hpp"
#include "storage.hpp"

namespace aren {

static float s_bgmVolume;
static float s_sfxVolume;
static bool s_cameraShake;

constexpr const char *BGM_VOLUME_KEY = "settings_bgmVolume";
constexpr const char *SFX_VOLUME_KEY = "settings_sfxVolume";
constexpr const char *CAMERA_SHAKE_KEY = "settings_cameraShake";

void Settings::init() {
    s_bgmVolume = float(Storage::getDouble(BGM_VOLUME_KEY, 0.5));
    s_sfxVolume = float(Storage::getDouble(SFX_VOLUME_KEY, 0.5));
    s_cameraShake = Storage::getBool(CAMERA_SHAKE_KEY, true);

    Audio::setMusicVolume(s_bgmVolume);
    Audio::setSfxVolume(s_sfxVolume);
}

float Settings::getBgmVolume() { return s_bgmVolume; }
float Settings::getSfxVolume() { return s_sfxVolume; }
bool Settings::getCameraShake() { return s_cameraShake; }

void Settings::setBgmVolume(float v) {
    s_bgmVolume = v;
    Storage::setDouble(BGM_VOLUME_KEY, v);
    Audio::setMusicVolume(v);
}

void Settings::setSfxVolume(float v) {
    s_sfxVolume = v;
    Storage::setDouble(SFX_VOLUME_KEY, v);
    Audio::setSfxVolume(v);
}

void Settings::setCameraShake(bool v) {
    s_cameraShake = v;
    Storage::setBool(CAMERA_SHAKE_KEY, v);
}

} // namespace aren
