#include <cmath>

#include "lib/Engine.hpp"

SCENE("menu") {
    scene.Spawn<Rect>().Position({300, 200}).Size({400, 300}).Paint(DARKBLUE);

    // piłka z własną logiką ruchu
    scene.Spawn<Circle>()
        .Position({500, 440})
        .Radius(20)
        .Paint(ORANGE)
        .Fill()
        .OnUpdate([time = 0.0f](Circle& self, float dt) mutable {
            time += dt;
            self.Position({500 + 150 * std::sin(time * 2), 440});
        });

    // przycisk: podświetla się pod kursorem, kliknięcie przełącza scenę
    scene.Spawn<Rect>()
        .Position({400, 300})
        .Size({200, 60})
        .Paint(DARKBLUE)
        .OnMouseEnter([](Rect& self) { self.Fill(); })
        .OnMouseLeave([](Rect& self) { self.Fill(false); })
        .OnClick([](Rect&) { Engine::Get().SwitchScene("prymitywy"); });

    scene.OnDraw([](PrimitiveRenderer&) {
        ::DrawText("MENU", 455, 230, 40, DARKBLUE);
        ::DrawText("START", 470, 320, 24, GRAY);
    });
}
