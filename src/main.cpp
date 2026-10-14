#include "./lib/Engine.hpp"

int main() {
    Engine& engine = Engine::Get();

    EngineConfig config;
    config.title = "Demo";
    config.mode = Windowed{800, 600}; // albo Fullscreen{}

    if (!engine.Init(config)) return 1;

    Vector2 player{400, 300};

    engine.Run(
        [&](float dt) {
            if (engine.KeyDown(KEY_RIGHT)) player.x += 200 * dt;
            if (engine.KeyDown(KEY_LEFT)) player.x -= 200 * dt;
            if (engine.KeyDown(KEY_DOWN)) player.y += 200 * dt;
            if (engine.KeyDown(KEY_UP)) player.y -= 200 * dt;
            if (engine.MouseDown(MOUSE_BUTTON_LEFT)) player = engine.MousePosition();
        },
        [&]() {
            engine.Clear(DARKGRAY);
            DrawCircleV(player, 20, ORANGE);
        });

    engine.Shutdown();
    return 0;
}
