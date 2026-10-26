#pragma once

#include "../Shape.hpp"

// From / To względem pozycji obiektu
class Line : public Shape<Line> {
  public:
    Line& From(Vector2 position) {
        from_ = position;
        return *this;
    }
    Line& To(Vector2 position) {
        to_ = position;
        return *this;
    }
    Line& Incremental(bool value = true) {
        incremental_ = value;
        return *this;
    }

    bool Contains(Vector2 point) const override {
        return ::CheckCollisionPointLine(point, World(from_), World(to_), kHitDistance);
    }

    static constexpr int kHitDistance = 4; // tolerancja trafienia w odcinek [px]

  protected:
    void Draw(PrimitiveRenderer& renderer) const override {
        renderer.Paint(color_).Incremental(incremental_).Line(World(from_), World(to_));
    }

  private:
    Vector2 from_{};
    Vector2 to_{};
    bool incremental_ = false;
};
