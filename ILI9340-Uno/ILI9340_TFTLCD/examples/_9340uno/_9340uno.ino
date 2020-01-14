//Technical support:goodtft@163.com
// Breakout/Arduino UNO pin usage:
// LCD Data Bit :   7   6   5   4   3   2   1   0
// Uno dig. pin :   7   6   5   4   3   2   9   8
// Uno port/pin : PD7 PD6 PD5 PD4 PD3 PD2 PB1 PB0
// Mega dig. pin:  29  28  27  26  25  24  23  22
#define LCD_RD   A0
#define LCD_WR   A1
#define LCD_RS   A2
#define LCD_CS   A3
#define LCD_REST A4

#define TFTWIDTH  240
#define TFTHEIGHT 320

#define CS_ACTIVE digitalWrite(LCD_CS, LOW)
#define CS_IDLE   digitalWrite(LCD_CS, HIGH)

#define WR_ACTIVE digitalWrite(LCD_RS, HIGH)
#define WR_IDLE   digitalWrite(LCD_RS, LOW)

#define CD_COMMAND *(portOutputRegister(digitalPinToPort(LCD_RS))) &= ~digitalPinToBitMask(LCD_RS); // LCD_RS=0;
#define CD_DATA    *(portOutputRegister(digitalPinToPort(LCD_RS))) |= digitalPinToBitMask(LCD_RS);  // LCD_RS=1;

void write8(unsigned char d)
{
  PORTD = (PORTD & B00000011) | ((d) & B11111100);
  PORTB = (PORTB & B11111100) | ((d) & B00000011);
  *(portOutputRegister(digitalPinToPort(LCD_WR))) &= ~digitalPinToBitMask(LCD_WR); // LCD_WR=0;
  *(portOutputRegister(digitalPinToPort(LCD_WR))) |= digitalPinToBitMask(LCD_WR);  // LCD_WR=1;
}

void Lcd_Write_Com(unsigned char VH)
{
  *(portOutputRegister(digitalPinToPort(LCD_RS))) &= ~digitalPinToBitMask(LCD_RS); //LCD_RS=0;
  write8(VH);
}

void Lcd_Write_Data(unsigned char VH)
{
  *(portOutputRegister(digitalPinToPort(LCD_RS))) |= digitalPinToBitMask(LCD_RS); //LCD_RS=1;
  write8(VH);
}

void Lcd_Write_Com_Data(unsigned char com, unsigned char dat)
{
  CD_COMMAND;
  write8(com);
  CD_DATA;
  write8(dat);
}

void setAddrWindow(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2)
{
  CD_COMMAND;
  write8(0x2a);
  CD_DATA;
  write8(x1>>8);
  write8(x1);
  write8(x2>>8);
  write8(x2);

  CD_COMMAND;
  write8(0x2b);
  CD_DATA;
  write8(y1>>8);
  write8(y1);
  write8(y2>>8);
  write8(y2);

  CD_COMMAND;
  write8(0x2c);
}

void Lcd_Init(void)
{
  digitalWrite(LCD_REST, HIGH);
  delay(5);

  digitalWrite(LCD_REST, LOW);
  delay(15);

  digitalWrite(LCD_REST, HIGH);
  delay(15);

  CS_IDLE;
  digitalWrite(LCD_WR, HIGH);
  CS_ACTIVE;  //CS

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
  write8(0xC0);    //Power control
  CD_DATA;
  write8(0x23);   //VRH[5:0]

  CD_COMMAND;
  write8(0xC1);    //Power control
  CD_DATA;
  write8(0x10);   //SAP[2:0];BT[3:0]

  CD_COMMAND;
  write8(0xC5);    //VCM control
  CD_DATA;
  write8(0x3e);   //Contrast
  write8(0x28);

  CD_COMMAND;
  write8(0xC7);    //VCM control2
  CD_DATA;
  write8(0x86);   //--

  CD_COMMAND;
  write8(0x36);    // Memory Access Control
  CD_DATA;
  write8(0x48);

  CD_COMMAND;
  write8(0x3A);
  CD_DATA;
  write8(0x55);

  CD_COMMAND;
  write8(0xB1);
  CD_DATA;
  write8(0x00);
  write8(0x18);

  CD_COMMAND;
  write8(0xB6);    // Display Function Control
  CD_DATA;
  write8(0x08);
  write8(0x82);
  write8(0x27);

  CD_COMMAND;
  write8(0x11);    //Exit Sleep
  delay(120);

  CD_COMMAND;
  write8(0x29);    //Display on
  write8(0x2c);
}

