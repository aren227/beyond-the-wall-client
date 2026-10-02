#pragma once

namespace aren {

struct AudioPlayParams {
    float volume = 1.0f;
    float pitch = 1.0f;
    float pan = 0.5f;
};

class Audio {
public:
    static void init();
    static void deinit();
    static void update();

    static void registerSfxVariants(int id, const std::vector<std::string> &paths);
    static void registerMusic(const std::string &path);

    static void play(int id, const AudioPlayParams &params = {});

    static void setMusicVolume(float volume);
    static void setSfxVolume(float volume);
};

} // namespace aren
