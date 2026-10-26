#include <string>

#include "lib/core/Engine.hpp"

// Sceny rejestrują się same - patrz SCENE(...) w src/scenes/*.cpp
int main() {
    Engine& engine = Engine::Get()
                         .Title("Silnik 2D - lab 3")
                         .VSync(false)           // podwójne buforowanie z synchronizacją pionową
                         .Window(1000, 700) // albo .Fullscreen()
                         .Background(RAYWHITE)
                         .StartScene("menu")
                         .Init();

    if (!engine.Ready()) return 1;

    return engine
        .OnUpdate([&](float) {
            if (engine.KeyPressed(KEY_ONE)) engine.SwitchScene("menu");
            if (engine.KeyPressed(KEY_TWO)) engine.SwitchScene("prymitywy");
            if (engine.KeyPressed(KEY_THREE)) engine.SwitchScene("figury");
        })
        .OnDraw([&](PrimitiveRenderer&) {
            const std::string hud =
                "Scena: " + engine.ActiveScene() + "   [1] menu  [2] prymitywy  [3] figury";
            ::DrawText(hud.c_str(), 20, 665, 20, DARKGRAY);
        })
        .Run();
}
