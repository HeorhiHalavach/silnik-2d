/**
 * @file ObjectInterfaces.hpp
 * @brief Podstawowe możliwości obiektów silnika.
 */
#pragma once

#include <raylib.h>

class Input;
class PrimitiveRenderer;

/**
 * @brief Wspólna baza obiektów silnika.
 *
 * Destruktor jest wirtualny, aby można było poprawnie usuwać
 * obiekt klasy pochodnej przez wskaźnik do GameObject.
 */
class GameObject {
  public:
    virtual ~GameObject() = default;
};

/** @brief Obiekt, którego logika może być aktualizowana co klatkę. */
class UpdatableObject : public virtual GameObject {
  public:
    virtual void Update(float dt, const Input& input) = 0;
};

/** @brief Obiekt, który może zostać narysowany. */
class DrawableObject : public virtual GameObject {
  public:
    virtual void Render(PrimitiveRenderer& renderer) const = 0;
};

/** @brief Obiekt podlegający przekształceniom geometrycznym 2D. */
class TransformableObject : public virtual GameObject {
  public:
    virtual void Translate(Vector2 offset) = 0;
    virtual void Rotate(float radians, Vector2 pivot) = 0;
    virtual void Scale(Vector2 factors, Vector2 pivot) = 0;
};

/** @brief Figura, którą można narysować i przekształcić. */
class ShapeObject : public virtual DrawableObject,
                    public virtual TransformableObject {};
