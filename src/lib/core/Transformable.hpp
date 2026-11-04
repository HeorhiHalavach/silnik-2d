/**
 * @file Transformable.hpp
 * @brief Wspólna implementacja przesuwania, obracania i skalowania obiektów sceny.
 */
#pragma once

#include "AffineMatrix2D.hpp"

/** @brief Obiekt z lokalną macierzą transformacji względem rodzica. */
class Transformable {
  public:
    virtual ~Transformable() = default;

    /**
     * @brief Przesuwa obiekt w układzie rodzica.
     * @param offset Przesunięcie w układzie rodzica.
     * @return Referencja Transformable& do tego samego obiektu.
     */
    virtual Transformable& Translate(Vector2 offset) {
        transform_.tx += offset.x;
        transform_.ty += offset.y;
        return *this;
    }

    /**
     * @brief Obraca obiekt i jego dzieci wokół punktu w układzie rodzica.
     * @param radians Kąt obrotu w radianach.
     * @param pivot Środek obrotu w układzie rodzica.
     * @return Referencja Transformable& do tego samego obiektu.
     */
    virtual Transformable& Rotate(float radians, Vector2 pivot) {
        transform_ = AffineMatrix2D::Translation(pivot) * AffineMatrix2D::Rotation(radians) *
                     AffineMatrix2D::Translation({-pivot.x, -pivot.y}) * transform_;
        return *this;
    }

    /**
     * @brief Skaluje obiekt i jego dzieci wokół punktu w układzie rodzica.
     * @param factors Mnożniki skali osi X i Y.
     * @param pivot Środek skalowania w układzie rodzica.
     * @return Referencja Transformable& do tego samego obiektu.
     */
    virtual Transformable& Scale(Vector2 factors, Vector2 pivot) {
        transform_ = AffineMatrix2D::Translation(pivot) * AffineMatrix2D::Scaling(factors) *
                     AffineMatrix2D::Translation({-pivot.x, -pivot.y}) * transform_;
        return *this;
    }

  protected:
    AffineMatrix2D transform_; /**< Lokalna macierz obiektu. */
};
