/**
 * @file TransformScene.cpp
 * @brief Pokaz przekształceń figur i sterowania graczem z laboratorium 4.
 */

#include "lib/core/Engine.hpp"

SCENE("transformacje") {
    auto& rectangle = scene.Spawn<shapes::Rect>();
    rectangle.Position({170, 150}).Size({170, 90}).Paint(DARKBLUE).Fill();
    rectangle.Rotate(PI / 6.0f, {255, 195});
    rectangle.OnClick([](shapes::Rect& self) { self.Paint(RED); });

    auto& circle = scene.Spawn<shapes::Circle>();
    circle.Position({650, 200}).Radius(65).Paint(SKYBLUE).Fill();
    circle.Scale({1.5f, 0.6f}, {650, 200});

    auto& group = scene.Spawn<shapes::Group>();
    group.Position({200, 440});
    group.Spawn<shapes::Rect>().Position({-50, -30}).Size({100, 60}).Paint(GOLD).Fill();
    group.Spawn<shapes::Circle>().Position({80, 0}).Radius(25).Paint(ORANGE).Fill();
    group.Rotate(PI / 5.0f, {200, 440});

    scene.Spawn<shapes::Player>().Position({500, 430}).Paint(DARKGREEN).Speed(240.0f);

    scene.OnDraw([](PrimitiveRenderer&) {
        ::DrawText("LAB 4: przeksztalcenia 2D", 30, 30, 30, DARKGRAY);
        ::DrawText("Obrocony prostokat: kliknij, aby zmienic kolor", 80, 300, 18, DARKGRAY);
        ::DrawText("Kolo po skalowaniu niejednorodnym", 490, 300, 18, DARKGRAY);
        ::DrawText("Grupa z obroconymi dziecmi", 80, 560, 18, DARKGRAY);
        ::DrawText("Gracz: WASD lub strzalki", 390, 560, 18, DARKGRAY);
    });
}
