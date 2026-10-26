#include "PrimitiveRenderer.hpp"

#include <algorithm>
#include <cmath>
#include <utility>

namespace {

int Round(float value) {
    return static_cast<int>(std::lround(value));
}

} // namespace

PrimitiveRenderer& PrimitiveRenderer::Reset() {
    color_ = BLACK;
    filled_ = false;
    incremental_ = false;
    symmetric_ = false;
    return *this;
}

// ------------------------------------------------------------
// Piksele
// ------------------------------------------------------------

void PrimitiveRenderer::Plot(int x, int y) {
    if (target_)
        target_->Pixel(x, y, color_);
    else
        ::DrawPixel(x, y, color_);
}

void PrimitiveRenderer::Span(int y, int fromX, int toX) {
    if (fromX > toX) std::swap(fromX, toX);
    for (int x = fromX; x <= toX; ++x)
        Plot(x, y);
}

// ------------------------------------------------------------
// Pojedyncze prymitywy
// ------------------------------------------------------------

PrimitiveRenderer& PrimitiveRenderer::Point(Vector2 position) {
    Plot(Round(position.x), Round(position.y));
    return *this;
}

PrimitiveRenderer& PrimitiveRenderer::Line(Vector2 from, Vector2 to) {
    if (incremental_)
        LineIncremental(from, to);
    else if (target_)
        ::ImageDrawLineV(&target_->Edit(), from, to, color_);
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
        Plot(Round(from.x), Round(from.y));
        return;
    }

    if (std::fabs(dx) >= std::fabs(dy)) { // |m| <= 1 -> krok po x
        if (from.x > to.x) std::swap(from, to);
        const float m = (to.y - from.y) / (to.x - from.x);
        float y = from.y;
        for (int x = Round(from.x); x <= Round(to.x); ++x) {
            Plot(x, Round(y));
            y += m;
        }
    } else { // |m| > 1 -> krok po y
        if (from.y > to.y) std::swap(from, to);
        const float mInverse = (to.x - from.x) / (to.y - from.y);
        float x = from.x;
        for (int y = Round(from.y); y <= Round(to.y); ++y) {
            Plot(Round(x), y);
            x += mInverse;
        }
    }
}

PrimitiveRenderer& PrimitiveRenderer::Rect(Vector2 position, Vector2 size) {
    const Rectangle rectangle{position.x, position.y, size.x, size.y};
    if (target_) {
        if (filled_)
            ::ImageDrawRectangleRec(&target_->Edit(), rectangle, color_);
        else
            ::ImageDrawRectangleLines(&target_->Edit(), rectangle, 1, color_);
    } else {
        if (filled_)
            ::DrawRectangleRec(rectangle, color_);
        else
            ::DrawRectangleLinesEx(rectangle, 1.0f, color_);
    }
    return *this;
}

PrimitiveRenderer& PrimitiveRenderer::Circle(Vector2 center, float radius) {
    if (symmetric_) {
        CircleSymmetric(center, radius);
    } else if (target_) {
        if (filled_)
            ::ImageDrawCircleV(&target_->Edit(), center, Round(radius), color_);
        else
            ::ImageDrawCircleLinesV(&target_->Edit(), center, Round(radius), color_);
    } else {
        if (filled_)
            ::DrawCircleV(center, radius, color_);
        else
            ::DrawCircleLinesV(center, radius, color_);
    }
    return *this;
}

