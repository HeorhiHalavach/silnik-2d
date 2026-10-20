#pragma once

#include <raylib.h>

#include <span>

// Rysowanie prymitywów 2D. Działa jak pędzel: ma bieżący kolor, wypełnienie
// i algorytm odcinka. Każda metoda zwraca *this, więc wywołania można łączyć:
//
//   renderer.Paint(RED).Fill().Circle({100, 100}, 40).Fill(false).Rect({200, 60}, {120, 80});
//
// Stan zostaje między wywołaniami - Reset() przywraca domyślny.
class PrimitiveRenderer {
  public:
    // stan pędzla
    PrimitiveRenderer& Paint(Color color) {
        color_ = color;
        return *this;
    }
    PrimitiveRenderer& Fill(bool value = true) {
        filled_ = value;
        return *this;
    }
    PrimitiveRenderer& Incremental(bool value = true) {
        incremental_ = value;
        return *this;
    }
    PrimitiveRenderer& Reset();

    // pojedyncze prymitywy
    PrimitiveRenderer& Point(Vector2 position);
    PrimitiveRenderer& Line(Vector2 from, Vector2 to);
    PrimitiveRenderer& Rect(Vector2 position, Vector2 size);
    PrimitiveRenderer& Circle(Vector2 center, float radius);
    PrimitiveRenderer& Triangle(Vector2 a, Vector2 b, Vector2 c);

    // wersje tablicowe (wiele prymitywów w jednej pętli)
    PrimitiveRenderer& Points(std::span<const Vector2> positions);
    PrimitiveRenderer& Lines(std::span<const Vector2> vertices); // pary (0,1), (2,3), ...
    PrimitiveRenderer& Polyline(std::span<const Vector2> vertices, bool closed = false);

  private:
    void LineIncremental(Vector2 from, Vector2 to); // algorytm przyrostowy

    Color color_ = BLACK;
    bool filled_ = false;
    bool incremental_ = false;
};