void H_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c)
{
  CD_COMMAND;
  write8(0x02c); //write_memory_start

  //WR_ACTIVE;
  //CS_ACTIVE;

  setAddrWindow(x, y, x+l, y);

  unsigned char c0 = (c & 0xFF00) >> 8, c1 = c & 0x00FF;

  CD_DATA;

  for(unsigned int i=1;i<=l;i++)
  {
    write8(c0);
    write8(c1);
  }

  //WR_IDLE;
  //CS_IDLE;
}

void V_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c)
{
  CD_COMMAND;
  write8(0x02c); //write_memory_start

  //WR_ACTIVE;
  //CS_ACTIVE;

  setAddrWindow(x, y, x, y+l);

  unsigned char c0 = (c & 0xFF00) >> 8, c1 = c & 0x00FF;

  CD_DATA;

  for(unsigned int i=1;i<=l;i++)
  {
    write8(c0);
    write8(c1);
  }

  //WR_IDLE;
  //CS_IDLE;
}

void dot(unsigned int x, unsigned int y, unsigned int c)
{
  CD_COMMAND;
  write8(0x02c); //write_memory_start

  //WR_ACTIVE;
  //CS_ACTIVE;

  setAddrWindow(x, y, x, y);

  CD_DATA;
  write8((c & 0xFF00) >> 8);
  write8(c & 0x00FF);

  //WR_IDLE;
  //CS_IDLE;
}

void Rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c)
{
  H_line(x  , y  , w, c);
  H_line(x  , y+h, w, c);
  V_line(x  , y  , h, c);
  V_line(x+w, y  , h, c);
}

void Rectf(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c)
{
  for(unsigned int i=0;i<h;i++)
  {
    H_line(x  , y  , w, c);
    H_line(x  , y+i, w, c);
  }
}

int color565(int r, int g, int b)
{
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

void LCD_Clear(unsigned int j)
{
  CD_COMMAND;
  write8(0x02c); //write_memory_start

  //WR_ACTIVE;
  //CS_ACTIVE;

  setAddrWindow(0, 0, TFTWIDTH, TFTHEIGHT);

  unsigned char j0 = (j & 0xFF00) >> 8, j1 = j & 0x00FF;

  CD_DATA;

  for(unsigned int i=0;i<TFTWIDTH;i++)
    for(unsigned int m=0;m<TFTHEIGHT;m++)
    {
      write8(j0);
      write8(j1);
    }

  //CS_IDLE;
  //WR_IDLE;
}

void LCD_Fill(void)
{
  CD_COMMAND;
  write8(0x02c); //write_memory_start

  //WR_ACTIVE;
  //CS_ACTIVE;

  setAddrWindow(0, 0, TFTWIDTH, TFTHEIGHT);

  CD_DATA;

  for(unsigned int i=0;i<TFTWIDTH;i++)
    for(unsigned int m=0;m<TFTHEIGHT;m++)
    {
      unsigned int j = random(65535);
      unsigned char j0 = (j & 0xFF00) >> 8, j1 = j & 0x00FF;

      write8(j0);
      write8(j1);
    }

  //CS_IDLE;
  //WR_IDLE;
}

unsigned int color2(unsigned int c1)
{
  unsigned int c2;

  do {
    c2 = random(65535);
  } while (c1 == c2);

  return c2;
}

void setup()
{
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

  Lcd_Init();

  Serial.begin(9600);
  Serial.println(F("TFT LCD test"));
}

void loop()
{
  /*
  LCD_Clear(0x0000);
  LCD_Clear(0xF800);
  LCD_Clear(0x07E0);
  LCD_Clear(0x001F);
  LCD_Clear(0xFFFF);
  */

  unsigned int c1 = random(65535), c2 = 0;
  unsigned int x, y, w, h;

  LCD_Clear(c1);

  for(int i=0;i<12;i++)
  {
    Serial.println(i);

    x = random(TFTWIDTH);
    w = random(TFTWIDTH);

    if (x + w > TFTWIDTH)
      w = TFTWIDTH - x;

    y = random(TFTHEIGHT);
    h = random(TFTHEIGHT);

    if (y + h > TFTHEIGHT)
      h = TFTHEIGHT - y;

    c2 = color2(c1);

    if (i % 2 == 0)
      Rect(x, y, w, h, c2); // rectangle at x, y, with, hight, color
    else
      Rectf(x, y, w, h, c2); // rectangle at x, y, with, hight, color

    if (i % 3 == 0)
      dot(random(TFTWIDTH), random(TFTHEIGHT), color2(c2));
  }

  LCD_Fill();

  delay(1000);

  Serial.println("");
}
