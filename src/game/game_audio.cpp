#include "game_audio.hpp"
#include "consts.hpp"
#include "../audio.hpp"

namespace aren {

static ::Music s_laserLoop = {};

void GameAudio::init() {
    Audio::registerMusic(SOUND_BASE + "/974745_The-Cairn.mp3");

    s_laserLoop = LoadMusicStream((SOUND_BASE + "/laser2.ogg").c_str());
    s_laserLoop.looping = true;
    SetMusicVolume(s_laserLoop, 0.0f);
    PlayMusicStream(s_laserLoop);

    Audio::registerSfxVariants(int(Sfx::Beep), {
        SOUND_BASE + "/blipSelect.wav"
    });
    Audio::registerSfxVariants(int(Sfx::Explosion), {
        SOUND_BASE + "/explosion_0.wav",
        SOUND_BASE + "/explosion_1.wav",
        SOUND_BASE + "/explosion_2.wav",
    });
    Audio::registerSfxVariants(int(Sfx::Hit), {
        SOUND_BASE + "/hitHurt.wav"
    });
    Audio::registerSfxVariants(int(Sfx::Shoot), {
        SOUND_BASE + "/shoot.wav"
    });
    Audio::registerSfxVariants(int(Sfx::Melee), {
        SOUND_BASE + "/melee.wav"
    });
    Audio::registerSfxVariants(int(Sfx::PowerUp), {
        SOUND_BASE + "/powerUp.wav"
    });
    Audio::registerSfxVariants(int(Sfx::Dash), {
        SOUND_BASE + "/dash.wav"
    });
}

void GameAudio::play(Sfx sfx) {
    Audio::play(int(sfx), {});
}

void GameAudio::laser(float volume) {
    SetMusicVolume(s_laserLoop, volume * Audio::getSfxVolume());
}

void GameAudio::update() {
    UpdateMusicStream(s_laserLoop);
}

}