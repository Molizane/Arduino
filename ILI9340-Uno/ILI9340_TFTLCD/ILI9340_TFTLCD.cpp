// IMPORTANT: LIBRARY MUST BE SPECIFICALLY CONFIGURED FOR EITHER TFT SHIELD
// OR BREAKOUT BOARD USAGE.  SEE RELEVANT COMMENTS IN ILI9340_TFTLCD.h

// Graphics library by ladyada/adafruit with init code from Rossum
// MIT license

#if defined(__SAM3X8E__)
  #include <include/pio.h>
  #define PROGMEM
  #define pgm_read_byte(addr) (*(const uint8_t *)(addr))
  #define pgm_read_word(addr) (*(const unsigned short *)(addr))
#endif

#ifdef __AVR__
  #include <avr/pgmspace.h>
#endif

#include "pins_arduino.h"
#include "wiring_private.h"
#include "ILI9340_TFTLCD.h"

// LCD controller chip identifiers
#define ID_932X    0
#define ID_7575    1
#define ID_9341    2
#define ID_HX8357D 3
#define ID_4535    4
#define ID_9340    5
#define ID_UNKNOWN 0xFF

#include "registers.h"

// Constructor for breakout board (configurable LCD control lines).
// Can still use this w/shield, but parameters are ignored.
ILI9340_TFTLCD::ILI9340_TFTLCD(uint8_t cs, uint8_t cd, uint8_t wr,
                               uint8_t rd, uint8_t reset) : Adafruit_GFX_ILI9340(TFTWIDTH, TFTHEIGHT)
{

#ifndef USE_ADAFRUIT_SHIELD_PINOUT
  // Convert pin numbers to registers and bitmasks
  _reset = reset;

#ifdef __AVR__
  csPort = portOutputRegister(digitalPinToPort(cs));
  cdPort = portOutputRegister(digitalPinToPort(cd));
  wrPort = portOutputRegister(digitalPinToPort(wr));
  rdPort = portOutputRegister(digitalPinToPort(rd));
#endif

#if defined(__SAM3X8E__)
  csPort = digitalPinToPort(cs);
  cdPort = digitalPinToPort(cd);
  wrPort = digitalPinToPort(wr);
  rdPort = digitalPinToPort(rd);
#endif

  csPinSet   = digitalPinToBitMask(cs);
  cdPinSet   = digitalPinToBitMask(cd);
  wrPinSet   = digitalPinToBitMask(wr);
  rdPinSet   = digitalPinToBitMask(rd);
  csPinUnset = ~csPinSet;
  cdPinUnset = ~cdPinSet;
  wrPinUnset = ~wrPinSet;
  rdPinUnset = ~rdPinSet;

#ifdef __AVR__
  *csPort |= csPinSet; // Set all control bits to HIGH (idle)
  *cdPort |= cdPinSet; // Signals are ACTIVE LOW
  *wrPort |= wrPinSet;
  *rdPort |= rdPinSet;
#endif

#if defined(__SAM3X8E__)
  csPort->PIO_SODR |= csPinSet; // Set all control bits to HIGH (idle)
  cdPort->PIO_SODR |= cdPinSet; // Signals are ACTIVE LOW
  wrPort->PIO_SODR |= wrPinSet;
  rdPort->PIO_SODR |= rdPinSet;
#endif

  pinMode(cs, OUTPUT);    // Enable outputs
  pinMode(cd, OUTPUT);
  pinMode(wr, OUTPUT);
  pinMode(rd, OUTPUT);

  if(reset) {
    digitalWrite(reset, HIGH);
    pinMode(reset, OUTPUT);
  }
#endif

  init();
}

// Constructor for shield (fixed LCD control lines)
ILI9340_TFTLCD::ILI9340_TFTLCD(void) : Adafruit_GFX_ILI9340(TFTWIDTH, TFTHEIGHT) {
  init();
}

