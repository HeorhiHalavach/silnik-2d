#pragma once

#include <raylib.h>

// Bitmapa w pamięci RAM. W odróżnieniu od ekranu można z niej odczytywać kolory pikseli,
// czego wymagają algorytmy wypełniania przez spójność (boundary fill, flood fill).
// Na ekran trafia jako tekstura, wysyłana na kartę graficzną dopiero przy rysowaniu
// i tylko wtedy, gdy zawartość się zmieniła.
//
//   Bitmap bitmap{400, 300, WHITE};
//   renderer.Target(bitmap).Paint(BLACK).Circle({200, 150}, 80).TargetScreen();
//   renderer.Blit(bitmap, {100, 100});
class Bitmap {
  public:
    Bitmap() = default;
    Bitmap(int width, int height, Color background = BLANK);
    ~Bitmap();

    Bitmap(const Bitmap&) = delete;
    Bitmap& operator=(const Bitmap&) = delete;
    Bitmap(Bitmap&& other) noexcept;
    Bitmap& operator=(Bitmap&& other) noexcept;

    int Width() const { return image_.width; }
    int Height() const { return image_.height; }
    bool Contains(int x, int y) const { return x >= 0 && y >= 0 && x < Width() && y < Height(); }

    // kolor piksela; poza bitmapą zwraca BLANK, a zapis poza nią jest ignorowany
    Color Pixel(int x, int y) const;
    Bitmap& Pixel(int x, int y, Color color);

    Bitmap& Clear(Color color);

    // dostęp do obrazu dla funkcji raylib ImageDraw*; oznacza bitmapę jako zmienioną
    Image& Edit() {
        dirty_ = true;
        return image_;
    }

    // tekstura z aktualną zawartością - wymaga otwartego okna
    const Texture2D& Texture() const;

  private:
    void Release();

    Image image_{};
    mutable Texture2D texture_{};
    mutable bool dirty_ = true;
};
