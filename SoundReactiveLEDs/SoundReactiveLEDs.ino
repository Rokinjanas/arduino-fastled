#include <FastLED.h>
const int sampleWindow = 50; // Sample window width in mS (50 mS = 20Hz)
unsigned int sample;
#define DATA_PIN    3
//#define CLK_PIN   4
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS    80

CRGB leds[NUM_LEDS];
int randColor = random(0,100);
#define BRIGHTNESS          100
#define FRAMES_PER_SECOND  120

void setup() 
{ 
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  //FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);
   Serial.begin(9600);
}
 
 
void loop() 
{
   unsigned long startMillis= millis();  // Start of sample window
   unsigned int peakToPeak = 0;   // peak-to-peak level
 
   unsigned int signalMax = 0;
   unsigned int signalMin = 1024;
 
   // collect data for 50 mS
   while (millis() - startMillis < sampleWindow)
   {
      sample = analogRead(0);
      if (sample < 1024)  // toss out spurious readings
      {
         if (sample > signalMax)
         {
            signalMax = sample;  // save just the max levels
         }
         else if (sample < signalMin)
         {
            signalMin = sample;  // save just the min levels
         }
      }
   }
   peakToPeak = signalMax - signalMin;  // max - min = peak-peak amplitude
   float volts = (peakToPeak * 5.0) / 1024;  // convert to volts
 
   Serial.println(volts*100);
   Serial.println(sample);
   EVERY_N_MILLISECONDS( 1000 ) { 
    randColor = random(0,64); 
    //randColor = random8();
    // 0 - 64     RED - ORANGE - YELLOW
    // 64 - 128   YELLOW - GREEN - AQUA
    // 128 - 192  AQUA - BLUE - PURPLE
    // 192 - 255  PURPLE - PINK - RED
   }
   ledScaleFromMiddle(volts*100);
   //FastLED.delay(10);
}


/*void ledScale(int volts) {
  CRGB bgColor( 0, 0, 0);
  int maxVal = 20;
  
  int scale = (volts*NUM_LEDS)/maxVal;
  if(volts >= maxVal) scale = NUM_LEDS;
  if(scale < NUM_LEDS) {
    CHSV randomColor (random8(),255,255);
    
      for(int i=0; i < scale; i++){
        leds[i] = randomColor;
        if(i == scale-1) leds[i] = CRGB::Red;
      }
  }
  fadeTowardColor(leds, NUM_LEDS, bgColor, 50);


}*/

void ledScaleFromMiddle(int volts) {
  CRGB bgColor( 0, 0, 0);
  int maxVal = 20;
  int middle = NUM_LEDS/2;
  int scale = (volts*middle)/maxVal;
  if(volts >= maxVal) scale = middle+3;
  if(scale < middle) {
    CHSV randomColor (randColor,255,255);
    int s = false;
      for(int i=0; i < scale; i++){
        for(int a=0;a<3;a++){
          leds[middle+a] = CRGB::Red;
          leds[middle-a] = CRGB::Red;
        }
        leds[middle+i+3] = randomColor;
        leds[middle-i-3] = randomColor;
        if(s){
          FastLED.show();
          FastLED.delay(2);
          s = false;
        } else {
          s = true;
        }
        //FastLED.show();
        //if(i == middle-1) leds[i] = CRGB::Red;
        //FastLED.delay(1);
      }
  }
  fadeTowardColor(leds, NUM_LEDS, bgColor, 50);

}

void nblendU8TowardU8( uint8_t& cur, const uint8_t target, uint8_t amount)
{
  if( cur == target) return;
  
  if( cur < target ) {
    uint8_t delta = target - cur;
    delta = scale8_video( delta, amount);
    cur += delta;
  } else {
    uint8_t delta = cur - target;
    delta = scale8_video( delta, amount);
    cur -= delta;
  }
}

// Blend one CRGB color toward another CRGB color by a given amount.
// Blending is linear, and done in the RGB color space.
// This function modifies 'cur' in place.
CRGB fadeTowardColor( CRGB& cur, const CRGB& target, uint8_t amount)
{
  nblendU8TowardU8( cur.red,   target.red,   amount);
  nblendU8TowardU8( cur.green, target.green, amount);
  nblendU8TowardU8( cur.blue,  target.blue,  amount);
  return cur;
}

// Fade an entire array of CRGBs toward a given background color by a given amount
// This function modifies the pixel array in place.
void fadeTowardColor( CRGB* L, uint16_t N, const CRGB& bgColor, uint8_t fadeAmount)
{
  for( uint16_t i = 0; i < N; i++) {
    fadeTowardColor( L[i], bgColor, fadeAmount);
  }
}
