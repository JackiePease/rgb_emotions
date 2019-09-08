// https://github.com/FastLED/FastLED/blob/master/examples/XYMatrix/XYMatrix.ino

#include <FastLED.h>

#define LED_PIN  3

#define COLOR_ORDER GRB
#define CHIPSET     WS2811

#define BRIGHTNESS 255
// Helper functions for an two-dimensional XY matrix of pixels.
// Simple 2-D demo code is included as well.
//
//     XY(x,y) takes x and y coordinates and returns an LED index number,
//             for use like this:  leds[ XY(x,y) ] == CRGB::Red;
//             No error checking is performed on the ranges of x and y.
//
//     XYsafe(x,y) takes x and y coordinates and returns an LED index number,
//             for use like this:  leds[ XY(x,y) ] == CRGB::Red;
//             Error checking IS performed on the ranges of x and y, and an
//             index of "-1" is returned.  Special instructions below
//             explain how to use this without having to do your own error
//             checking every time you use this function.  
//             This is a slightly more advanced technique, and 
//             it REQUIRES SPECIAL ADDITIONAL setup, described below.


// Params for width and height
const uint8_t kMatrixWidth = 9;
const uint8_t kMatrixHeight = 9;
const uint8_t kUnlitPixelsPerRow = 1;

// Param for different pixel layouts
const bool    kMatrixSerpentineLayout = true;


// Set 'kMatrixSerpentineLayout' to false if your pixels are 
// laid out all running the same way, like this:
//
//     0 >  1 >  2 >  3 >  4
//                         |
//     .----<----<----<----'
//     |
//     5 >  6 >  7 >  8 >  9
//                         |
//     .----<----<----<----'
//     |
//    10 > 11 > 12 > 13 > 14
//                         |
//     .----<----<----<----'
//     |
//    15 > 16 > 17 > 18 > 19
//
// Set 'kMatrixSerpentineLayout' to true if your pixels are 
// laid out back-and-forth, like this:
//
//     0 >  1 >  2 >  3 >  4
//                         |
//                         |
//     9 <  8 <  7 <  6 <  5
//     |
//     |
//    10 > 11 > 12 > 13 > 14
//                        |
//                        |
//    19 < 18 < 17 < 16 < 15
//
// Bonus vocabulary word: anything that goes one way 
// in one row, and then backwards in the next row, and so on
// is call "boustrophedon", meaning "as the ox plows."


// This function will return the right 'led index number' for 
// a given set of X and Y coordinates on your matrix.  
// IT DOES NOT CHECK THE COORDINATE BOUNDARIES.  
// That's up to you.  Don't pass it bogus values.
//
// Use the "XY" function like this:
//
//    for( uint8_t x = 0; x < kMatrixWidth; x++) {
//      for( uint8_t y = 0; y < kMatrixHeight; y++) {
//      
//        // Here's the x, y to 'led index' in action: 
//        leds[ XY( x, y) ] = CHSV( random8(), 255, 255);
//      
//      }
//    }
//
//
uint16_t XY( uint8_t x, uint8_t y)
{
  uint16_t i;
  if( kMatrixSerpentineLayout == false) {
    i = (y * kMatrixWidth) + x;
  }

  if( kMatrixSerpentineLayout == true) {
    if( y & 0x01) {
      // Odd rows run backwards
      uint8_t reverseX = (kMatrixWidth - 1) - x;
      // i = (y * kMatrixWidth) + reverseX;
      i = (y * kMatrixWidth) + reverseX + (y * kUnlitPixelsPerRow);
    } else {
      // Even rows run forwards
      // i = (y * kMatrixWidth) + x;
      i = (y * kMatrixWidth) + x + (y * kUnlitPixelsPerRow);
    }
  }
  
  return i;
}


