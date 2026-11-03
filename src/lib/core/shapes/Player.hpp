/**
 * @file Player.hpp
 * @brief Gracz poruszany klawiszami WASD lub strzałkami.
 */
#pragma once

#include <cmath>

#include "../Shape.hpp"

namespace shapes {

/** @brief Trójkątny gracz z ruchem niezależnym od liczby klatek na sekundę. */
class Player : public Shape<Player> {
  public:
    /** @brief Ustawia prędkość gracza w pikselach na sekundę. */
    Player& Speed(float pixelsPerSecond) {
        speed_ = pixelsPerSecond;
        return *this;
    }

    void Update(float dt, const Input& input) override {
        const float x = static_cast<float>(input.KeyDown(KEY_D) || input.KeyDown(KEY_RIGHT)) -
                        static_cast<float>(input.KeyDown(KEY_A) || input.KeyDown(KEY_LEFT));
        const float y = static_cast<float>(input.KeyDown(KEY_S) || input.KeyDown(KEY_DOWN)) -
                        static_cast<float>(input.KeyDown(KEY_W) || input.KeyDown(KEY_UP));
        const float length = std::sqrt(x * x + y * y);
        if (length > 0.0f)
            Translate({x * speed_ * dt / length, y * speed_ * dt / length});
        Entity::Update(dt, input);
    }

    bool Contains(Vector2 point) const override {
        const auto local = TryToLocal(point);
        return local && ::CheckCollisionPointTriangle(*local, kTop, kLeft, kRight);
    }

  protected:
    void Draw(PrimitiveRenderer& renderer) const override {
        renderer.Paint(color_).Fill().Triangle(kTop, kLeft, kRight);
    }

  private:
    static constexpr Vector2 kTop{0.0f, -20.0f};
    static constexpr Vector2 kLeft{-16.0f, 16.0f};
    static constexpr Vector2 kRight{16.0f, 16.0f};
    float speed_ = 220.0f;
};

} // namespace shapes
