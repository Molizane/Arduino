// Dice

#include <Adafruit_GFX_ILI9340.h>    // Core graphics library
#include <ILI9340_TFTLCD.h> // Hardware-specific library
#include <TouchScreen.h>

#ifndef USE_ADAFRUIT_SHIELD_PINOUT
 #error "This sketch is intended for use with the TFT LCD Shield. Make sure that USE_ADAFRUIT_SHIELD_PINOUT is #defined in the ILI9340_TFTLCD.h library file."
#endif

// These are the pins for the shield!
#define YP A1  // must be an analog pin, use "An" notation!
#define XM A2  // must be an analog pin, use "An" notation!
#define YM 7   // can be a digital pin
#define XP 6   // can be a digital pin

#define ShiftX 49
#define ShiftY 49

// For better pressure precision, we need to know the resistance
// between X+ and X- Use any multimeter to read it
// For the one we're using, its 300 ohms across the X plate
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

typedef struct _DiceDots
{
  int x,y;
} DiceDots;

/*
DiceDots one[]   = {{120, 120}};
DiceDots two[]   = {{70, 70}, {170, 170}};
DiceDots three[] = {{70, 70}, {120, 120}, {170, 170}};
DiceDots four[]  = {{70, 70}, {70, 170}, {170, 70}, {170, 170}};
DiceDots five[]  = {{70, 70}, {70, 170}, {120, 120}, {170, 70}, {170, 170}};
DiceDots six[]   = {{70, 70}, {70, 120}, {70, 170}, {170, 70}, {170, 120}, {170, 170}};
DiceDots all[]   = {{70, 70}, {70, 120}, {70, 170}, {120, 120}, {170, 70}, {170, 120}, {170, 170}};
*/

DiceDots one[]   = {{71+ShiftX, 71+ShiftY}};
DiceDots two[]   = {{23+ShiftX, 23+ShiftY}, {119+ShiftX, 119+ShiftY}};
DiceDots three[] = {{23+ShiftX, 23+ShiftY}, { 71+ShiftX,  71+ShiftY}, {119+ShiftX, 119+ShiftY}};
DiceDots four[]  = {{23+ShiftX, 23+ShiftY}, { 23+ShiftX, 119+ShiftY}, {119+ShiftX,  23+ShiftY}, {119+ShiftX, 119+ShiftY}};
DiceDots five[]  = {{23+ShiftX, 23+ShiftY}, { 23+ShiftX, 119+ShiftY}, { 71+ShiftX,  71+ShiftY}, {119+ShiftX,  23+ShiftY}, {119+ShiftX, 119+ShiftY}};
DiceDots six[]   = {{23+ShiftX, 23+ShiftY}, { 23+ShiftX,  71+ShiftY}, { 23+ShiftX, 119+ShiftY}, {119+ShiftX,  23+ShiftY}, {119+ShiftX,  71+ShiftY}, {119+ShiftX, 119+ShiftY}};
DiceDots all[]   = {{23+ShiftX, 23+ShiftY}, { 23+ShiftX,  71+ShiftY}, { 23+ShiftX, 119+ShiftY}, { 71+ShiftX,  71+ShiftY}, {119+ShiftX,  23+ShiftY}, {119+ShiftX,  71+ShiftY}, {119+ShiftX, 119+ShiftY}};

// Assign human-readable names to some common 16-bit color values:
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

ILI9340_TFTLCD tft;

short numLoops = 1;
byte oldRandNumber = 0, randNumber = 0;

void setup(void) {
  Serial.begin(9600);
  Serial.println(F("Dice!"));

  tft.reset();

  uint16_t identifier = tft.readID();

  /*
  if(identifier == 0x9325) {
    Serial.println(F("Found ILI9325 LCD driver"));
  } else if(identifier == 0x9328) {
    Serial.println(F("Found ILI9328 LCD driver"));
  } else if(identifier == 0x9340) {
    Serial.println(F("Found ILI9340 LCD driver"));
  } else if(identifier == 0x7575) {
    Serial.println(F("Found HX8347G LCD driver"));
  } else {
    Serial.print(F("Unknown LCD driver chip: "));
    Serial.println(identifier, HEX);
    numLoops = -1;
    return;
  }
  */

  tft.begin(identifier);
  tft.fillScreen(YELLOW);
  tft.fillRoundRect(0+ShiftX, 0+ShiftY, 143, 143, 5, WHITE);
  tft.drawRoundRect(0+ShiftX, 0+ShiftY, 143, 143, 5, BLACK);

  pinMode(13, OUTPUT);
  randomSeed(analogRead(0));
}

#define MINPRESSURE 10
#define MAXPRESSURE 1000

void clearDots(void)
{
  for(int j=0;j<7;j++)
    tft.fillCircle(all[j].x, all[j].y, 18, WHITE);
}

void loop()
{
  if (numLoops == -1)
    return;

  digitalWrite(13, HIGH);
  TSPoint p = ts.getPoint();
  digitalWrite(13, LOW);

  // if sharing pins, you'll need to fix the directions of the touchscreen pins
  //pinMode(XP, OUTPUT);
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
  //pinMode(YM, OUTPUT);

  if (!numLoops && (p.z >= MINPRESSURE && p.z <= MAXPRESSURE))
  {
    randomSeed(p.z);
    numLoops = random(5,8);
  }

  if (numLoops) {
     randNumber = random(1,7);

     if (oldRandNumber != randNumber)
     {
       numLoops--;
       oldRandNumber = randNumber;

       if (!numLoops)
         Serial.println(randNumber);

       clearDots();

       switch(randNumber)
       {
          case 1:
            for(int i=0;i<randNumber;i++)
              tft.fillCircle(one[i].x, one[i].y, 18, BLACK);
          break;
          case 2:
            for(int i=0;i<randNumber;i++)
              tft.fillCircle(two[i].x, two[i].y, 18, BLACK);
          break;
          case 3:
            for(int i=0;i<randNumber;i++)
              tft.fillCircle(three[i].x, three[i].y, 18, BLACK);
          break;
          case 4:
            for(int i=0;i<randNumber;i++)
              tft.fillCircle(four[i].x, four[i].y, 18, BLACK);
          break;
          case 5:
            for(int i=0;i<randNumber;i++)
              tft.fillCircle(five[i].x, five[i].y, 18, BLACK);
          break;
          case 6:
            for(int i=0;i<randNumber;i++)
              tft.fillCircle(six[i].x, six[i].y, 18, BLACK);
          break;
       }
     }
  }
}