// Initialization common to both shield & breakout configs
void ILI9340_TFTLCD::init(void) {

#ifdef USE_ADAFRUIT_SHIELD_PINOUT
  for(int p=0;p<10;p++)
    pinMode(p, OUTPUT);

  pinMode(A0, OUTPUT);
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);
  pinMode(A4, OUTPUT);

  digitalWrite(A0, HIGH);
  digitalWrite(A1, HIGH);
  digitalWrite(A2, HIGH);
  digitalWrite(A3, HIGH);
  digitalWrite(A4, HIGH);
#endif

  setWriteDirInline(); // Set up LCD data port(s) for WRITE operations

  rotation = 0;
  cursor_y = cursor_x = 0;
  textsize = 1;
  textcolor = 0xFFFF;
  _width = TFTWIDTH;
  _height = TFTHEIGHT;
}

void ILI9340_TFTLCD::write8(uint8_t d)
{
  WR_ACTIVE();
  PORTD = (PORTD & B00000011) | ((d) & B11111100);
  PORTB = (PORTB & B11111100) | ((d) & B00000011);
  //*(portOutputRegister(digitalPinToPort(LCD_WR))) &= ~digitalPinToBitMask(LCD_WR); // LCD_WR=0;
  //*(portOutputRegister(digitalPinToPort(LCD_WR))) |= digitalPinToBitMask(LCD_WR);  // LCD_WR=1;
  WR_IDLE();
}

uint8_t ILI9340_TFTLCD::read8(void)
{
  //*(portOutputRegister(digitalPinToPort(LCD_RD))) &= ~digitalPinToBitMask(LCD_RD); // LCD_RD=0;
  WR_IDLE();
  RD_ACTIVE();
  RD_IDLE();
  uint8_t r = (PORTD & B11111100) | (PORTB & B00000011);
  //*(portOutputRegister(digitalPinToPort(LCD_RD))) |= digitalPinToBitMask(LCD_RD);  // LCD_RD=1;
  return r;
}

void ILI9340_TFTLCD::writeRegister16(uint16_t a, uint16_t d)
{
  uint8_t hi, lo;

  hi = (a) >> 8;
  lo = (a);

  CD_COMMAND;
  write8(hi);
  write8(lo);

  hi = (d) >> 8;
  lo = (d);

  CD_DATA;
  write8(hi);
  write8(lo);
}

void ILI9340_TFTLCD::writeRegisterPair(uint8_t aH, uint8_t aL, uint16_t d)
{
  uint8_t hi = (d) >> 8, lo = (d);

  CD_COMMAND;
  write8(aH);

  CD_DATA;
  write8(hi);

  CD_COMMAND;
  write8(aL);

  CD_DATA;
  write8(lo);
}

void ILI9340_TFTLCD::writeRegister8(uint8_t com, uint8_t dat)
{
  CD_COMMAND;
  write8(com);

  CD_DATA;
  write8(dat);
}

// Initialization command tables for different LCD controllers
#define TFTLCD_DELAY 0xFF

