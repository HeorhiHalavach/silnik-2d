#pragma once

#include <raylib.h>

#include <optional>
#include <span>
#include <vector>

class PrimitiveRenderer;

// Punkt na płaszczyźnie. Zamienia się automatycznie z/na Vector2 z raylib,
// więc można go podawać wszędzie tam, gdzie silnik przyjmuje Vector2:
//
//   Point2D p{100, 50};
//   p.X(120).Y(80).Draw(renderer);
class Point2D {
  public:
    constexpr Point2D() = default;
    constexpr Point2D(float x, float y) : x_(x), y_(y) {}
    constexpr Point2D(Vector2 vector) : x_(vector.x), y_(vector.y) {}
    constexpr operator Vector2() const { return {x_, y_}; }

    float X() const { return x_; }
    float Y() const { return y_; }
    Point2D& X(float x) {
        x_ = x;
        return *this;
    }
    Point2D& Y(float y) {
        y_ = y;
        return *this;
    }

    // rysuje punkt bieżącym kolorem renderera
    const Point2D& Draw(PrimitiveRenderer& renderer) const;

    bool operator==(const Point2D&) const = default;

  private:
    float x_ = 0.0f;
    float y_ = 0.0f;
};

// Odcinek między dwoma punktami.
//
//   LineSegment edge{{0, 0}, {100, 50}};
//   edge.Draw(renderer.Incremental());   // algorytm wybiera renderer
class LineSegment {
  public:
    constexpr LineSegment() = default;
    constexpr LineSegment(Point2D from, Point2D to) : from_(from), to_(to) {}

    Point2D From() const { return from_; }
    Point2D To() const { return to_; }
    LineSegment& From(Point2D from) {
        from_ = from;
        return *this;
    }
    LineSegment& To(Point2D to) {
        to_ = to;
        return *this;
    }

    // czy odcinki mają choć jeden punkt wspólny (także styk końcami i nakładanie się)
    bool Intersects(const LineSegment& other) const;

    // rysuje odcinek bieżącym kolorem i algorytmem renderera
    const LineSegment& Draw(PrimitiveRenderer& renderer) const;

  private:
    Point2D from_;
    Point2D to_;
};

// Czy wierzchołki tworzą łamaną zwyczajną zamkniętą: co najmniej 3 wierzchołki,
// a krawędzie niesąsiednie nie mają punktów wspólnych. Inaczej to łamana wiązana.
bool IsSimplePolygon(std::span<const Point2D> vertices);

// Zamienia zbiór odcinków na listę wierzchołków. Odcinki muszą tworzyć zamknięty
// łańcuch (koniec każdego = początek następnego, koniec ostatniego = początek pierwszego).
std::optional<std::vector<Point2D>> PolygonVertices(std::span<const LineSegment> edges);
