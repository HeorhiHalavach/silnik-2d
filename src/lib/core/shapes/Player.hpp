/**
 * @file Player.hpp
 * @brief Gracz w postaci statku sterowanego klawiszami WASD lub strzałkami.
 */
#pragma once

#include <cmath>

#include "../Entity.hpp"

namespace shapes {

/**
 * @brief Statek obracający się w kierunku ruchu.
 * @details Kadłub, skrzydła i kabina powstają z prymitywów renderera.
 * Ruch uwzględnia czas klatki i ma jednakową prędkość także po przekątnej.
 */
class Player : public Entity {
  public:
    /**
     * @brief Ustawia prędkość gracza w pikselach na sekundę.
     * @param pixelsPerSecond Prędkość ruchu w układzie rodzica.
     * @return Ten sam gracz.
     */
    Player& Speed(float pixelsPerSecond) {
        speed_ = pixelsPerSecond;
        return *this;
    }

    /**
     * @brief Odczytuje kierunek ruchu, obraca statek i aktualizuje jego dzieci.
     * @details Wektor wejścia jest normalizowany przed przesunięciem. Wywołanie
     * Updatable::Update() zachowuje zdarzenia i aktualizację dzieci.
     * @param dt Czas od poprzedniej klatki w sekundach.
     * @param input Stan klawiatury i myszy.
     */
    void Update(float dt, const Input& input) override {
        const float x = static_cast<float>(input.KeyDown(KEY_D) || input.KeyDown(KEY_RIGHT)) -
                        static_cast<float>(input.KeyDown(KEY_A) || input.KeyDown(KEY_LEFT));
        const float y = static_cast<float>(input.KeyDown(KEY_S) || input.KeyDown(KEY_DOWN)) -
                        static_cast<float>(input.KeyDown(KEY_W) || input.KeyDown(KEY_UP));
        const float length = std::sqrt(x * x + y * y);
        moving_ = length > 0.0f;
        if (moving_) {
            const float direction = std::atan2(y, x) + PI / 2.0f;
            Rotate(direction - heading_, Position());
            heading_ = direction;
            Translate({x * speed_ * dt / length, y * speed_ * dt / length});
            flameTime_ += dt;
        }
        Updatable::Update(dt, input);
    }

    /**
     * @brief Sprawdza trafienie w kadłub statku, bez skrzydeł i płomienia.
     * @param point Punkt w układzie ekranu.
     * @return true, gdy punkt należy do kadłuba.
     */
    bool Contains(Vector2 point) const override {
        const auto local = TryToLocal(point);
        return local && ::CheckCollisionPointTriangle(*local, kTop, kLeft, kRight);
    }

  protected:
    /**
     * @brief Rysuje lokalnie płomień, skrzydła, kadłub, silnik i kabinę.
     * @param renderer Renderer prymitywów z nałożoną transformacją gracza.
     */
    void Draw(PrimitiveRenderer& renderer) const override {
        if (moving_) {
            const float flameLength = 27.0f + 4.0f * std::sin(flameTime_ * 18.0f);
            renderer.Paint(ORANGE).Fill().Triangle({-7, 13}, {7, 13}, {0, flameLength});
        }
        renderer.Paint(DARKBLUE).Fill().Triangle({-8, 5}, {-19, 18}, {-4, 15});
        renderer.Triangle({8, 5}, {19, 18}, {4, 15});
        renderer.Paint(color_).Fill().Triangle(kTop, kLeft, kRight);
        renderer.Paint(GOLD).Fill().Rect({-6, 12}, {12, 4});
        renderer.Paint(SKYBLUE).Fill().Circle({0, -4}, 6);
        renderer.Paint(WHITE).Fill().Circle({-2, -6}, 2);
    }

  private:
    static constexpr Vector2 kTop{0.0f, -20.0f};
    static constexpr Vector2 kLeft{-16.0f, 16.0f};
    static constexpr Vector2 kRight{16.0f, 16.0f};
    float speed_ = 220.0f;
    float heading_ = 0.0f;   /**< Kąt statku względem kierunku do góry. */
    float flameTime_ = 0.0f; /**< Czas animacji płomienia. */
    bool moving_ = false;    /**< Czy w bieżącej klatce wciśnięto klawisz ruchu. */
};

} // namespace shapes
