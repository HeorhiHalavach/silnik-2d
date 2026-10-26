#pragma once

#include "../Shape.hpp"

// Pusty obiekt bez kształtu - "uchwyt" do grupowania dzieci (np. cała postać z kilku figur)
class Group : public Shape<Group> {
  public:
    bool Contains(Vector2) const override { return false; }

  protected:
    void Draw(PrimitiveRenderer&) const override {}
};
