#pragma once

#include <raylib.h>

#include <algorithm>
#include <functional>
#include <memory>
#include <utility>
#include <vector>

#include "Bitmap.hpp"
#include "Input.hpp"
#include "PrimitiveRenderer.hpp"

// Baza wszystkich obiektów sceny. Obiekty tworzą drzewo: każdy może mieć dzieci
// (Spawn<T>()), a dzieci kolejne dzieci. Pozycja dziecka jest względna wobec rodzica,
// więc przesunięcie rodzica przesuwa całe poddrzewo.
//
// Zdarzenia:
//   OnUpdate     - co klatkę
//   OnClick      - kliknięcie lewym przyciskiem myszy na obiekcie
//   OnMouseEnter - kursor wszedł na obiekt
//   OnMouseLeave - kursor zszedł z obiektu
//   OnKeyPressed - naciśnięcie konkretnego klawisza
// Typowane wersje (z dostępem do obiektu) dodaje Shape<T>.
class Entity {
  public:
    virtual ~Entity() = default;

    // dziecko tego obiektu; zwraca referencję do dziecka, żeby je skonfigurować
    template <typename T> T& Spawn() {
        auto child = std::make_unique<T>();
        T& ref = *child;
        child->parent_ = this;
        children_.push_back(std::move(child));
        return ref;
    }

    // pozycja względem rodzica oraz wynikowa pozycja na ekranie
    Vector2 Position() const { return position_; }
    Vector2 WorldPosition() const {
        if (!parent_) return position_;
        const Vector2 origin = parent_->WorldPosition();
        return {origin.x + position_.x, origin.y + position_.y};
    }

    // punkt na ekranie (np. kursor) -> punkt względem pozycji obiektu
    Vector2 ToLocal(Vector2 point) const {
        const Vector2 origin = WorldPosition();
        return {point.x - origin.x, point.y - origin.y};
    }

    Entity* Parent() const { return parent_; }
    bool Hovered() const { return hovered_; }

    // warstwa: wyższa rysowana jest na wierzchu, domyślnie 0.
    // Działa między rodzeństwem - dzieci zawsze rysowane są nad swoim rodzicem.
    int Layer() const { return layer_; }

    // czy punkt na ekranie (np. kursor) leży na obiekcie - potrzebne do zdarzeń myszy
    virtual bool Contains(Vector2 point) const = 0;

    // co klatkę: zdarzenia tego obiektu, potem jego dzieci
    void Update(float dt, const Input& input) {
        HandleEvents(dt, input);
        // pętla po indeksie: zdarzenie może wywołać Spawn() i powiększyć wektor
        for (std::size_t i = 0; i < children_.size(); ++i)
            children_[i]->Update(dt, input);
    }

    // rysuje obiekt, a nad nim jego dzieci (wg warstw)
    void Render(PrimitiveRenderer& renderer) const {
        renderer.Reset().TargetScreen();
        Draw(renderer);
        for (const Entity* child : ByLayer(children_))
            child->Render(renderer);
    }

    // kolejność rysowania: od najniższej warstwy, w tej samej - w kolejności dodania
    static std::vector<const Entity*> ByLayer(const std::vector<std::unique_ptr<Entity>>& list) {
        std::vector<const Entity*> order;
        order.reserve(list.size());
        for (const auto& entity : list)
            order.push_back(entity.get());
        std::stable_sort(order.begin(), order.end(),
                         [](const Entity* a, const Entity* b) { return a->Layer() < b->Layer(); });
        return order;
    }

  protected:
    // sam kształt, bez dzieci; współrzędne ekranu liczy się przez World()
    virtual void Draw(PrimitiveRenderer& renderer) const = 0;

    // punkt lokalny (względem pozycji obiektu) -> punkt na ekranie
    Vector2 World(Vector2 local) const {
        const Vector2 origin = WorldPosition();
        return {origin.x + local.x, origin.y + local.y};
    }

    Vector2 position_{};
    Color color_ = BLACK;
    bool filled_ = false;
    bool hovered_ = false;
    int layer_ = 0;

    std::function<void(float)> update_;
    std::function<void()> click_;
    std::function<void()> mouseEnter_;
    std::function<void()> mouseLeave_;
    std::vector<std::pair<int, std::function<void()>>> keys_;

  private:
    void HandleEvents(float dt, const Input& input) {
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

    Entity* parent_ = nullptr;
    std::vector<std::unique_ptr<Entity>> children_;
};

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

// Pusty obiekt bez kształtu - "uchwyt" do grupowania dzieci (np. cała postać z kilku figur)
class Group : public Shape<Group> {
  public:
    bool Contains(Vector2) const override { return false; }

  protected:
    void Draw(PrimitiveRenderer&) const override {}
};

// From / To względem pozycji obiektu
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

    bool Contains(Vector2 point) const override {
        return ::CheckCollisionPointLine(point, World(from_), World(to_), kHitDistance);
    }

    static constexpr int kHitDistance = 4; // tolerancja trafienia w odcinek [px]

  protected:
    void Draw(PrimitiveRenderer& renderer) const override {
        renderer.Paint(color_).Incremental(incremental_).Line(World(from_), World(to_));
    }

  private:
    Vector2 from_{};
    Vector2 to_{};
    bool incremental_ = false;
};

// pozycja = lewy górny róg
class Rect : public Shape<Rect> {
  public:
    Rect& Size(Vector2 size) {
        size_ = size;
        return *this;
    }

    bool Contains(Vector2 point) const override {
        const Vector2 corner = WorldPosition();
        return ::CheckCollisionPointRec(point, {corner.x, corner.y, size_.x, size_.y});
    }

