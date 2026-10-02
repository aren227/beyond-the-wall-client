#pragma once

namespace aren {

class Settings {
public:
    static void init();

    static float getBgmVolume();
    static float getSfxVolume();
    static bool getCameraShake();

    static void setBgmVolume(float v);
    static void setSfxVolume(float v);
    static void setCameraShake(bool v);
};

} // namespace aren
