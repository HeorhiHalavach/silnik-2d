#pragma once

#include <raylib.h>

#include <fstream>
#include <functional>
#include <map>
#include <string>
#include <utility>

#include "../render/Bitmap.hpp"
#include "../render/PrimitiveRenderer.hpp"
#include "Input.hpp"
#include "Scene.hpp"
#include "Shapes.hpp"

// Silnik gry. Konfiguracja, scena i pętla gry przez method chaining:
//
//   Engine& engine = Engine::Get().Title("Gra").Window(800, 600).Fps(60).Init();
//   if (!engine.Ready()) return 1;
//
//   engine.CreateScene("menu", [&](Scene& scene) {   // pierwsza scena staje się aktywna
//       scene.Spawn<Circle>().Position({400, 300}).Radius(20).Paint(ORANGE).Fill();
//   });
//
//   engine.CreateScene("gra", [&](Scene& scene) {
//       scene.OnUpdate([&](float dt) { ... });
//   });
//
//   return engine
//       .OnUpdate([&](float) {
//           if (engine.KeyPressed(KEY_ENTER)) engine.SwitchScene("gra");
//       })
//       .Run();
//
// Settery okna działają też po Init() - zmieniają okno na bieżąco.
class Engine {
  public:
    // Singleton: jedyna instancja, tworzona przy pierwszym wywołaniu.
    static Engine& Get() {
        static Engine instance;
        return instance;
    }

    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;

    // konfiguracja okna
    Engine& Title(std::string title);
    Engine& Window(int width, int height);
    Engine& Fullscreen(bool value = true);
    Engine& Fps(int fps);
    Engine& Background(Color color) {
        background_ = color;
        return *this;
    }

    // konfiguracja wejścia i logów
    Engine& Keyboard(bool value = true) {
        input_.Keyboard(value);
        return *this;
    }
    Engine& Mouse(bool value = true) {
        input_.Mouse(value);
        return *this;
    }
    Engine& LogFile(std::string path) {
        logFile_ = std::move(path);
        return *this;
    }

    // logika globalna wywoływana co klatkę, niezależnie od aktywnej sceny
    Engine& OnUpdate(std::function<void(float dt)> update) {
        update_ = std::move(update);
        return *this;
    }
    Engine& OnDraw(std::function<void(PrimitiveRenderer&)> draw) {
        draw_ = std::move(draw);
        return *this;
    }

    // cykl życia: Init() otwiera okno, Run() kręci pętlę aż do zamknięcia i sprząta.
    // Run() zwraca kod wyjścia dla main (0 - ok, 1 - błąd inicjalizacji).
    Engine& Init();
    bool Ready() const { return initialized_; }
    int Run();
    void Shutdown();

    // sceny: aktualizowana i rysowana jest tylko aktywna.
    // SwitchScene() zmienia scenę na końcu bieżącej klatki.
    // CreateScene(name, setup) tworzy scenę, od razu wywołuje setup(scena) i zwraca silnik,
    // więc sceny można łączyć w jeden łańcuch. Wersja bez setup zwraca samą scenę.
    Engine& CreateScene(const std::string& name, const std::function<void(Scene&)>& setup);
    Scene& CreateScene(const std::string& name);

    // RegisterScene() tylko zapamiętuje setup - scena powstaje w Run(), już po otwarciu okna.
    // Zwykle używane przez makro SCENE (niżej). StartScene() wybiera scenę startową.
    Engine& RegisterScene(std::string name, std::function<void(Scene&)> setup);
    Engine& StartScene(std::string name) {
        start_ = std::move(name);
        return *this;
    }
    Scene& GetScene(const std::string& name) { return scenes_.at(name); }
    Engine& SwitchScene(const std::string& name);
    const std::string& ActiveScene() const { return active_; }

    // klawiatura i mysz
    bool KeyDown(int key) const { return input_.KeyDown(key); }
    bool KeyPressed(int key) const { return input_.KeyPressed(key); }
    bool MouseDown(int button) const { return input_.MouseDown(button); }
    bool MousePressed(int button) const { return input_.MousePressed(button); }
    Vector2 MousePosition() const { return input_.MousePosition(); }

    // czyszczenie ekranu / bitmapy
    Engine& Clear(Color color) {
        ::ClearBackground(color);
        return *this;
    }
    Engine& Clear(RenderTexture2D& target, Color color);
    Engine& Clear(Bitmap& bitmap, Color color) {
        bitmap.Clear(color);
        return *this;
    }

    // rysowanie prymitywów
    PrimitiveRenderer& Primitives() { return primitives_; }

    // błędy / diagnostyka
    Engine& Log(const std::string& message);

  private:
    Engine() = default;
    ~Engine() { Shutdown(); }

    std::string title_ = "Gra";
    int width_ = 800;
    int height_ = 600;
    bool fullscreen_ = false;
    int fps_ = 60;
    Color background_ = BLACK;
    Input input_;
    std::string logFile_ = "engine.log";

    std::function<void(float)> update_;
    std::function<void(PrimitiveRenderer&)> draw_;

    std::ofstream log_;
    bool initialized_ = false;

    std::map<std::string, Scene> scenes_;
    std::string active_;
    std::string pending_; // scena, na którą przełączamy po zakończeniu klatki
    std::string start_;   // scena startowa wybrana przez StartScene()
    std::map<std::string, std::function<void(Scene&)>> registered_;
    PrimitiveRenderer primitives_;

    void CreateRegisteredScenes();
};

// ------------------------------------------------------------
// Samorejestracja scen. W dowolnym pliku .cpp:
//
//   SCENE("menu") {
//       scene.Spawn<Rect>().Position({300, 200}).Size({400, 300});
//   }
//
// Wewnątrz bloku dostępna jest zmienna `scene` (Scene&). Scena zostanie utworzona w Run(),
// a scenę startową wybiera engine.StartScene("menu") w main.
// ------------------------------------------------------------
struct SceneRegistration {
    SceneRegistration(const char* name, void (*setup)(Scene&)) {
        Engine::Get().RegisterScene(name, setup);
    }
};

#define SCENE_CONCAT_(a, b) a##b
#define SCENE_CONCAT(a, b) SCENE_CONCAT_(a, b)
#define SCENE_IMPL_(name, setup)                                                                   \
    static void setup(Scene& scene);                                                               \
    static const SceneRegistration SCENE_CONCAT(setup, _registration){name, setup};                \
    static void setup(Scene& scene)
#define SCENE(name) SCENE_IMPL_(name, SCENE_CONCAT(scene_setup_, __LINE__))