  protected:
    void Draw(PrimitiveRenderer& renderer) const override {
        renderer.Paint(color_).Fill(filled_).Rect(WorldPosition(), size_);
    }

  private:
    Vector2 size_{10, 10};
};

// pozycja = środek
class Circle : public Shape<Circle> {
  public:
    Circle& Radius(float radius) {
        radius_ = radius;
        return *this;
    }
    // własny algorytm z 8-krotną symetrią zamiast raylib
    Circle& Symmetric(bool value = true) {
        symmetric_ = value;
        return *this;
    }

    bool Contains(Vector2 point) const override {
        return ::CheckCollisionPointCircle(point, WorldPosition(), radius_);
    }

  protected:
    void Draw(PrimitiveRenderer& renderer) const override {
        renderer.Paint(color_).Fill(filled_).Symmetric(symmetric_).Circle(WorldPosition(), radius_);
    }

  private:
    float radius_ = 10.0f;
    bool symmetric_ = false;
};
#include <cmath>

// pozycja = środek, promień = {poziomy, pionowy}
class Ellipse : public Shape<Ellipse> {
  public:
    Ellipse& Radius(Vector2 radius) {
        radius_ = radius;
        return *this;
    }
    // własny algorytm z 4-krotną symetrią zamiast raylib
    Ellipse& Symmetric(bool value = true) {
        symmetric_ = value;
        return *this;
    }

    bool Contains(Vector2 point) const override {
        const Vector2 local = ToLocal(point);
        const float x = local.x / radius_.x;
        const float y = local.y / radius_.y;
        return x * x + y * y <= 1.0f;
    }

  protected:
    void Draw(PrimitiveRenderer& renderer) const override {
        renderer.Paint(color_)
            .Fill(filled_)
            .Symmetric(symmetric_)
            .Ellipse(WorldPosition(), radius_);
    }

  private:
    Vector2 radius_{20, 10};
    bool symmetric_ = false;
};

// wierzchołki względem pozycji obiektu
class Triangle : public Shape<Triangle> {
  public:
    Triangle& Vertices(Vector2 a, Vector2 b, Vector2 c) {
        a_ = a;
        b_ = b;
        c_ = c;
        return *this;
    }

    bool Contains(Vector2 point) const override {
        return ::CheckCollisionPointTriangle(point, World(a_), World(b_), World(c_));
    }

  protected:
    void Draw(PrimitiveRenderer& renderer) const override {
        renderer.Paint(color_).Fill(filled_).Triangle(World(a_), World(b_), World(c_));
    }

  private:
    Vector2 a_{};
    Vector2 b_{};
    Vector2 c_{};
};

// wierzchołki względem pozycji obiektu
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

    // zamknięta łamana: cały wielokąt, otwarta: tylko jej odcinki
    bool Contains(Vector2 point) const override {
        const std::vector<Vector2> world = WorldVertices();
        const int count = static_cast<int>(world.size());
        if (closed_ && count >= 3) return ::CheckCollisionPointPoly(point, world.data(), count);
        for (int i = 0; i + 1 < count; ++i)
            if (::CheckCollisionPointLine(point, world[i], world[i + 1], Line::kHitDistance))
                return true;
        return false;
    }

  protected:
    void Draw(PrimitiveRenderer& renderer) const override {
        renderer.Paint(color_).Incremental(incremental_).Polyline(WorldVertices(), closed_);
    }

  private:
    std::vector<Vector2> WorldVertices() const {
        std::vector<Vector2> world;
        world.reserve(vertices_.size());
        for (const Vector2& vertex : vertices_)
            world.push_back(World(vertex));
        return world;
    }

    std::vector<Vector2> vertices_;
    bool closed_ = false;
    bool incremental_ = false;
};

// Płótno: bitmapa w RAM wyświetlana w pozycji obiektu (lewy górny róg). Można po niej rysować
// i wypełniać obszary (BoundaryFill / FloodFill), bo w odróżnieniu od ekranu da się z niej
// odczytać kolory pikseli. Współrzędne rysowania są względem lewego górnego rogu płótna.
//
//   scene.Spawn<Canvas>().Position({50, 50}).Size({300, 200}).Background(WHITE)
//       .Content([](PrimitiveRenderer& r) { r.Paint(BLACK).Circle({150, 100}, 80); });
class Canvas : public Shape<Canvas> {
  public:
    Canvas& Size(Vector2 size) {
        bitmap_ = Bitmap{static_cast<int>(size.x), static_cast<int>(size.y), background_};
        return *this;
    }
    Canvas& Background(Color color) {
        background_ = color;
        bitmap_.Clear(color);
        return *this;
    }
    Canvas& Clear() {
        bitmap_.Clear(background_);
        return *this;
    }
    // rysowanie po płótnie - renderer ma już ustawiony cel na bitmapę płótna
    Canvas& Content(const std::function<void(PrimitiveRenderer&)>& paint) {
        PrimitiveRenderer renderer;
        renderer.Target(bitmap_);
        paint(renderer);
        return *this;
    }

    Bitmap& Pixels() { return bitmap_; }

    bool Contains(Vector2 point) const override {
        const Vector2 corner = WorldPosition();
        return ::CheckCollisionPointRec(point,
                                        {corner.x, corner.y, static_cast<float>(bitmap_.Width()),
                                         static_cast<float>(bitmap_.Height())});
    }

  protected:
    void Draw(PrimitiveRenderer& renderer) const override {
        if (bitmap_.Width() > 0) renderer.Blit(bitmap_, WorldPosition());
    }

  private:
    Bitmap bitmap_;
    Color background_ = WHITE;
};

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
