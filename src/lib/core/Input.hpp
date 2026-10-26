#pragma once

#include <raylib.h>

// Stan klawiatury i myszy. Gdy urządzenie jest wyłączone, zapytania zwracają false / {0, 0}.
class Input {
  public:
    Input& Keyboard(bool value = true) {
        keyboard_ = value;
        return *this;
    }
    Input& Mouse(bool value = true) {
        mouse_ = value;
        return *this;
    }
    bool KeyboardEnabled() const { return keyboard_; }
    bool MouseEnabled() const { return mouse_; }

    bool KeyDown(int key) const { return keyboard_ && ::IsKeyDown(key); }
    bool KeyPressed(int key) const { return keyboard_ && ::IsKeyPressed(key); }
    bool MouseDown(int button) const { return mouse_ && ::IsMouseButtonDown(button); }
    bool MousePressed(int button) const { return mouse_ && ::IsMouseButtonPressed(button); }
    Vector2 MousePosition() const { return mouse_ ? ::GetMousePosition() : Vector2{0, 0}; }

  private:
    bool keyboard_ = true;
    bool mouse_ = true;
};
