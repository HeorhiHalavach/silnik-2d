/**
 * @file TransformScene.cpp
 * @brief Pokaz figur przed i po transformacji oraz sterowania graczem z laboratorium 4.
 */

#include "lib/core/Engine.hpp"

/** @brief Tworzy trzy porównania transformacji i sterowany statek. */
SCENE("transformacje") {
    // Warstwa 1 rysuje szare kontury stanu przed transformacją nad kolorowymi figurami.
    scene.Spawn<shapes::Rect>()
        .Position({170, 150}).Size({170, 90}).Paint(GRAY).Layer(1);
    auto& rectangle = scene.Spawn<shapes::Rect>();
    rectangle.Position({170, 150}).Size({170, 90}).Paint(DARKBLUE).Fill();
    rectangle.Rotate(PI / 6.0f, {255, 195});
    rectangle.OnClick([](shapes::Rect& self) { self.Paint(RED); });

    scene.Spawn<shapes::Circle>()
        .Position({650, 200}).Radius(65).Paint(GRAY).Layer(1);
    auto& circle = scene.Spawn<shapes::Circle>();
    circle.Position({650, 200}).Radius(65).Paint(SKYBLUE).Fill();
    circle.Scale({1.5f, 0.6f}, {650, 200});

    auto& originalGroup = scene.Spawn<shapes::Group>();
    originalGroup.Position({200, 440}).Layer(1);
    originalGroup.Spawn<shapes::Rect>().Position({-50, -30}).Size({100, 60}).Paint(GRAY);
    originalGroup.Spawn<shapes::Circle>().Position({80, 0}).Radius(25).Paint(GRAY);

    auto& group = scene.Spawn<shapes::Group>();
    group.Position({200, 440});
    group.Spawn<shapes::Rect>().Position({-50, -30}).Size({100, 60}).Paint(GOLD).Fill();
    group.Spawn<shapes::Circle>().Position({80, 0}).Radius(25).Paint(ORANGE).Fill();
    group.Rotate(PI / 5.0f, {200, 440});

    auto& player = scene.Spawn<shapes::Player>();
    player.Position({650, 440}).Paint(DARKGREEN).Speed(240.0f);
    player.Scale({1.6f, 1.6f}, {650, 440});

    // Podpisy i punkty obrotu/skalowania są rysowane już we współrzędnych ekranu.
    scene.OnDraw([](PrimitiveRenderer&) {
        ::DrawText("Szary kontur = PRZED     Kolor = PO", 30, 70, 20, DARKGRAY);
        ::DrawText("Obrot prostokata o 30 stopni (kliknij)", 80, 300, 18, DARKGRAY);
        ::DrawText("Skalowanie kola: X 1.5, Y 0.6", 520, 300, 18, DARKGRAY);
        ::DrawText("Obrot grupy razem z dziecmi", 80, 560, 18, DARKGRAY);
        ::DrawText("Statek: WASD lub strzalki", 520, 560, 18, DARKGRAY);
        ::DrawCircle(255, 195, 3, MAROON);
        ::DrawCircle(650, 200, 3, MAROON);
        ::DrawCircle(200, 440, 3, MAROON);
    });
}
