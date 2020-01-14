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
#define ShiftY 85

// For better pressure precision, we need to know the resistance
// between X+ and X- Use any multimeter to read it
// For the one we're using, its 300 ohms across the X plate
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

typedef struct _DiceDots
{
  int x,y;
} DiceDots;

/*
  ---------
 | A  B  C |
 | D  E  F |
 | G  H  I |
  ---------
*/

DiceDots A = {36+ShiftX,  36+ShiftY}, B = {72+ShiftX,  36+ShiftY}, C = {108+ShiftX,  36+ShiftY},
         D = {36+ShiftX,  72+ShiftY}, E = {72+ShiftX,  72+ShiftY}, F = {108+ShiftX,  72+ShiftY},
         G = {36+ShiftX, 108+ShiftY}, H = {72+ShiftX, 108+ShiftY}, I = {108+ShiftX, 108+ShiftY};

DiceDots zero[]  = {};
DiceDots one[]   = {E};
DiceDots two[]   = {A, I};
DiceDots three[] = {A, E, I};
DiceDots four[]  = {A, C, G, I};
DiceDots five[]  = {A, C, E, G, I};
DiceDots six[]   = {A, C, D, F, G, I};
DiceDots seven[] = {A, C, D, E, F, G, I};
DiceDots eight[] = {A, B, C, D, F, G, H, I};
DiceDots nine[]  = {A, B, C, D, E, F, G, H, I};

//char *Numeros[10] = {"  Zero", "  One", "   Two", " Three", "   Four", "  Five", "   Six", " Seven", " Eight", "  Nine"};
//char *(*ptr)[10] = &Numeros;

//String Numeros[] = {"  Zero", "  One", "   Two", " Three", "   Four", "  Five", "   Six", " Seven", " Eight", "  Nine"};

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
boolean mustClear = false;
//unsigned long start = 0;

void setup(void) {
  Serial.begin(9600);
  delay(500);
  Serial.println(F(" Dice!"));

  tft.reset();

  uint16_t identifier = tft.readID();

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

  tft.begin(identifier);
  tft.fillScreen(YELLOW);
  tft.fillRoundRect(0+ShiftX, 0+ShiftY, 143, 143, 8, WHITE);
  tft.drawRoundRect(0+ShiftX, 0+ShiftY, 143, 143, 8, BLACK);

  pinMode(13, OUTPUT);
  randomSeed(analogRead(0));
  //start = millis();
}

#define MINPRESSURE 10
#define MAXPRESSURE 1000

void clearAllDots()
{
  for(int j=0;j<9;j++)
    tft.fillCircle(nine[j].x, nine[j].y, 14, WHITE);
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

  //if (!numLoops && ((p.z >= MINPRESSURE && p.z <= MAXPRESSURE) || millis() - start > 10000))
  if (!numLoops && p.z >= MINPRESSURE && p.z <= MAXPRESSURE)
  {
    //randomSeed(p.z);
    numLoops = random(5,8);

    if (mustClear)
    {
      //tft.fillRect(tft.width() / 2 - 9, 153+ShiftY, 10, 50, YELLOW);
      tft.setCursor(tft.width() / 2 - 9, 153+ShiftY);
      tft.setTextColor(YELLOW);
      tft.setTextSize(5);
      tft.println(oldRandNumber);
      mustClear = false;
    }

    //// In order not to trigger while the dice is "rolling"
    //start = millis() + 500000;
  }

  if (numLoops) {
     randNumber = random(1, 7);

     if (oldRandNumber != randNumber)
     {
       numLoops--;

       if (!numLoops)
         Serial.println(randNumber);

       //clearAllDots();

       switch(oldRandNumber)
       {
         case 1:
           for(int i=0;i<oldRandNumber;i++)
             tft.fillCircle(one[i].x, one[i].y, 14, WHITE);
         break;
         case 2:
           for(int i=0;i<oldRandNumber;i++)
             tft.fillCircle(two[i].x, two[i].y, 14, WHITE);
         break;
         case 3:
           for(int i=0;i<oldRandNumber;i++)
             tft.fillCircle(three[i].x, three[i].y, 14, WHITE);
         break;
         case 4:
           for(int i=0;i<oldRandNumber;i++)
             tft.fillCircle(four[i].x, four[i].y, 14, WHITE);
         break;
         case 5:
           for(int i=0;i<oldRandNumber;i++)
             tft.fillCircle(five[i].x, five[i].y, 14, WHITE);
         break;
         case 6:
           for(int i=0;i<oldRandNumber;i++)
             tft.fillCircle(six[i].x, six[i].y, 14, WHITE);
         break;
         case 7:
           for(int i=0;i<oldRandNumber;i++)
             tft.fillCircle(seven[i].x, seven[i].y, 14, WHITE);
         break;
         case 8:
           for(int i=0;i<oldRandNumber;i++)
             tft.fillCircle(eight[i].x, eight[i].y, 14, WHITE);
         break;
         case 9:
           for(int i=0;i<oldRandNumber;i++)
             tft.fillCircle(nine[i].x, nine[i].y, 14, WHITE);
         break;
       }

       switch(randNumber)
       {
         case 1:
           for(int i=0;i<randNumber;i++)
             tft.fillCircle(one[i].x, one[i].y, 14, BLACK);
         break;
         case 2:
           for(int i=0;i<randNumber;i++)
             tft.fillCircle(two[i].x, two[i].y, 14, BLACK);
         break;
         case 3:
           for(int i=0;i<randNumber;i++)
             tft.fillCircle(three[i].x, three[i].y, 14, BLACK);
         break;
         case 4:
           for(int i=0;i<randNumber;i++)
             tft.fillCircle(four[i].x, four[i].y, 14, BLACK);
         break;
         case 5:
           for(int i=0;i<randNumber;i++)
             tft.fillCircle(five[i].x, five[i].y, 14, BLACK);
         break;
         case 6:
           for(int i=0;i<randNumber;i++)
             tft.fillCircle(six[i].x, six[i].y, 14, BLACK);
         break;
         case 7:
           for(int i=0;i<randNumber;i++)
             tft.fillCircle(seven[i].x, seven[i].y, 14, BLACK);
         break;
         case 8:
           for(int i=0;i<randNumber;i++)
             tft.fillCircle(eight[i].x, eight[i].y, 14, BLACK);
         break;
         case 9:
           for(int i=0;i<randNumber;i++)
             tft.fillCircle(nine[i].x, nine[i].y, 14, BLACK);
         break;
       }

       mustClear = true;
       oldRandNumber = randNumber;

       if (!numLoops)
       {
         //tft.setCursor(0, 153+ShiftY);
         tft.setCursor(tft.width() / 2 - 9, 153+ShiftY);
         tft.setTextColor(BLUE);
         tft.setTextSize(5);
         //tft.print((*ptr)[randNumber]);
         //tft.print(Numeros[randNumber]);
         tft.println(randNumber);

         //start = millis();
       }
     }
  }
}
