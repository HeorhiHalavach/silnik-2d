/**
 * @file Updatable.hpp
 * @brief Aktualizacja obiektu, obsługa wejścia i aktualizacja dzieci.
 */
#pragma once

#include <cstddef>
#include <functional>
#include <utility>
#include <vector>

#include "Input.hpp"

/** @brief Obiekt aktualizowany co klatkę wraz ze swoimi dziećmi. */
class Updatable {
  public:
    virtual ~Updatable() = default;

    /**
     * @brief Obsługuje zdarzenia obiektu, a następnie aktualizuje dzieci.
     * @param dt Czas od poprzedniej klatki w sekundach.
     * @param input Stan klawiatury i myszy.
     */
    virtual void Update(float dt, const Input& input) {
        HandleEvents(dt, input);
        // Zdarzenie może dodać dziecko, dlatego rozmiar jest sprawdzany w każdej iteracji.
        for (std::size_t i = 0; i < UpdateChildCount(); ++i)
            UpdateChild(i).Update(dt, input);
    }

    /** @brief Zwraca zapamiętany stan najechania kursora. */
    bool Hovered() const { return hovered_; }

  protected:
    /** @brief Sprawdza trafienie kursorem; geometrię dostarcza obiekt pochodny. */
    virtual bool HitTest(Vector2 point) const = 0;
    /** @brief Zwraca bieżącą liczbę dzieci do aktualizacji. */
    virtual std::size_t UpdateChildCount() const = 0;
    /** @brief Udostępnia dziecko o podanym indeksie do aktualizacji. */
    virtual Updatable& UpdateChild(std::size_t index) = 0;

    std::function<void(float)> update_; /**< Callback wykonywany co klatkę. */
    std::function<void()> click_;       /**< Reakcja na kliknięcie. */
    std::function<void()> mouseEnter_;  /**< Reakcja na wejście kursora. */
    std::function<void()> mouseLeave_;  /**< Reakcja na wyjście kursora. */
    std::vector<std::pair<int, std::function<void()>>> keys_; /**< Reakcje na klawisze. */

  private:
    /** @brief Wywołuje callbacki myszy, klawiatury i aktualizacji. */
    void HandleEvents(float dt, const Input& input) {
        if (input.MouseEnabled()) {
            const bool inside = HitTest(input.MousePosition());
            if (inside && !hovered_ && mouseEnter_) mouseEnter_();
            if (!inside && hovered_ && mouseLeave_) mouseLeave_();
            hovered_ = inside;
            if (inside && input.MousePressed(MOUSE_BUTTON_LEFT) && click_) click_();
        }
        for (const auto& [key, callback] : keys_)
            if (input.KeyPressed(key)) callback();
        if (update_) update_(dt);
    }

    bool hovered_ = false; /**< Ostatni zapamiętany stan najechania kursora. */
};
