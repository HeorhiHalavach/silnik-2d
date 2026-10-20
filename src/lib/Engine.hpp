#pragma once

#include <raylib.h>

#include <fstream>
#include <functional>
#include <string>
#include <variant>

struct Windowed {
    int width;
    int height;
};
struct Fullscreen {};
using WindowMode = std::variant<Windowed, Fullscreen>;

struct EngineConfig {
    std::string title = "Gra";
    WindowMode mode = Windowed{800, 600};
    int fps = 60;
    bool keyboard = true;
    bool mouse = true;
    std::string logFile = "engine.log";
};

class Engine {
  public:
    // Singleton: jedyna instancja, tworzona przy pierwszym wywołaniu.
    static Engine& Get() {
        static Engine instance;
        return instance;
    }

    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;

    bool Init(const EngineConfig& config);
    void Run(const std::function<void(float dt)>& update, const std::function<void()>& draw);
    void Shutdown();

    // klawiatura i mysz
    bool KeyDown(int key) const { return cfg_.keyboard && ::IsKeyDown(key); }
    bool KeyPressed(int key) const { return cfg_.keyboard && ::IsKeyPressed(key); }
    bool MouseDown(int button) const { return cfg_.mouse && ::IsMouseButtonDown(button); }
    Vector2 MousePosition() const { return cfg_.mouse ? ::GetMousePosition() : Vector2{0, 0}; }

    // czyszczenie ekranu / bitmapy
    void Clear(Color color) { ::ClearBackground(color); }
    void Clear(RenderTexture2D& target, Color color);

    // błędy / diagnostyka
    void Log(const std::string& message);

  private:
    Engine() = default;
    ~Engine() { Shutdown(); }

    EngineConfig cfg_;
    std::ofstream log_;
    bool initialized_ = false;
};