// Once you've gotten the basics working (AND NOT UNTIL THEN!)
// here's a helpful technique that can be tricky to set up, but 
// then helps you avoid the needs for sprinkling array-bound-checking
// throughout your code.
//
// It requires a careful attention to get it set up correctly, but
// can potentially make your code smaller and faster.
//
// Suppose you have an 8 x 5 matrix of 40 LEDs.  Normally, you'd
// delcare your leds array like this:
//    CRGB leds[40];
// But instead of that, declare an LED buffer with one extra pixel in
// it, "leds_plus_safety_pixel".  Then declare "leds" as a pointer to
// that array, but starting with the 2nd element (id=1) of that array: 
//    CRGB leds_with_safety_pixel[41];
//    CRGB* const leds( leds_plus_safety_pixel + 1);
// Then you use the "leds" array as you normally would.
// Now "leds[0..N]" are aliases for "leds_plus_safety_pixel[1..(N+1)]",
// AND leds[-1] is now a legitimate and safe alias for leds_plus_safety_pixel[0].
// leds_plus_safety_pixel[0] aka leds[-1] is now your "safety pixel".
//
// Now instead of using the XY function above, use the one below, "XYsafe".
//
// If the X and Y values are 'in bounds', this function will return an index
// into the visible led array, same as "XY" does.
// HOWEVER -- and this is the trick -- if the X or Y values
// are out of bounds, this function will return an index of -1.
// And since leds[-1] is actually just an alias for leds_plus_safety_pixel[0],
// it's a totally safe and legal place to access.  And since the 'safety pixel'
// falls 'outside' the visible part of the LED array, anything you write 
// there is hidden from view automatically.
// Thus, this line of code is totally safe, regardless of the actual size of
// your matrix:
//    leds[ XYsafe( random8(), random8() ) ] = CHSV( random8(), 255, 255);
//
// The only catch here is that while this makes it safe to read from and
// write to 'any pixel', there's really only ONE 'safety pixel'.  No matter
// what out-of-bounds coordinates you write to, you'll really be writing to
// that one safety pixel.  And if you try to READ from the safety pixel,
// you'll read whatever was written there last, reglardless of what coordinates
// were supplied.

#define NUM_LEDS (kMatrixWidth * kMatrixHeight) + (kMatrixHeight * kUnlitPixelsPerRow)
CRGB leds_plus_safety_pixel[ NUM_LEDS + 1];
CRGB* const leds( leds_plus_safety_pixel + 1);

uint16_t XYsafe( uint8_t x, uint8_t y)
{
  if( x >= kMatrixWidth) return -1;
  if( y >= kMatrixHeight) return -1;
  return XY(x,y);
}

// Demo that USES "XY" follows code below

void loop()
{
//    uint32_t ms = millis();
//    int32_t yHueDelta32 = ((int32_t)cos16( ms * (27/1) ) * (350 / kMatrixWidth));
//    int32_t xHueDelta32 = ((int32_t)cos16( ms * (39/1) ) * (310 / kMatrixHeight));
//    DrawOneFrame( ms / 65536, yHueDelta32 / 32768, xHueDelta32 / 32768);
//    if( ms < 5000 ) {
//      FastLED.setBrightness( scale8( BRIGHTNESS, (ms * 256) / 5000));
//    } else {
//      FastLED.setBrightness(BRIGHTNESS);
//    }
//     VerticalHorizontalRainbow(10);
//       SingleHSV(10, 255, 255);
//       FastLED.show();
//       delay(1000);
//         SingleHSV(138, 255, 255);
//       FastLED.show();
//       delay(1000);      
//        HueGradient(); 
//        FastLED.show();    
        //RowsSingleHSV();

// Random colours 
   for (byte i = 0; i < 25; i++) {
      RandomColours();
      FastLED.show();
      delay(50);
   }

// Polluted sparkles
 
    for (byte i = 0; i < 10; i++) {      
      SingleHSV(70, 210, 150);
      SparklingOverlay(50, 74, 210, 150);
      SparklingOverlay(25, 81, 255, 150);
      SparklingOverlay(25, 81, 210, 150);
      FastLED.show(); 
      delay(1000);
    }

    Alert();
    delay(1000);

    PrettyPinkSparkles();
    PrettyBlueSparkles();     

    DrawOneFrameHSV(40, 3, 3, 255, 0, 0, 150, 0, 0);
    FastLED.show();
    delay(5000);

      for (byte i = 0; i < 50; i++) {  
        DrawOneFrameHSV(60, i/2, i/2, 0, i/4, i/4, 0, i/3, i/3);
        FastLED.show();
        delay(100);
      }

}

