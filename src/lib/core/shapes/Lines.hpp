#pragma once

#include <utility>
#include <vector>

#include "../Shape.hpp"
#include "Line.hpp"

// Wiele niezależnych odcinków (wersja tablicowa): pary wierzchołków (0,1), (2,3), ...
class Lines : public Shape<Lines> {
  public:
    Lines& Vertices(std::vector<Vector2> vertices) {
        vertices_ = std::move(vertices);
        return *this;
    }
    Lines& Incremental(bool value = true) {
        incremental_ = value;
        return *this;
    }

    bool Contains(Vector2 point) const override {
        for (std::size_t i = 0; i + 1 < vertices_.size(); i += 2)
            if (::CheckCollisionPointLine(point, World(vertices_[i]), World(vertices_[i + 1]),
                                          Line::kHitDistance))
                return true;
        return false;
    }

  protected:
    void Draw(PrimitiveRenderer& renderer) const override {
        renderer.Paint(color_).Incremental(incremental_).Lines(WorldVertices(vertices_));
    }

  private:
    std::vector<Vector2> vertices_;
    bool incremental_ = false;
};
