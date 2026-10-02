#pragma once

namespace aren {

class Input {
public:
    static Vector2f mousePosition();
    static Vector2f mouseDelta();
    static float mouseWheel();
    static bool mouseButtonPressed(int button);
    static bool mouseButtonDown(int button);
    static bool mouseButtonReleased(int button);

    static bool keyPressed(int key);
    static bool keyDown(int key);
    static bool keyReleased(int key);
};

} // namespace aren