void ILI9340_TFTLCD::begin(uint16_t id) {
  uint8_t i = 0;

  reset();

  delay(200);

  id = 0x9340;

  if (id == 0x9340) {
    driver = ID_9340;

    digitalWrite(LCD_REST, HIGH);
    delay(5);

    digitalWrite(LCD_REST, LOW);
    delay(15);

    digitalWrite(LCD_REST, HIGH);
    delay(15);

    CS_IDLE();

    WR_IDLE();

    CS_ACTIVE();  //CS

    CD_COMMAND;
    write8(0xCB);

    CD_DATA;
    write8(0x39);
    write8(0x2C);
    write8(0x00);
    write8(0x34);
    write8(0x02);

    CD_COMMAND;
    write8(0xCF);

    CD_DATA;
    write8(0x00);
    write8(0xC1);
    write8(0x30);

    CD_COMMAND;
    write8(0xE8);

    CD_DATA;
    write8(0x85);
    write8(0x00);
    write8(0x78);

    CD_COMMAND;
    write8(0xEA);

    CD_DATA;
    write8(0x00);
    write8(0x00);

    CD_COMMAND;
    write8(0xED);

    CD_DATA;
    write8(0x64);
    write8(0x03);
    write8(0x12);
    write8(0x81);

    CD_COMMAND;
    write8(0xF7);

    CD_DATA;
    write8(0x20);

    CD_COMMAND;
    write8(ILI9340_POWERCONTROL1);    //Power control

    CD_DATA;
    write8(0x23);   //VRH[5:0]

    CD_COMMAND;
    write8(ILI9340_POWERCONTROL2);    //Power control

    CD_DATA;
    write8(0x10);   //SAP[2:0];BT[3:0]

    CD_COMMAND;
    write8(ILI9340_VCOMCONTROL1);    //VCM control

    CD_DATA;
    write8(0x3e);   //Contrast
    write8(0x28);

    CD_COMMAND;
    write8(ILI9340_VCOMCONTROL2);    //VCM control2

    CD_DATA;
    write8(0x86);   //--

    CD_COMMAND;
    write8(ILI9340_MEMCONTROL);    // Memory Access Control

    CD_DATA;
    write8(0x48);

    CD_COMMAND;
    write8(ILI9340_PIXELFORMAT);

    CD_DATA;
    write8(0x55);

    CD_COMMAND;
    write8(ILI9340_FRAMECONTROL);

    CD_DATA;
    write8(0x00);
    write8(0x18);

    CD_COMMAND;
    write8(ILI9340_DISPLAYFUNC);    // Display Function Control

    CD_DATA;
    write8(0x08);
    write8(0x82);
    write8(0x27);

    CD_COMMAND;
    write8(ILI9340_SLEEPOUT);    //Exit Sleep
    delay(120);

    CD_COMMAND;
    write8(ILI9340_DISPLAYON);    //Display on
    write8(ILI9340_MEMORYWRITE);
  } else {
    driver = ID_UNKNOWN;
  }
}

void ILI9340_TFTLCD::reset(void) {
  digitalWrite(LCD_REST, HIGH);
  delay(5);

  digitalWrite(LCD_REST, LOW);
  delay(15);

  digitalWrite(LCD_REST, HIGH);
  delay(15);

  CS_IDLE();

  WR_IDLE();

  CS_ACTIVE();  //CS

  CD_COMMAND;
  write8(0xCB);

  CD_DATA;
  write8(0x39);
  write8(0x2C);
  write8(0x00);
  write8(0x34);
  write8(0x02);

  CD_COMMAND;
  write8(0xCF);

  CD_DATA;
  write8(0x00);
  write8(0XC1);
  write8(0X30);

  CD_COMMAND;
  write8(0xE8);

  CD_DATA;
  write8(0x85);
  write8(0x00);
  write8(0x78);

  CD_COMMAND;
  write8(0xEA);

  CD_DATA;
  write8(0x00);
  write8(0x00);

  CD_COMMAND;
  write8(0xED);

  CD_DATA;
  write8(0x64);
  write8(0x03);
  write8(0X12);
  write8(0X81);

  CD_COMMAND;
  write8(0xF7);

  CD_DATA;
  write8(0x20);

  CD_COMMAND;
  write8(ILI9340_POWERCONTROL1);    //Power control

  CD_DATA;
  write8(0x23);   //VRH[5:0]

  CD_COMMAND;
  write8(ILI9340_POWERCONTROL2);    //Power control

  CD_DATA;
  write8(0x10);   //SAP[2:0];BT[3:0]

  CD_COMMAND;
  write8(ILI9340_VCOMCONTROL1);    //VCM control1

  CD_DATA;
  write8(0x3e);   //Contrast
  write8(0x28);

  CD_COMMAND;
  write8(ILI9340_VCOMCONTROL2);    //VCM control2

  CD_DATA;
  write8(0x86);   //--

  CD_COMMAND;
  write8(ILI9340_MEMCONTROL);    // Memory Access Control

  CD_DATA;
  write8(0x48);

  CD_COMMAND;
  write8(ILI9340_PIXELFORMAT);

  CD_DATA;
  write8(0x55);

  CD_COMMAND;
  write8(ILI9340_FRAMECONTROL);

  CD_DATA;
  write8(0x00);
  write8(0x18);

  CD_COMMAND;
  write8(ILI9340_DISPLAYFUNC);    // Display Function Control

  CD_DATA;
  write8(0x08);
  write8(0x82);
  write8(0x27);

  CD_COMMAND;
  write8(ILI9340_SLEEPOUT);    //Exit Sleep

  delay(120);

  CD_COMMAND;
  write8(ILI9340_DISPLAYON);    //Display on
  //write8(ILI9340_MEMORYWRITE);
}

