/**
 * @file Geometry.cpp
 * @brief Test przecięcia odcinków i sprawdzanie obwodu wielokąta.
 * @details Test orientacji i przypadki współliniowe wykrywają wspólne punkty
 * niesąsiednich krawędzi.
 */

#include "Geometry.hpp"

#include <algorithm>
#include <cmath>

#include "PrimitiveRenderer.hpp"

namespace {

/**
 * @brief Iloczyn wektorowy (b - a) x (c - a).
 * @return > 0 - c po lewej od prostej ab, < 0 - po prawej, 0 - na prostej.
 */
float Cross(Point2D a, Point2D b, Point2D c) {
    return (b.X() - a.X()) * (c.Y() - a.Y()) - (b.Y() - a.Y()) * (c.X() - a.X());
}

/** @brief Znak liczby: -1, 0 albo 1. */
int Sign(float value) {
    return (value > 0.0f) - (value < 0.0f);
}

/**
 * @brief Dla c leżącego na prostej ab sprawdza, czy leży też w prostokącie
 * ograniczającym odcinek ab.
 */
bool OnSegment(Point2D a, Point2D b, Point2D c) {
    return std::min(a.X(), b.X()) <= c.X() && c.X() <= std::max(a.X(), b.X()) &&
           std::min(a.Y(), b.Y()) <= c.Y() && c.Y() <= std::max(a.Y(), b.Y());
}

} // namespace

void Point2D::Render(PrimitiveRenderer& renderer) const {
    renderer.Point(*this);
}

void Point2D::Translate(Vector2 offset) {
    x_ += offset.x;
    y_ += offset.y;
}

void Point2D::Rotate(float radians, Vector2 pivot) {
    const float dx = x_ - pivot.x;
    const float dy = y_ - pivot.y;
    const float cosine = std::cos(radians);
    const float sine = std::sin(radians);

    x_ = pivot.x + dx * cosine - dy * sine;
    y_ = pivot.y + dx * sine + dy * cosine;
}

void Point2D::Scale(Vector2 factors, Vector2 pivot) {
    x_ = pivot.x + (x_ - pivot.x) * factors.x;
    y_ = pivot.y + (y_ - pivot.y) * factors.y;
}

/**
 * @details Klasyczny test orientacji: odcinki AB i CD przecinają się, gdy C i D leżą po
 * różnych stronach AB, a A i B po różnych stronach CD. Przypadki współliniowe są
 * sprawdzane osobno.
 */
bool LineSegment::Intersects(const LineSegment& other) const {
    const Point2D a = from_, b = to_, c = other.from_, d = other.to_;
    const int d1 = Sign(Cross(a, b, c));
    const int d2 = Sign(Cross(a, b, d));
    const int d3 = Sign(Cross(c, d, a));
    const int d4 = Sign(Cross(c, d, b));

    if (d1 * d2 < 0 && d3 * d4 < 0) return true;

    return (d1 == 0 && OnSegment(a, b, c)) || (d2 == 0 && OnSegment(a, b, d)) ||
           (d3 == 0 && OnSegment(c, d, a)) || (d4 == 0 && OnSegment(c, d, b));
}

void LineSegment::Render(PrimitiveRenderer& renderer) const {
    renderer.Line(from_, to_);
}

void LineSegment::Translate(Vector2 offset) {
    from_.Translate(offset);
    to_.Translate(offset);
}

void LineSegment::Rotate(float radians, Vector2 pivot) {
    from_.Rotate(radians, pivot);
    to_.Rotate(radians, pivot);
}

void LineSegment::Scale(Vector2 factors, Vector2 pivot) {
    from_.Scale(factors, pivot);
    to_.Scale(factors, pivot);
}

bool IsSimplePolygon(std::span<const Point2D> vertices) {
    const std::size_t count = vertices.size();
    if (count < 3) return false;

    for (std::size_t i = 0; i < count; ++i) {
        const LineSegment edge{vertices[i], vertices[(i + 1) % count]};
        // krawędzie sąsiednie zawsze stykają się we wspólnym wierzchołku - pomijamy je
        for (std::size_t j = i + 2; j < count; ++j) {
            if (i == 0 && j == count - 1) continue; // pierwsza i ostatnia też są sąsiednie
            const LineSegment other{vertices[j], vertices[(j + 1) % count]};
            if (edge.Intersects(other)) return false;
        }
    }
    return true;
}

std::optional<std::vector<Point2D>> PolygonVertices(std::span<const LineSegment> edges) {
    if (edges.empty()) return std::nullopt;

    std::vector<Point2D> vertices;
    vertices.reserve(edges.size());
    for (std::size_t i = 0; i < edges.size(); ++i) {
        const LineSegment& next = edges[(i + 1) % edges.size()];
        if (edges[i].To() != next.From()) return std::nullopt; // łańcuch się nie domyka
        vertices.push_back(edges[i].From());
    }
    return vertices;
}
