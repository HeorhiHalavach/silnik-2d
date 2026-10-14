#include "Engine.hpp"

#include <iostream>

bool Engine::Init(const EngineConfig& config) {
    cfg_ = config;
    if (!cfg_.logFile.empty()) log_.open(cfg_.logFile);

    if (auto* w = std::get_if<Windowed>(&cfg_.mode)) {
        ::InitWindow(w->width, w->height, cfg_.title.c_str());
    } else {
        ::SetConfigFlags(FLAG_FULLSCREEN_MODE);
        ::InitWindow(0, 0, cfg_.title.c_str());
    }

    if (!::IsWindowReady()) {
        Log("BLAD: nie udalo sie utworzyc okna");
        return false;
    }

    ::SetTargetFPS(cfg_.fps);
    initialized_ = true;
    Log("Silnik zainicjalizowany");
    return true;
}

void Engine::Run(const std::function<void(float)>& update, const std::function<void()>& draw) {
    while (!::WindowShouldClose()) {
        update(::GetFrameTime());
        ::BeginDrawing();
        draw();
        ::EndDrawing();
    }
}

void Engine::Shutdown() {
    if (!initialized_) return;
    Log("Zamykanie silnika");
    ::CloseWindow();
    initialized_ = false;
}

void Engine::Clear(RenderTexture2D& target, Color color) {
    ::BeginTextureMode(target);
    ::ClearBackground(color);
    ::EndTextureMode();
}

void Engine::Log(const std::string& message) {
    std::cerr << message << '\n';
    if (log_.is_open()) log_ << message << std::endl;
}