void SparklingOverlay(byte sparkleChance, byte H, byte S, byte V)
{
  for( byte y = 0; y < kMatrixHeight; y++) {
    for( byte x = 0; x < kMatrixWidth; x++) {
        if (Sparkling(sparkleChance)) {
          leds[ XY(x, y)]  = CHSV( H, S, V);
        }
    }
  }
}

boolean Sparkling(byte sparkleChance)
{
    return sparkleChance > 0 and sparkleChance >= random(256);
}

void VerticalHorizontalRainbow(byte count)
{
      for (byte i = 0; i < count; i++) {
         VerticalRainbow();
         FastLED.show();
         delay(500);
    
         HorizontalRainbow();
         FastLED.show();
         delay(500);
      }
}

void Alert()
{

        for (byte i = 0; i < 30; i++) {      
          RowSingleHSV(6, 85, 255, 255);
          RowSingleHSV(5, 200, 255, 255);
          RowSingleHSV(4, 85, 255, 255);
          RowSingleHSV(3, 200, 255, 255);        
          RowSingleHSV(2, 85, 255, 255);
          FastLED.show(); 
          delay(50);
                    
          RowSingleHSV(6, 200, 255, 255);
          RowSingleHSV(5, 85, 255, 255);
          RowSingleHSV(4, 200, 255, 255);
          RowSingleHSV(3, 85, 255, 255);        
          RowSingleHSV(2, 200, 255, 255);
          FastLED.show(); 
          delay(50);
        }  
}
void PrettyPinkSparkles()
{
        SparklingOverlay(25, 210, 100, 255);
        FastLED.show();
        delay(300);
        SparklingOverlay(25, 216, 150, 255);
        FastLED.show();  
        delay(300);
        SparklingOverlay(25, 230, 210, 255);
        FastLED.show();  
        delay(300);
        SparklingOverlay(25, 210, 100, 255);
        FastLED.show();
        delay(300);
        SparklingOverlay(25, 216, 150, 255);
        FastLED.show();  
        delay(300);
        SparklingOverlay(25, 230, 210, 255);
        FastLED.show();  
        delay(300);
        SparklingOverlay(25, 210, 100, 255);
        FastLED.show();
        delay(300);
        SparklingOverlay(25, 216, 150, 255);
        FastLED.show();  
        delay(300);
        SparklingOverlay(25, 230, 210, 255);
        FastLED.show();  
        delay(300);
        SparklingOverlay(25, 210, 100, 255);
        FastLED.show();
        delay(300);
        SparklingOverlay(25, 216, 150, 255);
        FastLED.show();  
        delay(300);
        SparklingOverlay(25, 230, 210, 255);
        FastLED.show();  
        delay(300);
        
        for (byte i = 0; i < 10; i++) {
            SingleHSV(216, 210, 255);
            SparklingOverlay(25, 230, 50, 255);
            SparklingOverlay(25, 210, 255, 255);                
            SparklingOverlay(25, 200, 100, 255);
            FastLED.show();
            delay(500);
            SparklingOverlay(10, 216, 0, 255);
            FastLED.show();  
            delay(50);
        }      
}

void PrettyBlueSparkles()
{
        for (byte i = 0; i < 20; i++) {
            SingleHSV(148, 255, 255);
            FastLED.show();
            delay(500);
            SparklingOverlay(10, 148, 0, 255);
            FastLED.show();  
            delay(50);
        }   
}

void HueGradient()
{
  for( byte y = 0; y < kMatrixHeight; y++) {
    for( byte x = 0; x < kMatrixWidth; x++) {
      leds[ XY(x, y)]  = CHSV( y * 255 / kMatrixHeight + (x * 255 / (kMatrixHeight * kMatrixWidth)), 255, 255);
//      Serial.print(x);
//      Serial.print(" ");
//      Serial.print(y);
//      Serial.print(" ");
//      Serial.println(y * 255 / kMatrixHeight + (x * 255 / (kMatrixHeight * kMatrixWidth)));
    }
  }  
}
void RowsSaturation()
{
        RowSaturation(0, 20, 255);  // Dull Orange
        RowSaturation(1, 30, 255);  // Brown
        RowSaturation(2, 74, 255);  // Polluted Yellow
        RowSaturation(3, 81, 255);  // Polluted Green
        RowSaturation(4, 113, 255); // Polluted Turquoise
        RowSaturation(5, 138, 255); // Natural sky
        RowSaturation(6, 148, 255); // Bluer sky
        RowSaturation(7, 195, 255); // Purple
        RowSaturation(8, 216, 255); // Pretty pink
}

