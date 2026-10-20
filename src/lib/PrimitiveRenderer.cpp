#include "PrimitiveRenderer.hpp"

#include <cmath>
#include <utility>

PrimitiveRenderer& PrimitiveRenderer::Reset() {
    color_ = BLACK;
    filled_ = false;
    incremental_ = false;
    return *this;
}

PrimitiveRenderer& PrimitiveRenderer::Point(Vector2 position) {
    ::DrawPixelV(position, color_);
    return *this;
}

PrimitiveRenderer& PrimitiveRenderer::Line(Vector2 from, Vector2 to) {
    if (incremental_)
        LineIncremental(from, to);
    else
        ::DrawLineV(from, to, color_);
    return *this;
}

// Algorytm przyrostowy:
//  1. m = dy/dx
//  2. x rośnie o 1 od lewej strony, y rośnie o m, piksel w (x, round(y))
// Dla |m| > 1 zamieniamy role osi (krok po y, x += 1/m), inaczej odcinek miałby dziury.
void PrimitiveRenderer::LineIncremental(Vector2 from, Vector2 to) {
    const float dx = to.x - from.x;
    const float dy = to.y - from.y;

    if (dx == 0.0f && dy == 0.0f) { // odcinek zdegenerowany do punktu
        ::DrawPixelV(from, color_);
        return;
    }

    if (std::fabs(dx) >= std::fabs(dy)) { // |m| <= 1 -> krok po x
        if (from.x > to.x) std::swap(from, to);
        const float m = (to.y - from.y) / (to.x - from.x);
        const int x0 = static_cast<int>(std::lround(from.x));
        const int x1 = static_cast<int>(std::lround(to.x));
        float y = from.y;
        for (int x = x0; x <= x1; ++x) {
            ::DrawPixel(x, static_cast<int>(std::lround(y)), color_);
            y += m;
        }
    } else { // |m| > 1 -> krok po y
        if (from.y > to.y) std::swap(from, to);
        const float mInverse = (to.x - from.x) / (to.y - from.y);
        const int y0 = static_cast<int>(std::lround(from.y));
        const int y1 = static_cast<int>(std::lround(to.y));
        float x = from.x;
        for (int y = y0; y <= y1; ++y) {
            ::DrawPixel(static_cast<int>(std::lround(x)), y, color_);
            x += mInverse;
        }
    }
}

PrimitiveRenderer& PrimitiveRenderer::Rect(Vector2 position, Vector2 size) {
    if (filled_)
        ::DrawRectangleV(position, size, color_);
    else
        ::DrawRectangleLinesEx({position.x, position.y, size.x, size.y}, 1.0f, color_);
    return *this;
}

PrimitiveRenderer& PrimitiveRenderer::Circle(Vector2 center, float radius) {
    if (filled_)
        ::DrawCircleV(center, radius, color_);
    else
        ::DrawCircleLinesV(center, radius, color_);
    return *this;
}

PrimitiveRenderer& PrimitiveRenderer::Triangle(Vector2 a, Vector2 b, Vector2 c) {
    if (filled_) {
        // raylib wymaga wierzchołków przeciwnie do ruchu wskazówek zegara
        const float cross = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
        if (cross > 0.0f) std::swap(b, c);
        ::DrawTriangle(a, b, c, color_);
    } else {
        ::DrawTriangleLines(a, b, c, color_);
    }
    return *this;
}

PrimitiveRenderer& PrimitiveRenderer::Points(std::span<const Vector2> positions) {
    for (const Vector2& position : positions)
        Point(position);
    return *this;
}

PrimitiveRenderer& PrimitiveRenderer::Lines(std::span<const Vector2> vertices) {
    for (std::size_t i = 0; i + 1 < vertices.size(); i += 2)
        Line(vertices[i], vertices[i + 1]);
    return *this;
}

PrimitiveRenderer& PrimitiveRenderer::Polyline(std::span<const Vector2> vertices, bool closed) {
    if (vertices.size() < 2) return *this;
    for (std::size_t i = 0; i + 1 < vertices.size(); ++i)
        Line(vertices[i], vertices[i + 1]);
    if (closed) Line(vertices.back(), vertices.front());
    return *this;
}
