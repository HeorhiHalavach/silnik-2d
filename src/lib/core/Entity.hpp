/**
 * @file Entity.hpp
 * @brief Abstrakcyjna baza obiektów sceny: drzewo dzieci i współrzędne.
 */
#pragma once

#include <raylib.h>
#include <memory>
#include <optional>
#include <vector>

#include "Drawable.hpp"
#include "Transformable.hpp"
#include "Updatable.hpp"

/**
 * @brief Abstrakcyjna baza wszystkich obiektów sceny.
 * @details Obiekty tworzą drzewo: każdy może mieć dzieci (Spawn<T>()), a dzieci kolejne
 * dzieci. Pozycja dziecka jest względna wobec rodzica, więc przesunięcie rodzica przesuwa
 * całe poddrzewo. Dzieci są zawsze rysowane nad swoim rodzicem.
 * Aktualizację i zdarzenia zapewnia Updatable, rysowanie i warstwy Drawable,
 * a macierz oraz jej zmiany Transformable.
 *
 * Obsługiwane zdarzenia (sprawdzane w Update()):
 * - OnUpdate - co klatkę,
 * - OnClick - kliknięcie lewym przyciskiem myszy na obiekcie,
 * - OnMouseEnter / OnMouseLeave - kursor wszedł na obiekt / zszedł z niego,
 * - OnKeyPressed - naciśnięcie konkretnego klawisza.
 *
 * @section entity_lancuch Łańcuchowanie
 * Settery i zdarzenia z klas bazowych (Position(), Rotate(), Paint(), OnClick() itd.) używają
 * jawnego parametru obiektu z C++23 (`this Self& self`). Zwracają więc konkretny typ, na którym
 * je wywołano, a callbacki zdarzeń dostają ten typ jako argument:
 * @code{.cpp}
 * scene.Spawn<shapes::Circle>()
 *     .Position({400, 300})
 *     .Rotate(PI / 4)                   // nadal shapes::Circle&
 *     .Radius(30)                       // setter samego okręgu
 *     .OnClick([](shapes::Circle& self) { self.Radius(50); });
 * @endcode
 *
 * @section entity_wlasne Własny kształt
 * Wystarczy dziedziczyć po Entity i nadpisać Contains() oraz Draw(). Umieszczenie klasy
 * w przestrzeni nazw `shapes` sprawia, że edytor podpowie ją razem z wbudowanymi.
 * @code{.cpp}
 * namespace shapes {
 * class Star : public Entity {
 *   public:
 *     bool Contains(Vector2 point) const override;
 *   protected:
 *     void Draw(PrimitiveRenderer& renderer) const override;
 * };
 * } // namespace shapes
 *
 * scene.Spawn<shapes::Star>().Position({100, 100}).Paint(GOLD);
 * @endcode
 */
class Entity : public Updatable, public Drawable, public Transformable {
  public:
    /** @brief Wirtualny destruktor umożliwiający usuwanie obiektów przez wskaźnik Entity. */
    virtual ~Entity() = default;

    /**
     * @brief Tworzy dziecko wybranego typu.
     * @tparam T Kształt z przestrzeni nazw @ref shapes albo własna klasa dziedzicząca
     * po Entity, np. `Spawn<shapes::Circle>()`.
     * @return Referencja do nowego dziecka, do dalszej konfiguracji.
     */
    template <typename T> T& Spawn() {
        auto child = std::make_unique<T>();
        T& ref = *child;
        child->parent_ = this;
        children_.push_back(std::move(child));
        return ref;
    }

    /**
     * @brief Wykonuje konfigurację dzieci i pozwala kontynuować łańcuch.
     * @param setup Funkcja otrzymująca ten obiekt jako jego konkretny typ
     * (dzieci tworzy się w niej przez Spawn<T>()).
     * @return Ten sam obiekt.
     */
    template <typename Self> Self& Children(this Self& self, const Callback<Self>& setup) {
        setup(self);
        return self;
    }

    /**
     * @brief Oblicza pozycję obiektu w układzie ekranu (z uwzględnieniem rodziców).
     * @return Pozycja na ekranie.
     */
    Vector2 WorldPosition() const { return World({0.0f, 0.0f}); }

    /**
     * @brief Przelicza punkt ekranowy (np. kursor) na układ lokalny obiektu.
     * @param point Punkt w układzie ekranu.
     * @return Punkt w układzie lokalnym albo {0, 0}, gdy macierz jest nieodwracalna.
     */
    Vector2 ToLocal(Vector2 point) const { return TryToLocal(point).value_or(Vector2{}); }

    /**
     * @brief Przelicza punkt ekranowy na lokalny, jeśli przekształcenie jest odwracalne.
     * @param point Punkt w układzie ekranu.
     * @return Punkt lokalny albo brak wyniku, np. po skalowaniu przez zero.
     */
    std::optional<Vector2> TryToLocal(Vector2 point) const {
        return WorldTransform().InverseApply(point);
    }

    /**
     * @brief Pobiera rodzica obiektu.
     * @return Wskaźnik rodzica albo nullptr dla obiektu na najwyższym poziomie sceny.
     */
    Entity* Parent() const { return parent_; }

    /**
     * @brief Sprawdza trafienie punktem (np. kursorem) w obiekt.
     * @param point Punkt w układzie ekranu.
     * @return true, gdy punkt należy do obiektu.
     */
    virtual bool Contains(Vector2 point) const = 0;

  protected:
    /**
     * @brief Rysuje sam kształt obiektu, bez dzieci.
     * @details Wierzchołki należy podawać w układzie lokalnym. Render() nakłada
     * przekształcenia obiektu i jego rodziców podczas rysowania.
     * @param renderer Renderer z wyczyszczonym pędzlem i celem ustawionym na ekran.
     */
    void Draw(PrimitiveRenderer& renderer) const override = 0;

    /**
     * @brief Przelicza punkt lokalny (względem pozycji obiektu) na punkt ekranowy.
     * @param local Punkt lokalny.
     * @return Punkt w układzie ekranu.
     */
    Vector2 World(Vector2 local) const {
        return WorldTransform().Apply(local);
    }

    /** @brief Łączy własne przekształcenie z przekształceniami rodziców. */
    AffineMatrix2D WorldTransform() const {
        if (!parent_) return transform_;
        return parent_->WorldTransform() * transform_;
    }

    /**
     * @brief Przelicza listę wierzchołków lokalnych na ekranowe.
     * @tparam Vertex Typ wierzchołka (Vector2 albo Point2D).
     * @param local Wierzchołki lokalne.
     * @return Wierzchołki w układzie ekranu.
     */
    template <typename Vertex>
    std::vector<Vertex> WorldVertices(const std::vector<Vertex>& local) const {
        std::vector<Vertex> world;
        world.reserve(local.size());
        for (const Vertex& vertex : local)
            world.push_back(Vertex{World(vertex)});
        return world;
    }

  private:
    bool HitTest(Vector2 point) const override { return Contains(point); }
    std::size_t UpdateChildCount() const override { return children_.size(); }
    Updatable& UpdateChild(std::size_t index) override { return *children_[index]; }
    AffineMatrix2D LocalTransform() const override { return transform_; }
    std::vector<const Drawable*> DrawableChildren() const override {
        std::vector<const Drawable*> result;
        result.reserve(children_.size());
        for (const auto& child : children_)
            result.push_back(child.get());
        return result;
    }

    Entity* parent_ = nullptr;                      /**< Rodzic albo nullptr. */
    std::vector<std::unique_ptr<Entity>> children_; /**< Dzieci obiektu. */
};