// Sets the LCD address window (and address counter, on 932X).
// Relevant to rect/screen fills and H/V lines.  Input coordinates are
// assumed pre-sorted (e.g. x2 >= x1).
void ILI9340_TFTLCD::setAddrWindow(int x1, int y1, int x2, int y2) {
  CS_ACTIVE();

  CD_COMMAND;
  write8(ILI9340_COLADDRSET);

  CD_DATA;
  write8(x1>>8);
  write8(x1);
  write8(x2>>8);
  write8(x2);

  CD_COMMAND;
  write8(ILI9340_PAGEADDRSET);

  CD_DATA;
  write8(y1>>8);
  write8(y1);
  write8(y2>>8);
  write8(y2);

  //CD_COMMAND;
  //write8(ILI9340_MEMORYWRITE);

  CS_IDLE();
}

// Unlike the 932X drivers that set the address window to the full screen
// by default (using the address counter for drawPixel operations), the
// 7575 needs the address window set on all graphics operations.  In order
// to save a few register writes on each pixel drawn, the lower-right
// corner of the address window is reset after most fill operations, so
// that drawPixel only needs to change the upper left each time.
void ILI9340_TFTLCD::setLR(void) {
  /*
  CS_ACTIVE();

  writeRegisterPair(HX8347G_COLADDREND_HI, HX8347G_COLADDREND_LO, TFTWIDTH  - 1);
  writeRegisterPair(HX8347G_ROWADDREND_HI, HX8347G_ROWADDREND_LO, TFTHEIGHT - 1);

  CS_IDLE();
  */
}

// Fast block fill operation for fillScreen, fillRect, H/V line, etc.
// Requires setAddrWindow() has previously been called to set the fill
// bounds.  'len' is inclusive, MUST be >= 1.
void ILI9340_TFTLCD::flood(uint16_t color, uint32_t len) {
  uint16_t blocks;
  uint8_t  i, hi = color >> 8, lo = color;

  CS_ACTIVE();

  CD_COMMAND;
  write8(ILI9340_MEMORYWRITE);

  // Write first pixel normally, decrement counter by 1
  CD_DATA;
  write8(hi);
  write8(lo);

  len--;

  blocks = (uint16_t)(len / 64); // 64 pixels/block

  if(hi == lo) {
    // High and low bytes are identical.  Leave prior data
    // on the port(s) and just toggle the write strobe.
    while(blocks--) {
      i = 16; // 64 pixels/block / 4 pixels/pass

      do {
        WR_STROBE(); WR_STROBE(); WR_STROBE(); WR_STROBE(); // 2 bytes/pixel
        WR_STROBE(); WR_STROBE(); WR_STROBE(); WR_STROBE(); // x 4 pixels
      } while(--i);
    }

    // Fill any remaining pixels (1 to 64)
    for(i = (uint8_t)len & 63; i--; ) {
      WR_STROBE();
      WR_STROBE();
    }
  } else {
    while(blocks--) {
      i = 16; // 64 pixels/block / 4 pixels/pass

      do {
        write8(hi); write8(lo); write8(hi); write8(lo);
        write8(hi); write8(lo); write8(hi); write8(lo);
      } while(--i);
    }

    for(i = (uint8_t)len & 63; i--;) {
      write8(hi);
      write8(lo);
    }
  }

  CS_IDLE();
}

