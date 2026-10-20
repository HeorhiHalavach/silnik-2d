#pragma once

#include <raylib.h>

#include <algorithm>
#include <functional>
#include <memory>
#include <utility>
#include <vector>

#include "Input.hpp"
#include "PrimitiveRenderer.hpp"

// Baza wszystkich obiektów sceny. Obsługuje zdarzenia:
//   OnUpdate     - co klatkę
//   OnClick      - kliknięcie lewym przyciskiem myszy na obiekcie
//   OnMouseEnter - kursor wszedł na obiekt
//   OnMouseLeave - kursor zszedł z obiektu
//   OnKeyPressed - naciśnięcie konkretnego klawisza
// Typowane wersje (z dostępem do obiektu) dodaje Shape<T>.
class Entity {
  public:
    virtual ~Entity() = default;
    virtual void Draw(PrimitiveRenderer& renderer) const = 0;

    // czy punkt (np. kursor) leży na obiekcie - potrzebne do zdarzeń myszy
    virtual bool Contains(Vector2 point) const = 0;

    bool Hovered() const { return hovered_; }

    // warstwa: wyższa rysowana jest na wierzchu, domyślnie 0
    int Layer() const { return layer_; }

    // wywoływane przez scenę co klatkę: sprawdza zdarzenia i woła podpięte funkcje
    void Update(float dt, const Input& input) {
        if (input.MouseEnabled()) {
            const bool inside = Contains(input.MousePosition());
            if (inside && !hovered_ && mouseEnter_) mouseEnter_();
            if (!inside && hovered_ && mouseLeave_) mouseLeave_();
            hovered_ = inside;
            if (inside && input.MousePressed(MOUSE_BUTTON_LEFT) && click_) click_();
        }
        for (const auto& [key, callback] : keys_)
            if (input.KeyPressed(key)) callback();
        if (update_) update_(dt);
    }

  protected:
    Color color_ = BLACK;
    bool filled_ = false;
    bool hovered_ = false;
    int layer_ = 0;

    std::function<void(float)> update_;
    std::function<void()> click_;
    std::function<void()> mouseEnter_;
    std::function<void()> mouseLeave_;
    std::vector<std::pair<int, std::function<void()>>> keys_;
};