void RandomColours()
{
  for( byte y = 0; y < kMatrixHeight; y++) {  
    for( byte x = 0; x < kMatrixWidth; x++) {
      leds[ XY(x, y)]  = CHSV( random8(), 255, 255);
    }
  }
}

void RowSaturation(byte Row, byte H, byte V)
{
  for (byte x = 0; x < kMatrixWidth; x++) {
    leds[ XY(x, Row)] = CHSV( H, 255 - (x * 15), V);
//    Serial.print(x); 
//    Serial.print(" ");
//    Serial.println(255 - (x * 15));
  }
}
void RowsSingleHSV()
{
        RowSingleHSV(0, 30, 255, 150);  // Brown
        RowSingleHSV(1, 30, 210, 150);  // Brown
        RowSingleHSV(2, 74, 255, 150);  // Polluted Yellow
        RowSingleHSV(3, 74, 210, 150);  // Polluted Yellow
        RowSingleHSV(4, 81, 255, 150);  // Polluted Green
        RowSingleHSV(5, 81, 210, 150);  // Polluted Green
        RowSingleHSV(6, 113, 255, 150); // Polluted Turquoise
        RowSingleHSV(7, 113, 210, 150); // Polluted Turquoise
        RowSingleHSV(8, 74, 210, 150); // Polluted Yellow - less bright
}

void RowSingleHSV(byte Row, byte H, byte S, byte V)
{
  for (byte x = 0; x < kMatrixWidth; x++) {
    leds[ XY(x, Row)] = CHSV( H, S, V);
//    Serial.print(x); 
//    Serial.print(" ");
  }
}

void SingleHSV(byte H, byte S, byte V)
{
  for( byte y = 0; y < kMatrixHeight; y++) {  
    for( byte x = 0; x < kMatrixWidth; x++) {
      leds[ XY(x, y)]  = CHSV( H, S, V);
    }
  }
}

void VerticalRainbow()
{
      for( byte y = 0; y < kMatrixHeight; y++) {      
          for( byte x = 0; x < kMatrixWidth; x++) {
              leds[ XY(x, y)]  = CHSV( y*25.5, 255, 255);
          };
      };  
}

void HorizontalRainbow()
{
      for( byte x = 0; x < kMatrixHeight; x++) {      
          for( byte y = 0; y < kMatrixWidth; y++) {
              leds[ XY(x, y)]  = CHSV( x*25.5, 255, 255);
          };
      };
}

void DrawOneFrame( byte startHue8, int8_t yHueDelta8, int8_t xHueDelta8)
{
  byte lineStartHue = startHue8;
  for( byte y = 0; y < kMatrixHeight; y++) {
    lineStartHue += yHueDelta8;
    byte pixelHue = lineStartHue;      
    for( byte x = 0; x < kMatrixWidth; x++) {
      pixelHue += xHueDelta8;
      leds[ XY(x, y)]  = CHSV( pixelHue, 255, 255);
    }
  }
}

void DrawOneFrameHSV( byte startHue8, int8_t yHueDelta8, int8_t xHueDelta8, byte startS8, int8_t ySDelta8, int8_t xSDelta8, byte startV8, int8_t yVDelta8, int8_t xVDelta8)
{
  byte lineStartHue = startHue8;
  byte lineStartS = startS8;
  byte lineStartV = startV8;
  for( byte y = 0; y < kMatrixHeight; y++) {
    lineStartHue += yHueDelta8;
    lineStartS += ySDelta8;
    lineStartV += yVDelta8;
    byte pixelHue = lineStartHue;  
    byte pixelS = lineStartS;   
    byte pixelV = lineStartV; 
    for( byte x = 0; x < kMatrixWidth; x++) {
      pixelHue += xHueDelta8;
      pixelS += xSDelta8;
      pixelV += xVDelta8;
      leds[ XY(x, y)]  = CHSV( pixelHue, pixelS, pixelV);
    }
  }
}

void setup() {
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);
  FastLED.setBrightness( BRIGHTNESS );
  // Serial.begin(115200);
}