void ILI9340_TFTLCD::drawFastHLine(int16_t x, int16_t y, int16_t length, uint16_t color)
{
  int16_t x2;

  // Initial off-screen clipping
  if((length <= 0) || (y < 0) || (y >= height()) || (x >= width()) || ((x2 = (x+length-1)) < 0))
    return;

  if(x < 0) {        // Clip left
    length += x;
    x = 0;
  }

  if(x2 >= width()) { // Clip right
    x2 = width() - 1;
    length = x2 - x + 1;
  }

  setAddrWindow(x, y, x2, y);
  flood(color, length);


  //if(driver == ID_932X)
    setAddrWindow(0, 0, width() - 1, height() - 1);
  //else
  //  setLR();
}

void ILI9340_TFTLCD::drawFastVLine(int16_t x, int16_t y, int16_t length,  uint16_t color)
{
  int16_t y2;

  // Initial off-screen clipping
  if((length <= 0) ||
    (x < 0) || (x >= width()) ||
    (y >= height()) || ((y2 = (y+length-1)) < 0))
    return;

  if(y < 0) {         // Clip top
    length += y;
    y = 0;
  }

  if(y2 >= height()) { // Clip bottom
    y2 = height() - 1;
    length = y2 - y + 1;
  }

  setAddrWindow(x, y, x, y2);
  flood(color, length);

  //if(driver == ID_932X)
    setAddrWindow(0, 0, width() - 1, height() - 1);
  //else
  //  setLR();
}

void ILI9340_TFTLCD::fillRect(int16_t x1, int16_t y1, int16_t w, int16_t h, uint16_t fillcolor) {
  int16_t  x2, y2;

  // Initial off-screen clipping
  if((w <= 0) || (h <= 0) || (x1 >= width()) || (y1 >= height()) || ((x2 = x1+w-1) < 0) || ((y2 = y1+h-1) < 0))
    return;

  if(x1 < 0) { // Clip left
    w += x1;
    x1 = 0;
  }

  if(y1 < 0) { // Clip top
    h += y1;
    y1 = 0;
  }

  if(x2 >= width()) { // Clip right
    x2 = width() - 1;
    w = x2 - x1 + 1;
  }

  if(y2 >= height()) { // Clip bottom
    y2 = height() - 1;
    h = y2 - y1 + 1;
  }

  setAddrWindow(x1, y1, x2, y2);
  flood(fillcolor, (uint32_t)w * (uint32_t)h);

  //if(driver == ID_932X)
  //  setAddrWindow(0, 0, width() - 1, height() - 1);
  //else
  //  setLR();
}

void ILI9340_TFTLCD::fillScreen(uint16_t color) {

  uint16_t x, y;

  /*
  switch(rotation) {
    default:
      x = TFTWIDTH;
      y = TFTHEIGHT;
      break;
    case 2 :
      x = TFTWIDTH;
      y = TFTHEIGHT;
      break;
    case 1 :
      x = TFTHEIGHT;
      y = TFTWIDTH;
      break;
    case 3 :
      x = TFTHEIGHT;
      y = TFTWIDTH;
      break;
  }
  */

  x = width();
  y = height();

  //CS_ACTIVE();

  //CD_COMMAND;
  //write8(ILI9340_MEMORYWRITE); //write_memory_start

  setAddrWindow(0, 0, x, y);
  flood(color, (long)x * (long)y);
}

void ILI9340_TFTLCD::drawPixel(int16_t x, int16_t y, uint16_t color) {
  // Clip
  if((x < 0) || (y < 0) || (x >= width()) || (y >= height()))
    return;

  //CS_ACTIVE();

  setAddrWindow(x, y, width()-1, height()-1);

  CS_ACTIVE();

  CD_COMMAND;
  write8(ILI9340_MEMORYWRITE);

  CD_DATA;
  write8(color >> 8); write8(color);

  CS_IDLE();
}

