#pragma once

#include "../Shape.hpp"
#include "Line.hpp"

// Punkt w pozycji obiektu
class Point : public Shape<Point> {
  public:
    bool Contains(Vector2 point) const override {
        return ::CheckCollisionPointCircle(point, WorldPosition(), Line::kHitDistance);
    }

  protected:
    void Draw(PrimitiveRenderer& renderer) const override {
        renderer.Paint(color_).Point(WorldPosition());
    }
};
