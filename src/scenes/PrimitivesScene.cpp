#include "lib/Engine.hpp"

SCENE("prymitywy") {
    // prostokąt sterowany strzałkami, L przenosi go nad / pod błękitny
    scene.Spawn<Rect>()
        .Position({40, 60})
        .Size({120, 80})
        .Paint(DARKBLUE)
        .Fill()
        .OnUpdate([](Rect& self, float dt) {
            const Engine& engine = Engine::Get();
            Vector2 position = self.Position();
            const float speed = 200 * dt;
            if (engine.KeyDown(KEY_RIGHT)) position.x += speed;
            if (engine.KeyDown(KEY_LEFT)) position.x -= speed;
            if (engine.KeyDown(KEY_DOWN)) position.y += speed;
            if (engine.KeyDown(KEY_UP)) position.y -= speed;
            self.Position(position);
        })
        .OnKeyPressed(KEY_L, [](Rect& self) { self.Layer(self.Layer() == 0 ? 1 : 0); });

    scene.Spawn<Rect>().Position({190, 60}).Size({120, 80}).Paint(SKYBLUE).Fill();

    // okrąg zmienia kolor pod kursorem
    scene.Spawn<Circle>()
        .Position({100, 260})
        .Radius(50)
        .Paint(ORANGE)
        .OnMouseEnter([](Circle& self) { self.Paint(RED); })
        .OnMouseLeave([](Circle& self) { self.Paint(ORANGE); });

    // koło: kliknięcie przełącza wypełnienie
    scene.Spawn<Circle>()
        .Position({250, 260})
        .Radius(50)
        .Paint(ORANGE)
        .Fill()
        .OnClick([filled = true](Circle& self) mutable { self.Fill(filled = !filled); });

    scene.Spawn<Triangle>()
        .Vertices({600, 80}, {540, 200}, {680, 200})
        .Paint(DARKGREEN)
        .OnClick([](Triangle& self) { self.Fill(); });

    scene.Spawn<Triangle>().Vertices({740, 80}, {680, 200}, {800, 200}).Paint(LIME).Fill();

    scene.Spawn<Polyline>()
        .Vertices({{450, 80}, {520, 130}, {495, 210}, {405, 210}, {380, 130}})
        .Closed()
        .Paint(PURPLE)
        .OnMouseEnter([](Polyline& self) { self.Paint(MAGENTA); })
        .OnMouseLeave([](Polyline& self) { self.Paint(PURPLE); });

    // porównanie: algorytm przyrostowy (czerwony) i raylib (niebieski)
    scene.Spawn<Line>().From({100, 400}).To({400, 550}).Incremental().Paint(RED);
    scene.Spawn<Line>().From({500, 400}).To({800, 550}).Paint(BLUE);

    // odcinek: PPM przesuwa koniec, SPACJA przełącza algorytm
    scene.Spawn<Line>()
        .From({500, 300})
        .To({700, 300})
        .Incremental()
        .Paint(RED)
        .OnUpdate([](Line& self, float) {
            const Engine& engine = Engine::Get();
            if (engine.MouseDown(MOUSE_BUTTON_RIGHT)) self.To(engine.MousePosition());
        })
        .OnKeyPressed(KEY_SPACE, [incremental = true](Line& self) mutable {
            incremental = !incremental;
            self.Incremental(incremental).Paint(incremental ? RED : BLUE);
        });

    scene.OnDraw([](PrimitiveRenderer&) {
        ::DrawText("strzalki - prostokat, L - warstwa, PPM - odcinek, "
                   "SPACJA - algorytm, LPM - klik",
                   20, 630, 18, GRAY);
    });
}