// Issues 'raw' an array of 16-bit color values to the LCD; used
// externally by BMP examples.  Assumes that setWindowAddr() has
// previously been set to define the bounds.  Max 255 pixels at
// a time (BMP examples read in small chunks due to limited RAM).
void ILI9340_TFTLCD::pushColors(uint16_t *data, uint8_t len, boolean first) {
  uint16_t color;
  uint8_t  hi, lo;

  CS_ACTIVE();

  if(first == true) { // Issue GRAM write command only on first call
    CD_COMMAND;
    write8(ILI9340_MEMORYWRITE);
  }

  CD_DATA;

  while(len--) {
    color = *data++;
    hi = color >> 8; // Don't simplify or merge these
    lo = color;      // lines, there's macro shenanigans
    write8(hi);         // going on.
    write8(lo);
  }

  CS_IDLE();
}

void ILI9340_TFTLCD::setRotation(uint8_t x) {

  // Call parent rotation func first -- sets up rotation flags, etc.
  Adafruit_GFX_ILI9340::setRotation(x);
  // Then perform hardware-specific rotation operations...

  // MEME, HX8357D uses same registers as 9340 but different values
  uint16_t t;

  switch (rotation) {
    case 2:
      t = ILI9340_MADCTL_MX | ILI9340_MADCTL_BGR;
      break;
    case 3:
      t = ILI9340_MADCTL_MV | ILI9340_MADCTL_BGR;
      break;
    case 0:
      t = ILI9340_MADCTL_MY | ILI9340_MADCTL_BGR;
      break;
    case 1:
      t = ILI9340_MADCTL_MX | ILI9340_MADCTL_MY | ILI9340_MADCTL_MV | ILI9340_MADCTL_BGR;
      break;
    default:
      t = 0;
      break;
  }

  CS_ACTIVE();
  writeRegister8(ILI9340_MADCTL, t); // MADCTL
  // For 9340, init default full-screen address window:
  setAddrWindow(0, 0, width() - 1, height() - 1); // CS_IDLE happens here
}

// Because this function is used infrequently, it configures the ports for
// the read operation, reads the data, then restores the ports to the write
// configuration.  Write operations happen a LOT, so it's advantageous to
// leave the ports in that state as a default.
uint16_t ILI9340_TFTLCD::readPixel(int16_t x, int16_t y) {

  //if((x < 0) || (y < 0) || (x >= width()) || (y >= height()))
  return 0;
}

// Ditto with the read/write port directions, as above.
uint16_t ILI9340_TFTLCD::readID(void) {

  //uint8_t hi, lo;

  /*
  for (uint8_t i=0; i<128; i++) {
  Serial.print("$")); Serial.print(i, HEX);
  Serial.print(" = 0x")); Serial.println(readReg(i), HEX);
  }
  */

  //Serial.print("Reg 0x04=0x");
  //Serial.println(
  readReg(0x04);
  //, HEX);

  //if (readReg(0x04) == 0x8000) { // eh close enough
  //  // setc!
  //  /*
  //  Serial.println(F("!"));
  //  for (uint8_t i=0; i<254; i++) {
  //  Serial.print("$")); Serial.print(i, HEX);
  //  Serial.print(" = 0x")); Serial.println(readReg(i), HEX);
  //  }
  //  */
  //  writeRegister24(HX8357D_SETC, 0xFF8357);
  //  delay(300);
  //
  //  //Serial.println(readReg(0xD0), HEX);
  //
  //  if (readReg(0xD0) == 0x990000) {
  //    return 0x8357;
  //  }
  //}

  //Serial.print("Reg 0xD3=0x");
  //Serial.println(readReg(0xD3), HEX);

  //uint16_t id = readReg(0xD3);

  //if (id == 0x9340 || id == 0x9341) {
  //  return id;
  //}

  //CS_ACTIVE();

  //CD_COMMAND;
  //write8(ILI9340_NOP);

  //WR_STROBE();          // Repeat prior byte (0x00)
  //setReadDirInline();   // Set up LCD data port(s) for READ operations

  //write8(ILI9340_MEMORYREAD);

  //CD_DATA;
  //hi = read8();
  //lo = read8();

  //setWriteDirInline();  // Restore LCD data port(s) to WRITE configuration

  //CD_COMMAND;
  //write8(ILI9340_MEMORYWRITE);

  //CD_DATA;

  //CS_IDLE();

  //id = hi;
  //id <<= 8;
  //id |= lo;
  //return id;



  return 0x9340;
}

