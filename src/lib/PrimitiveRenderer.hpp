#pragma once

#include <raylib.h>

#include <span>

#include "Bitmap.hpp"

// Rysowanie prymitywów 2D. Działa jak pędzel: ma bieżący kolor, wypełnienie, algorytmy
// i cel rysowania (ekran albo bitmapa). Każda metoda zwraca *this, więc wywołania można łączyć:
//
//   renderer.Paint(RED).Fill().Circle({100, 100}, 40).Fill(false).Rect({200, 60}, {120, 80});
//
// Stan zostaje między wywołaniami - Reset() przywraca domyślny pędzel. Celu rysowania
// Reset() nie zmienia (robią to tylko Target() / TargetScreen()), więc wywołany w trakcie
// rysowania po bitmapie nie przeniesie przypadkiem rysowania na ekran.
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
    // okręgi i elipsy: własny algorytm z symetrią (8-krotną / 4-krotną) zamiast raylib
    PrimitiveRenderer& Symmetric(bool value = true) {
        symmetric_ = value;
        return *this;
    }
    PrimitiveRenderer& Reset(); // czarny, bez wypełnienia, algorytmy raylib

    // cel rysowania: ekran (domyślnie) albo bitmapa w RAM
    PrimitiveRenderer& Target(Bitmap& bitmap) {
        target_ = &bitmap;
        return *this;
    }
    PrimitiveRenderer& TargetScreen() {
        target_ = nullptr;
        return *this;
    }

    // pojedyncze prymitywy
    PrimitiveRenderer& Point(Vector2 position);
    PrimitiveRenderer& Line(Vector2 from, Vector2 to);
    PrimitiveRenderer& Rect(Vector2 position, Vector2 size);
    PrimitiveRenderer& Circle(Vector2 center, float radius);
    PrimitiveRenderer& Ellipse(Vector2 center, Vector2 radius); // radius = {poziomy, pionowy}
    PrimitiveRenderer& Triangle(Vector2 a, Vector2 b, Vector2 c);

    // wersje tablicowe (wiele prymitywów w jednej pętli)
    PrimitiveRenderer& Points(std::span<const Vector2> positions);
    PrimitiveRenderer& Lines(std::span<const Vector2> vertices); // pary (0,1), (2,3), ...
    PrimitiveRenderer& Polyline(std::span<const Vector2> vertices, bool closed = false);

    // wyświetla bitmapę na ekranie (lewy górny róg w position)
    PrimitiveRenderer& Blit(const Bitmap& bitmap, Vector2 position);

  private:
    void Plot(int x, int y);              // pojedynczy piksel w bieżącym celu
    void Span(int y, int fromX, int toX); // poziomy odcinek pikseli (do wypełnień)

    void LineIncremental(Vector2 from, Vector2 to); // algorytm przyrostowy
    void CircleSymmetric(Vector2 center, float radius);
    void EllipseSymmetric(Vector2 center, Vector2 radius);

    Color color_ = BLACK;
    bool filled_ = false;
    bool incremental_ = false;
    bool symmetric_ = false;
    Bitmap* target_ = nullptr; // nullptr = ekran
};
