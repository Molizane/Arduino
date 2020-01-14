// IMPORTANT: SEE COMMENTS @ LINE 15 REGARDING SHIELD VS BREAKOUT BOARD USAGE.

// Graphics library by ladyada/adafruit with init code from Rossum
// MIT license

#ifndef _ILI9340_TFTLCD_H_
#define _ILI9340_TFTLCD_H_

#if ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#include <Adafruit_GFX_ILI9340.h>

// **** IF USING THE LCD BREAKOUT BOARD, COMMENT OUT THIS NEXT LINE. ****
// **** IF USING THE LCD SHIELD, LEAVE THE LINE ENABLED:             ****

#define USE_ADAFRUIT_SHIELD_PINOUT 1

#define TFTWIDTH   240
#define TFTHEIGHT  320

#define LCD_RD   A0
#define LCD_WR   A1
#define LCD_RS   A2
#define LCD_CS   A3
#define LCD_REST A4

// Color definitions
#define ILI9340_BLACK       0x0000      /*   0,   0,   0 */
#define ILI9340_NAVY        0x000F      /*   0,   0, 128 */
#define ILI9340_DARKGREEN   0x03E0      /*   0, 128,   0 */
#define ILI9340_DARKCYAN    0x03EF      /*   0, 128, 128 */
#define ILI9340_MAROON      0x7800      /* 128,   0,   0 */
#define ILI9340_PURPLE      0x780F      /* 128,   0, 128 */
#define ILI9340_OLIVE       0x7BE0      /* 128, 128,   0 */
#define ILI9340_LIGHTGREY   0xC618      /* 192, 192, 192 */
#define ILI9340_DARKGREY    0x7BEF      /* 128, 128, 128 */
#define ILI9340_BLUE        0x001F      /*   0,   0, 255 */
#define ILI9340_GREEN       0x07E0      /*   0, 255,   0 */
#define ILI9340_CYAN        0x07FF      /*   0, 255, 255 */
#define ILI9340_RED         0xF800      /* 255,   0,   0 */
#define ILI9340_MAGENTA     0xF81F      /* 255,   0, 255 */
#define ILI9340_YELLOW      0xFFE0      /* 255, 255,   0 */
#define ILI9340_WHITE       0xFFFF      /* 255, 255, 255 */
#define ILI9340_ORANGE      0xFD20      /* 255, 165,   0 */
#define ILI9340_GREENYELLOW 0xAFE5      /* 173, 255,  47 */
#define ILI9340_PINK        0xF81F
#define ILI9340_GREY        0x5AEB

#define CD_COMMAND *(portOutputRegister(digitalPinToPort(LCD_RS))) &= ~digitalPinToBitMask(LCD_RS); // LCD_RS=0;
#define CD_DATA    *(portOutputRegister(digitalPinToPort(LCD_RS))) |= digitalPinToBitMask(LCD_RS);  // LCD_RS=1;
//                                      OOxOxxTR            xxOxOOOO
//#define setWriteDirInline() { DDRD |=  B11010000; DDRB |=  B00101111; }
#define setWriteDirInline() { DDRD |=  B11111100; DDRB |=  B00000011; }
//                                      IIxIxxxx            xxIxIIII
//                                     B001011TR           B11010000
//#define setReadDirInline()  { DDRD &= ~B11010000; DDRB &= ~B00101111; }
#define setReadDirInline()  { DDRD &= ~B11111100; DDRB &= ~B00000011; }

#define DELAY7        \
  asm volatile(       \
    "rjmp .+0" "\n\t" \
    "rjmp .+0" "\n\t" \
    "rjmp .+0" "\n\t" \
    "rjmp .+0" "\n\t" \
    "rjmp .+0" "\n\t" \
    "rjmp .+0" "\n\t" \
    "rjmp .+0" "\n\t" \
    "rjmp .+0" "\n\t" \
    "rjmp .+0" "\n\t" \
    "nop"      "\n"   \
    ::);

class ILI9340_TFTLCD : public Adafruit_GFX_ILI9340 {

 public:
  ILI9340_TFTLCD(uint8_t cs, uint8_t cd, uint8_t wr, uint8_t rd, uint8_t rst);
  ILI9340_TFTLCD(void);

  void begin(uint16_t id = 0x9325);
  void drawPixel(int16_t x, int16_t y, uint16_t color);
  void drawFastHLine(int16_t x0, int16_t y0, int16_t w, uint16_t color);
  void drawFastVLine(int16_t x0, int16_t y0, int16_t h, uint16_t color);
  void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t c);
  void fillScreen(uint16_t color);
  void reset(void);
  void setRegisters8(uint8_t *ptr, uint8_t n);
  void setRegisters16(uint16_t *ptr, uint8_t n);
  void setRotation(uint8_t x);

  // These methods are public in order for BMP examples to work:
  void setAddrWindow(int x1, int y1, int x2, int y2);
  void pushColors(uint16_t *data, uint8_t len, boolean first);

  uint16_t color565(uint8_t r, uint8_t g, uint8_t b),
           readPixel(int16_t x, int16_t y),
           readID(void);
  uint32_t readReg(uint8_t r);

  void CS_ACTIVE(void);
  void CS_IDLE(void);
  void WR_ACTIVE(void);
  void WR_IDLE(void);
  void RD_ACTIVE(void);
  void RD_IDLE(void);
  void WR_STROBE(void);
  void write8(uint8_t d);
  uint8_t read8(void);
private:
  void init(),
       // These items may have previously been defined as macros
       // in pin_magic.h.  If not, function versions are declared:
       setWriteDir(void),
       setReadDir(void),
       writeRegister8(uint8_t com, uint8_t dat),
       writeRegister16(uint16_t a, uint16_t d),
       writeRegister24(uint8_t a, uint32_t d),
       writeRegister32(uint8_t a, uint32_t d),
       writeRegisterPair(uint8_t aH, uint8_t aL, uint16_t d),
       setLR(void),
       flood(uint16_t color, uint32_t len);

  uint8_t driver;

#ifndef USE_ADAFRUIT_SHIELD_PINOUT
  #ifdef __AVR__
    volatile uint8_t *csPort, *cdPort, *wrPort, *rdPort;
    uint8_t csPinSet, cdPinSet, wrPinSet, rdPinSet,
            csPinUnset, cdPinUnset, wrPinUnset, rdPinUnset,
            _reset;
  #endif

  #if defined(__SAM3X8E__)
    Pio *csPort, *cdPort, *wrPort, *rdPort;
    uint32_t csPinSet, cdPinSet, wrPinSet, rdPinSet,
             csPinUnset, cdPinUnset, wrPinUnset, rdPinUnset,
             _reset;
  #endif
#endif
};

// For compatibility with sketches written for older versions of library.
// Color function name was changed to 'color565' for parity with 2.2" LCD
// library.
#define Color565 color565

#endif