// Okrąg: x = x0 + R*cos(a), y = y0 + R*sin(a), z ośmiokrotną symetrią.
// Liczymy tylko a z <0, pi/4>, a pozostałe 7 punktów powstaje przez zamianę x i y
// oraz zmianę znaków. Krok kąta ~1/R sprawia, że sąsiednie punkty dzieli najwyżej 1 piksel.
// Wypełnienie: zamiast 8 punktów rysujemy 4 poziome odcinki między punktami symetrycznymi.
void PrimitiveRenderer::CircleSymmetric(Vector2 center, float radius) {
    const int cx = Round(center.x);
    const int cy = Round(center.y);
    const int steps = std::max(1, static_cast<int>(std::ceil(radius * PI / 4.0f)));

    for (int i = 0; i <= steps; ++i) {
        const float angle = (PI / 4.0f) * static_cast<float>(i) / static_cast<float>(steps);
        const int x = Round(radius * std::cos(angle));
        const int y = Round(radius * std::sin(angle));

        if (filled_) {
            Span(cy + y, cx - x, cx + x);
            Span(cy - y, cx - x, cx + x);
            Span(cy + x, cx - y, cx + y);
            Span(cy - x, cx - y, cx + y);
        } else {
            Plot(cx + x, cy + y);
            Plot(cx - x, cy + y);
            Plot(cx + x, cy - y);
            Plot(cx - x, cy - y);
            Plot(cx + y, cy + x);
            Plot(cx - y, cy + x);
            Plot(cx + y, cy - x);
            Plot(cx - y, cy - x);
        }
    }
}

PrimitiveRenderer& PrimitiveRenderer::Ellipse(Vector2 center, Vector2 radius) {
    // raylib nie ma funkcji rysującej elipsę na Image, więc na bitmapie zawsze własny algorytm
    if (symmetric_ || target_) {
        EllipseSymmetric(center, radius);
    } else {
        if (filled_)
            ::DrawEllipse(Round(center.x), Round(center.y), radius.x, radius.y, color_);
        else
            ::DrawEllipseLines(Round(center.x), Round(center.y), radius.x, radius.y, color_);
    }
    return *this;
}

// Elipsa: x = x0 + RX*cos(a), y = y0 + RY*sin(a), z czterokrotną symetrią - liczymy
// a z <0, pi/2>, resztę dają zmiany znaków (8-krotna symetria działa tylko dla okręgu).
// Krok kąta ~1/max(RX, RY), żeby punkty się stykały także wzdłuż dłuższej osi.
void PrimitiveRenderer::EllipseSymmetric(Vector2 center, Vector2 radius) {
    const int cx = Round(center.x);
    const int cy = Round(center.y);
    const float longer = std::max(radius.x, radius.y);
    const int steps = std::max(1, static_cast<int>(std::ceil(longer * PI / 2.0f)));

    for (int i = 0; i <= steps; ++i) {
        const float angle = (PI / 2.0f) * static_cast<float>(i) / static_cast<float>(steps);
        const int x = Round(radius.x * std::cos(angle));
        const int y = Round(radius.y * std::sin(angle));

        if (filled_) {
            Span(cy + y, cx - x, cx + x);
            Span(cy - y, cx - x, cx + x);
        } else {
            Plot(cx + x, cy + y);
            Plot(cx - x, cy + y);
            Plot(cx + x, cy - y);
            Plot(cx - x, cy - y);
        }
    }
}

PrimitiveRenderer& PrimitiveRenderer::Triangle(Vector2 a, Vector2 b, Vector2 c) {
    if (filled_) {
        // raylib wymaga wierzchołków przeciwnie do ruchu wskazówek zegara
        const float cross = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
        if (cross > 0.0f) std::swap(b, c);
        if (target_)
            ::ImageDrawTriangle(&target_->Edit(), a, b, c, color_);
        else
            ::DrawTriangle(a, b, c, color_);
    } else {
        if (target_)
            ::ImageDrawTriangleLines(&target_->Edit(), a, b, c, color_);
        else
            ::DrawTriangleLines(a, b, c, color_);
    }
    return *this;
}

// ------------------------------------------------------------
// Wersje tablicowe
// ------------------------------------------------------------

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

// ------------------------------------------------------------
// Bitmapy
// ------------------------------------------------------------

PrimitiveRenderer& PrimitiveRenderer::Blit(const Bitmap& bitmap, Vector2 position) {
    ::DrawTextureV(bitmap.Texture(), position, WHITE);
    return *this;
}
