#include "Adafruit_GFX.h"

#ifdef __AVR__
// Bitmask tables of 0x80>>X and ~(0x80>>X), because X>>Y is slow on AVR
const uint8_t PROGMEM Canvas1setBit[] = {0x80, 0x40, 0x20, 0x10,
                                                0x08, 0x04, 0x02, 0x01};
const uint8_t PROGMEM Canvas1clrBit[] = {0x7F, 0xBF, 0xDF, 0xEF,
                                                0xF7, 0xFB, 0xFD, 0xFE};
#endif

template <int16_t W, int16_t H>
class Canvas1 : public Adafruit_GFX {
    public:
        Canvas1(void);
        void drawPixel(int16_t x, int16_t y, uint16_t color);
        bool getPixel(int16_t x, int16_t y) const;
        void fillScreen(uint16_t color);

        const uint8_t *getBuffer(void) const { return buffer; }

    protected:
        bool getRawPixel(int16_t x, int16_t y) const;

    private:
        static const uint32_t size = ((W + 7) / 8) * H;
        uint8_t buffer[size];
};

template <int16_t W, int16_t H>
Canvas1<W, H>::Canvas1(void) :
    Adafruit_GFX(W, H) {
}

template <int16_t W, int16_t H>
bool Canvas1<W, H>::getPixel(int16_t x, int16_t y) const {
  int16_t t;
  switch (rotation) {
  case 1:
    t = x;
    x = W - 1 - y;
    y = t;
    break;
  case 2:
    x = W - 1 - x;
    y = H - 1 - y;
    break;
  case 3:
    t = x;
    x = y;
    y = H - 1 - t;
    break;
  }
  return getRawPixel(x, y);
}

template <int16_t W, int16_t H>
bool Canvas1<W, H>::getRawPixel(int16_t x, int16_t y) const {
  if ((x < 0) || (y < 0) || (x >= W) || (y >= H))
    return 0;
  if (buffer) {
    uint8_t *ptr = &buffer[(x / 8) + y * ((W + 7) / 8)];

#ifdef __AVR__
    return ((*ptr) & pgm_read_byte(&Canvas1setBit[x & 7])) != 0;
#else
    return ((*ptr) & (0x80 >> (x & 7))) != 0;
#endif
  }
  return 0;
}

template <int16_t W, int16_t H>
void Canvas1<W, H>::fillScreen(uint16_t color) {
  memset(buffer, color ? 0xFF : 0x00, size);
}

template <int16_t W, int16_t H>
void Canvas1<W, H>::drawPixel(int16_t x, int16_t y, uint16_t color) {
    if ((x < 0) || (y < 0) || (x >= _width) || (y >= _height))
      return;

    int16_t t;
    switch (rotation) {
    case 1:
      t = x;
      x = W - 1 - y;
      y = t;
      break;
    case 2:
      x = W - 1 - x;
      y = H - 1 - y;
      break;
    case 3:
      t = x;
      x = y;
      y = H - 1 - t;
      break;
    }

    uint8_t *ptr = &buffer[(x / 8) + y * ((W + 7) / 8)];
#ifdef __AVR__
    if (color)
      *ptr |= pgm_read_byte(&Canvas1setBit[x & 7]);
    else
      *ptr &= pgm_read_byte(&Canvas1clrBit[x & 7]);
#else
    if (color)
      *ptr |= 0x80 >> (x & 7);
    else
      *ptr &= ~(0x80 >> (x & 7));
#endif
}
