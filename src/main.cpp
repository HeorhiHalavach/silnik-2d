#include "lib/Engine.hpp"

// Sceny rejestrują się same - patrz SCENE(...) w src/scenes/*.cpp
int main() {
    Engine& engine = Engine::Get()
                         .Title("Lab 2 - prymitywy")
                         .Window(1000, 700) // albo .Fullscreen()
                         .Background(RAYWHITE)
                         .StartScene("menu")
                         .Init();

    if (!engine.Ready()) return 1;

    // logika globalna: działa w każdej scenie
    return engine
        .OnUpdate([&](float) {
            if (engine.KeyPressed(KEY_ONE)) engine.SwitchScene("menu");
            if (engine.KeyPressed(KEY_TWO)) engine.SwitchScene("prymitywy");
        })
        .OnDraw([&](PrimitiveRenderer&) {
            ::DrawText(("Scena: " + engine.ActiveScene() + "   [1] menu  [2] prymitywy").c_str(),
                       20, 665, 20, DARKGRAY);
        })
        .Run();
}
