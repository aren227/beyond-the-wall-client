#include "input.hpp"

namespace aren {

Vector2f Input::mousePosition() {
    return GetMousePosition();
}

Vector2f Input::mouseDelta() {
    return GetMouseDelta();
}

float Input::mouseWheel() {
    return GetMouseWheelMove();
}

bool Input::mouseButtonPressed(int button) {
    return IsMouseButtonPressed(button);
}

bool Input::mouseButtonDown(int button) {
    return IsMouseButtonDown(button);
}

bool Input::mouseButtonReleased(int button) {
    return IsMouseButtonReleased(button);
}

bool Input::keyPressed(int key) {
    return IsKeyPressed(key);
}

bool Input::keyDown(int key) {
    return IsKeyDown(key);
}

bool Input::keyReleased(int key) {
    return IsKeyReleased(key);
}

} // namespace aren
