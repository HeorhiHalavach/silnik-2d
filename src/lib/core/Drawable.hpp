/**
 * @file Drawable.hpp
 * @brief Rysowanie obiektu i jego dzieci z uwzględnieniem transformacji i warstw.
 */
#pragma once

#include <raylib.h>
#include <rlgl.h>

#include <algorithm>
#include <vector>

#include "../render/PrimitiveRenderer.hpp"
#include "AffineMatrix2D.hpp"

/** @brief Obiekt rysowany razem z dziećmi w kolejności warstw. */
class Drawable {
  public:
    virtual ~Drawable() = default;

    /**
     * @brief Nakłada lokalną macierz, rysuje obiekt i dzieci, po czym przywraca macierz.
     * @details Draw() przekazuje rendererowi współrzędne lokalne; złożone macierze rodziców
     * i dzieci przenoszą je na ekran. Każde dziecko rozpoczyna z czystym pędzlem.
     * @param renderer Renderer prymitywów; metoda ustawia jego cel na ekran.
     */
    virtual void Render(PrimitiveRenderer& renderer) const {
        renderer.Reset().TargetScreen();
        const auto matrix = LocalTransform().RlglMatrix();
        rlPushMatrix();
        rlMultMatrixf(matrix.data());
        Draw(renderer);
        for (const Drawable* child : ByLayer(DrawableChildren()))
            child->Render(renderer);
        rlPopMatrix();
    }

    /**
     * @brief Pobiera warstwę rysowania wśród obiektów tego samego rodzica.
     * @return Numer warstwy; większa wartość oznacza rysowanie później.
     */
    int Layer() const { return layer_; }

    /**
     * @brief Ustawia warstwę wśród obiektów tego samego rodzica.
     * @param layer Numer warstwy.
     * @return Ten sam obiekt jako jego konkretny typ (deducing this, C++23).
     */
    template <typename Self> Self& Layer(this Self& self, int layer) {
        self.layer_ = layer;
        return self;
    }

    /**
     * @brief Ustawia kolor rysowania.
     * @param color Kolor.
     * @return Ten sam obiekt jako jego konkretny typ.
     */
    template <typename Self> Self& Paint(this Self& self, Color color) {
        self.color_ = color;
        return self;
    }

    /**
     * @brief Włącza lub wyłącza wypełnienie figury.
     * @details Działa w Rect, Triangle, Circle, Ellipse i Polygon; pozostałe kształty
     * je ignorują.
     * @param value Czy figura ma być wypełniona.
     * @return Ten sam obiekt jako jego konkretny typ.
     */
    template <typename Self> Self& Fill(this Self& self, bool value = true) {
        self.filled_ = value;
        return self;
    }

    /**
     * @brief Ustawia obiekty od najniższej warstwy, zachowując kolejność dodania.
     * @param objects Obiekty o wspólnym rodzicu.
     * @return Wskaźniki w kolejności rysowania.
     */
    static std::vector<const Drawable*> ByLayer(std::vector<const Drawable*> objects) {
        std::stable_sort(objects.begin(), objects.end(),
                         [](const Drawable* a, const Drawable* b) {
                             return a->Layer() < b->Layer();
                         });
        return objects;
    }

  protected:
    /** @brief Rysuje sam obiekt bez dzieci. */
    virtual void Draw(PrimitiveRenderer& renderer) const = 0;
    virtual AffineMatrix2D LocalTransform() const = 0;
    virtual std::vector<const Drawable*> DrawableChildren() const = 0;

    Color color_ = BLACK; /**< Kolor figury. */
    bool filled_ = false; /**< Czy figura jest wypełniona. */
    int layer_ = 0;       /**< Warstwa wśród rodzeństwa. */
};
