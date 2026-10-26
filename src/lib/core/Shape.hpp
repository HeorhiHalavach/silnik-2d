#pragma once

#include <raylib.h>

#include <functional>
#include <utility>

#include "Entity.hpp"

// Wspólne settery i zdarzenia. Zwracają T&, żeby łańcuch nie "zgubił" typu,
// a funkcje zdarzeń dostają obiekt jako T&, np.:
//
//   scene.Spawn<Circle>().Radius(30).OnClick([](Circle& self) { self.Radius(50); });
//
// Dzieci można dodać w łańcuchu przez Children():
//
//   scene.Spawn<Rect>().Position({100, 100}).Size({200, 100}).Children([](Rect& self) {
//       self.Spawn<Circle>().Position({20, 20}).Radius(10);   // względem prostokąta
//   });
template <typename T> class Shape : public Entity {
  public:
    using Entity::Position; // getter Position() z Entity, inaczej przesłoniłby go setter
    T& Position(Vector2 position) {
        position_ = position;
        return Self();
    }
    T& Move(Vector2 offset) {
        position_ = {position_.x + offset.x, position_.y + offset.y};
        return Self();
    }
    T& Paint(Color color) {
        color_ = color;
        return Self();
    }
    using Entity::Layer; // jak wyżej
    T& Layer(int layer) {
        layer_ = layer;
        return Self();
    }
    T& Fill(bool value = true) {
        filled_ = value;
        return Self();
    }

    // dzieci w łańcuchu: setup dostaje ten obiekt, a Children() zwraca go dalej
    T& Children(const std::function<void(T&)>& setup) {
        setup(Self());
        return Self();
    }

    T& OnUpdate(std::function<void(T&, float)> callback) {
        update_ = [this, callback = std::move(callback)](float dt) { callback(Self(), dt); };
        return Self();
    }
    T& OnClick(std::function<void(T&)> callback) {
        click_ = Bind(std::move(callback));
        return Self();
    }
    T& OnMouseEnter(std::function<void(T&)> callback) {
        mouseEnter_ = Bind(std::move(callback));
        return Self();
    }
    T& OnMouseLeave(std::function<void(T&)> callback) {
        mouseLeave_ = Bind(std::move(callback));
        return Self();
    }
    T& OnKeyPressed(int key, std::function<void(T&)> callback) {
        keys_.emplace_back(key, Bind(std::move(callback)));
        return Self();
    }

  protected:
    T& Self() { return static_cast<T&>(*this); }

  private:
    std::function<void()> Bind(std::function<void(T&)> callback) {
        return [this, callback = std::move(callback)] { callback(Self()); };
    }
};
