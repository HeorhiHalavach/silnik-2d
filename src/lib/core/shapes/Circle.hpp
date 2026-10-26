#pragma once

#include "../Shape.hpp"

// pozycja = środek
class Circle : public Shape<Circle> {
  public:
    Circle& Radius(float radius) {
        radius_ = radius;
        return *this;
    }
    // własny algorytm z 8-krotną symetrią zamiast raylib
    Circle& Symmetric(bool value = true) {
        symmetric_ = value;
        return *this;
    }

    bool Contains(Vector2 point) const override {
        return ::CheckCollisionPointCircle(point, WorldPosition(), radius_);
    }

  protected:
    void Draw(PrimitiveRenderer& renderer) const override {
        renderer.Paint(color_).Fill(filled_).Symmetric(symmetric_).Circle(WorldPosition(), radius_);
    }

  private:
    float radius_ = 10.0f;
    bool symmetric_ = false;
};
