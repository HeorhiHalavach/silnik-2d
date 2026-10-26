#pragma once

#include "../Shape.hpp"

// pozycja = środek, promień = {poziomy, pionowy}
class Ellipse : public Shape<Ellipse> {
  public:
    Ellipse& Radius(Vector2 radius) {
        radius_ = radius;
        return *this;
    }
    // własny algorytm z 4-krotną symetrią zamiast raylib
    Ellipse& Symmetric(bool value = true) {
        symmetric_ = value;
        return *this;
    }

    bool Contains(Vector2 point) const override {
        const Vector2 local = ToLocal(point);
        const float x = local.x / radius_.x;
        const float y = local.y / radius_.y;
        return x * x + y * y <= 1.0f;
    }

  protected:
    void Draw(PrimitiveRenderer& renderer) const override {
        renderer.Paint(color_)
            .Fill(filled_)
            .Symmetric(symmetric_)
            .Ellipse(WorldPosition(), radius_);
    }

  private:
    Vector2 radius_{20, 10};
    bool symmetric_ = false;
};