uint32_t ILI9340_TFTLCD::readReg(uint8_t r) {
  uint32_t id;
  uint8_t x;

  Serial.print("Read $"); Serial.println(r, HEX);

  // try reading register #4
  CS_ACTIVE();

  CD_COMMAND;
  write8(r);

  setReadDirInline();  // Set up LCD data port(s) for READ operations

  CD_DATA;
  delayMicroseconds(50);
  x = read8();

  Serial.print(" x = 0x"); Serial.println(x, HEX);

  id = x;          // Do not merge or otherwise simplify
  id <<= 8;        // these lines.  It's an unfortunate
  x = read8();

  Serial.print(" x = 0x"); Serial.println(x, HEX);

  id  |= x;        // shenanigans that are going on.
  id <<= 8;        // these lines.  It's an unfortunate
  x = read8();

  Serial.print(" x = 0x"); Serial.println(x, HEX);

  id  |= x;        // shenanigans that are going on.
  id <<= 8;        // these lines.  It's an unfortunate
  x = read8();

  Serial.print(" x = 0x"); Serial.println(x, HEX);

  id  |= x;        // shenanigans that are going on.

  CS_IDLE();

  setWriteDirInline();  // Restore LCD data port(s) to WRITE configuration

  Serial.print(" id = 0x"); Serial.println(id, HEX);

  return id;
}

// Pass 8-bit (each) R,G,B, get back 16-bit packed color
uint16_t ILI9340_TFTLCD::color565(uint8_t r, uint8_t g, uint8_t b) {
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

// For I/O macros that were left undefined, declare function
// versions that reference the inline macros just once:

void ILI9340_TFTLCD::writeRegister24(uint8_t r, uint32_t d) {
  CS_ACTIVE();

  CD_COMMAND;
  write8(r);

  CD_DATA;
  delayMicroseconds(10);
  write8(d >> 16);
  delayMicroseconds(10);
  write8(d >> 8);
  delayMicroseconds(10);
  write8(d);

  CS_IDLE();
}


void ILI9340_TFTLCD::writeRegister32(uint8_t r, uint32_t d) {
  CS_ACTIVE();

  CD_COMMAND;
  write8(r);

  CD_DATA;
  delayMicroseconds(10);
  write8(d >> 24);
  delayMicroseconds(10);
  write8(d >> 16);
  delayMicroseconds(10);
  write8(d >> 8);
  delayMicroseconds(10);
  write8(d);

  CS_IDLE();
}

void ILI9340_TFTLCD::CS_ACTIVE(void)
{
  digitalWrite(LCD_CS, LOW);
}

void ILI9340_TFTLCD::CS_IDLE(void)
{
  digitalWrite(LCD_CS, HIGH);
}

void ILI9340_TFTLCD::WR_ACTIVE(void)
{
  digitalWrite(LCD_WR, LOW);
}

void ILI9340_TFTLCD::WR_IDLE(void)
{
  digitalWrite(LCD_WR, HIGH);
}

void ILI9340_TFTLCD::RD_ACTIVE(void)
{
  digitalWrite(LCD_RD, LOW);
}

void ILI9340_TFTLCD::RD_IDLE(void)
{
  digitalWrite(LCD_RD, HIGH);
}

void ILI9340_TFTLCD::WR_STROBE(void)
{
  WR_ACTIVE();
  WR_IDLE();
}
