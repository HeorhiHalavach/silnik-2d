#pragma once

#include <vector>

#include "../../render/Geometry.hpp"
#include "../Shape.hpp"

// Wielokąt (łamana zwyczajna zamknięta), wierzchołki względem pozycji obiektu.
// Łamana wiązana (przecinające się krawędzie) nie jest rysowana - patrz Simple().
// Fill() wypełnia go algorytmem scanline z kontrolą parzystości.
class Polygon : public Shape<Polygon> {
  public:
    Polygon& Vertices(std::vector<Point2D> vertices) {
        vertices_ = std::move(vertices);
        return *this;
    }
    // zbiór odcinków tworzących zamknięty łańcuch; inny zbiór daje pusty wielokąt
    Polygon& Edges(const std::vector<LineSegment>& edges) {
        vertices_ = PolygonVertices(edges).value_or(std::vector<Point2D>{});
        return *this;
    }
    Polygon& Incremental(bool value = true) {
        incremental_ = value;
        return *this;
    }

    bool Simple() const { return IsSimplePolygon(vertices_); }

    bool Contains(Vector2 point) const override {
        if (!Simple()) return false;
        std::vector<Vector2> world;
        for (const Point2D& vertex : WorldVertices(vertices_))
            world.push_back(vertex);
        return ::CheckCollisionPointPoly(point, world.data(), static_cast<int>(world.size()));
    }

  protected:
    void Draw(PrimitiveRenderer& renderer) const override {
        renderer.Paint(color_)
            .Fill(filled_)
            .Incremental(incremental_)
            .Polygon(WorldVertices(vertices_));
    }

  private:
    std::vector<Point2D> vertices_;
    bool incremental_ = false;
};
