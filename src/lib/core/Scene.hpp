#pragma once

#include <functional>
#include <memory>
#include <utility>
#include <vector>

#include "../render/PrimitiveRenderer.hpp"
#include "Entity.hpp"
#include "Input.hpp"

// Scena: własne obiekty i własna logika. Silnik aktualizuje i rysuje tylko aktywną scenę.
// Obiekty utworzone przez scene.Spawn<T>() to korzenie drzew - mogą mieć własne dzieci.
class Scene {
  public:
    template <typename T> T& Spawn() {
        entities_.push_back(std::make_unique<T>());
        return static_cast<T&>(*entities_.back());
    }

    Scene& OnUpdate(std::function<void(float dt)> update) {
        update_ = std::move(update);
        return *this;
    }
    Scene& OnDraw(std::function<void(PrimitiveRenderer&)> draw) {
        draw_ = std::move(draw);
        return *this;
    }

    void Update(float dt, const Input& input) {
        if (update_) update_(dt);
        // pętla po indeksie: zdarzenie obiektu może wywołać Spawn() i powiększyć wektor
        for (std::size_t i = 0; i < entities_.size(); ++i)
            entities_[i]->Update(dt, input);
    }

    // rysuje drzewa obiektów od najniższej warstwy; w tej samej - w kolejności dodania
    void Draw(PrimitiveRenderer& renderer) const {
        for (const Entity* entity : Entity::ByLayer(entities_))
            entity->Render(renderer);
        if (draw_) draw_(renderer.Reset().TargetScreen());
    }

  private:
    std::vector<std::unique_ptr<Entity>> entities_;
    std::function<void(float)> update_;
    std::function<void(PrimitiveRenderer&)> draw_;
};
