#pragma once

#include <utility>
#include <vector>

#include "../Shape.hpp"
#include "Line.hpp"

// Wiele punktów naraz (wersja tablicowa), położenia względem pozycji obiektu
class Points : public Shape<Points> {
  public:
    Points& Positions(std::vector<Vector2> positions) {
        positions_ = std::move(positions);
        return *this;
    }

    bool Contains(Vector2 point) const override {
        for (const Vector2& position : positions_)
            if (::CheckCollisionPointCircle(point, World(position), Line::kHitDistance))
                return true;
        return false;
    }

  protected:
    void Draw(PrimitiveRenderer& renderer) const override {
        renderer.Paint(color_).Points(WorldVertices(positions_));
    }

  private:
    std::vector<Vector2> positions_;
};