// Wspólne settery i zdarzenia. Zwracają T&, żeby łańcuch nie "zgubił" typu,
// a funkcje zdarzeń dostają obiekt jako T&, np.:
//
//   scene.Spawn<Circle>().Radius(30).OnClick([](Circle& self) { self.Radius(50); });
template <typename T> class Shape : public Entity {
  public:
    T& Paint(Color color) {
        color_ = color;
        return Self();
    }
    using Entity::Layer; // getter Layer() z Entity, inaczej przesłoniłby go setter poniżej
    T& Layer(int layer) {
        layer_ = layer;
        return Self();
    }
    T& Fill(bool value = true) {
        filled_ = value;
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

class Line : public Shape<Line> {
  public:
    Line& From(Vector2 position) {
        from_ = position;
        return *this;
    }
    Line& To(Vector2 position) {
        to_ = position;
        return *this;
    }
    Line& Incremental(bool value = true) {
        incremental_ = value;
        return *this;
    }

    void Draw(PrimitiveRenderer& renderer) const override {
        renderer.Paint(color_).Incremental(incremental_).Line(from_, to_);
    }
    bool Contains(Vector2 point) const override {
        return ::CheckCollisionPointLine(point, from_, to_, kHitDistance);
    }

    static constexpr int kHitDistance = 4; // tolerancja trafienia w odcinek [px]

  private:
    Vector2 from_{};
    Vector2 to_{};
    bool incremental_ = false;
};

// position = lewy górny róg
class Rect : public Shape<Rect> {
  public:
    Rect& Position(Vector2 position) {
        position_ = position;
        return *this;
    }
    Rect& Size(Vector2 size) {
        size_ = size;
        return *this;
    }
    Vector2 Position() const { return position_; }

    void Draw(PrimitiveRenderer& renderer) const override {
        renderer.Paint(color_).Fill(filled_).Rect(position_, size_);
    }
    bool Contains(Vector2 point) const override {
        return ::CheckCollisionPointRec(point, {position_.x, position_.y, size_.x, size_.y});
    }

  private:
    Vector2 position_{};
    Vector2 size_{10, 10};
};

// position = środek
class Circle : public Shape<Circle> {
  public:
    Circle& Position(Vector2 position) {
        position_ = position;
        return *this;
    }
    Circle& Radius(float radius) {
        radius_ = radius;
        return *this;
    }
    Vector2 Position() const { return position_; }

    void Draw(PrimitiveRenderer& renderer) const override {
        renderer.Paint(color_).Fill(filled_).Circle(position_, radius_);
    }
    bool Contains(Vector2 point) const override {
        return ::CheckCollisionPointCircle(point, position_, radius_);
    }

  private:
    Vector2 position_{};
    float radius_ = 10.0f;
};

class Triangle : public Shape<Triangle> {
  public:
    Triangle& Vertices(Vector2 a, Vector2 b, Vector2 c) {
        a_ = a;
        b_ = b;
        c_ = c;
        return *this;
    }

    void Draw(PrimitiveRenderer& renderer) const override {
        renderer.Paint(color_).Fill(filled_).Triangle(a_, b_, c_);
    }
    bool Contains(Vector2 point) const override {
        return ::CheckCollisionPointTriangle(point, a_, b_, c_);
    }

  private:
    Vector2 a_{};
    Vector2 b_{};
    Vector2 c_{};
};

class Polyline : public Shape<Polyline> {
  public:
    Polyline& Vertices(std::vector<Vector2> vertices) {
        vertices_ = std::move(vertices);
        return *this;
    }
    Polyline& Closed(bool value = true) {
        closed_ = value;
        return *this;
    }
    Polyline& Incremental(bool value = true) {
        incremental_ = value;
        return *this;
    }

    void Draw(PrimitiveRenderer& renderer) const override {
        renderer.Paint(color_).Incremental(incremental_).Polyline(vertices_, closed_);
    }
    // zamknięta łamana: cały wielokąt, otwarta: tylko jej odcinki
    bool Contains(Vector2 point) const override {
        const int count = static_cast<int>(vertices_.size());
        if (closed_ && count >= 3) return ::CheckCollisionPointPoly(point, vertices_.data(), count);
        for (int i = 0; i + 1 < count; ++i)
            if (::CheckCollisionPointLine(point, vertices_[i], vertices_[i + 1],
                                          Line::kHitDistance))
                return true;
        return false;
    }

  private:
    std::vector<Vector2> vertices_;
    bool closed_ = false;
    bool incremental_ = false;
};

// Scena: własne obiekty i własna logika. Silnik aktualizuje i rysuje tylko aktywną scenę.
//
//   engine.CreateScene("gra")
//       .OnUpdate([&](float dt) { ... })
//       .Spawn<Circle>().Position({400, 300}).Radius(20);
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

    // rysuje od najniższej warstwy; w tej samej warstwie - w kolejności dodania
    void Draw(PrimitiveRenderer& renderer) const {
        std::vector<const Entity*> order;
        order.reserve(entities_.size());
        for (const auto& entity : entities_)
            order.push_back(entity.get());
        std::stable_sort(order.begin(), order.end(),
                         [](const Entity* a, const Entity* b) { return a->Layer() < b->Layer(); });

        for (const Entity* entity : order) {
            renderer.Reset();
            entity->Draw(renderer);
        }
        if (draw_) draw_(renderer.Reset());
    }

  private:
    std::vector<std::unique_ptr<Entity>> entities_;
    std::function<void(float)> update_;
    std::function<void(PrimitiveRenderer&)> draw_;
};
