#pragma once

namespace aren {

enum class Sfx {
    Beep,
    Explosion,
    Hit,
    Shoot,
    Melee,
    PowerUp,
    Dash,
};

class GameAudio {
public:
    static void init();
    static void play(Sfx sfx);
    static void laser(float volume);
    static void update();
};

} // namespace aren