/**
 * @file Geometry.hpp
 * @brief Klasy geometryczne Point2D i LineSegment oraz funkcje dla wielokątów.
 */
#pragma once

#include <raylib.h>

#include <optional>
#include <span>
#include <vector>

#include "../core/ObjectInterfaces.hpp"

class PrimitiveRenderer;

/**
 * @brief Punkt dwuwymiarowy zgodny ze współrzędnymi raylib.
 * @details Zamienia się automatycznie z i na Vector2, więc można go podawać wszędzie tam,
 * gdzie silnik przyjmuje Vector2.
 * @code{.cpp}
 * Point2D p{100, 50};
 * p.X(120).Y(80).Render(renderer);
 * @endcode
 */
class Point2D : public ShapeObject {
  public:
    /** @brief Tworzy punkt w początku układu. */
    Point2D() = default;

    /**
     * @brief Tworzy punkt z dwóch współrzędnych.
     * @param x Współrzędna pozioma.
     * @param y Współrzędna pionowa.
     */
    Point2D(float x, float y) : x_(x), y_(y) {}

    /**
     * @brief Tworzy punkt z wektora raylib.
     * @param vector Współrzędne wektora.
     */
    Point2D(Vector2 vector) : x_(vector.x), y_(vector.y) {}

    /**
     * @brief Konwertuje punkt na wektor raylib.
     * @return Wektor o tych samych współrzędnych.
     */
    operator Vector2() const { return {x_, y_}; }

    /**
     * @brief Pobiera współrzędną poziomą.
     * @return Wartość X.
     */
    float X() const { return x_; }

    /**
     * @brief Pobiera współrzędną pionową.
     * @return Wartość Y.
     */
    float Y() const { return y_; }

    /**
     * @brief Zmienia współrzędną poziomą.
     * @param x Nowa wartość X.
     * @return Ten sam punkt.
     */
    Point2D& X(float x) {
        x_ = x;
        return *this;
    }

    /**
     * @brief Zmienia współrzędną pionową.
     * @param y Nowa wartość Y.
     * @return Ten sam punkt.
     */
    Point2D& Y(float y) {
        y_ = y;
        return *this;
    }

    /**
     * @brief Rysuje punkt bieżącym kolorem renderera.
     * @param renderer Renderer docelowy.
     */
    void Render(PrimitiveRenderer& renderer) const override;

    void Translate(Vector2 offset) override;
    void Rotate(float radians, Vector2 pivot) override;
    void Scale(Vector2 factors, Vector2 pivot) override;

    /** @brief Porównuje obie współrzędne punktów. */
    bool operator==(const Point2D& other) const {
        return x_ == other.x_ && y_ == other.y_;
    }

  private:
    float x_ = 0.0f; /**< Współrzędna pozioma. */
    float y_ = 0.0f; /**< Współrzędna pionowa. */
};

/**
 * @brief Odcinek wyznaczony przez dwa punkty Point2D.
 * @code{.cpp}
 * LineSegment edge{{0, 0}, {100, 50}};
 * renderer.Incremental(false);
 * edge.Render(renderer);     // algorytm domyślny (raylib)
 * renderer.Incremental(true);
 * edge.Render(renderer);     // algorytm przyrostowy
 * @endcode
 */
class LineSegment : public ShapeObject {
  public:
    /** @brief Tworzy odcinek zerowy w początku układu. */
    LineSegment() = default;

    /**
     * @brief Tworzy odcinek między dwoma punktami.
     * @param from Początek.
     * @param to Koniec.
     */
    LineSegment(Point2D from, Point2D to) : from_(from), to_(to) {}

    /**
     * @brief Pobiera początek odcinka.
     * @return Punkt początkowy.
     */
    Point2D From() const { return from_; }

    /**
     * @brief Pobiera koniec odcinka.
     * @return Punkt końcowy.
     */
    Point2D To() const { return to_; }

    /**
     * @brief Zmienia początek odcinka.
     * @param from Nowy początek.
     * @return Ten sam odcinek.
     */
    LineSegment& From(Point2D from) {
        from_ = from;
        return *this;
    }

    /**
     * @brief Zmienia koniec odcinka.
     * @param to Nowy koniec.
     * @return Ten sam odcinek.
     */
    LineSegment& To(Point2D to) {
        to_ = to;
        return *this;
    }

    /**
     * @brief Sprawdza, czy odcinki mają choć jeden punkt wspólny.
     * @details Uwzględnia także styk końcami i nakładanie się odcinków współliniowych.
     * @param other Drugi odcinek.
     * @return true, gdy odcinki mają wspólny punkt.
     */
    bool Intersects(const LineSegment& other) const;

    /**
     * @brief Rysuje odcinek bieżącym kolorem i algorytmem renderera.
     * @param renderer Renderer docelowy.
     */
    void Render(PrimitiveRenderer& renderer) const override;

    void Translate(Vector2 offset) override;
    void Rotate(float radians, Vector2 pivot) override;
    void Scale(Vector2 factors, Vector2 pivot) override;

  private:
    Point2D from_; /**< Początek odcinka. */
    Point2D to_;   /**< Koniec odcinka. */
};

/**
 * @brief Sprawdza, czy wierzchołki tworzą łamaną zwyczajną zamkniętą.
 * @details Wymaga co najmniej trzech wierzchołków, a krawędzie niesąsiednie nie mogą mieć
 * punktów wspólnych. W przeciwnym razie jest to łamana wiązana.
 * @param vertices Wierzchołki w kolejności obwodu.
 * @return false dla mniej niż trzech punktów lub przecięcia krawędzi niesąsiednich.
 * @note Powtórzone sąsiednie wierzchołki nie są osobno odrzucane.
 */
bool IsSimplePolygon(std::span<const Point2D> vertices);

/**
 * @brief Zamienia uporządkowany, zamknięty łańcuch odcinków na listę wierzchołków.
 * @details Koniec każdego odcinka musi być początkiem następnego, a koniec ostatniego -
 * początkiem pierwszego.
 * @param edges Odcinki w kolejności obwodu.
 * @return Lista wierzchołków albo std::nullopt, gdy łańcuch jest pusty lub niedomknięty.
 */
std::optional<std::vector<Point2D>> PolygonVertices(std::span<const LineSegment> edges);
