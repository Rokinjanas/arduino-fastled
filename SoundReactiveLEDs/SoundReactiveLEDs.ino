#include <FastLED.h>

const int sampleWindow = 50; // Sample window width in mS (50 mS = 20Hz)
unsigned int sample;
#define DATA_PIN    3
//#define CLK_PIN   4
#define LED_TYPE    WS2811
#define COLOR_ORDER BRG //GRB
#define NUM_LEDS    100
#define ChangeTime 60
CRGB leds[NUM_LEDS];
int randColor = random(0,100);
#define BRIGHTNESS          50
#define FRAMES_PER_SECOND  120

uint8_t gHue = 0;
uint8_t gCurrentPatternNumber = 0;

void setup() 
{ 
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  //FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  
  /* BUTTON */
  attachInterrupt(0, toggleState, RISING);

  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);
   Serial.begin(9600);
}

// STATE MACHINE
volatile boolean state = true;
void toggleState() {
  state = !state; 
}

// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm };


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
   
   //Serial.println(volts*100);
   //Serial.println(sample);
   Serial.println(state);
   EVERY_N_MILLISECONDS( 1000 ) { 
    randColor = random(128,255); 
    //randColor = random8();
    // 0 - 64     RED - ORANGE - YELLOW
    // 64 - 128   YELLOW - GREEN - AQUA
    // 128 - 192  AQUA - BLUE - PURPLE
    // 192 - 255  PURPLE - PINK - RED
   }
   EVERY_N_MILLISECONDS(20) { gHue++;};
   EVERY_N_SECONDS( ChangeTime ) { 
    if(state){
      nextPattern();  
    }
   } // change patterns periodically


   if(state) gPatterns[gCurrentPatternNumber]();
   if(!state) {
      //fadeToBlackBy( leds, NUM_LEDS, 20);
      ledScaleFromMiddle(volts * 100);
    }
   
   //ledScaleFromMiddle(volts*100);
   FastLED.delay(5);
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
  int maxVal = 25;
  int middle = NUM_LEDS/2;
  int scale = (volts*middle)/maxVal;
  if(volts >= maxVal) scale = middle+3;
  if(scale < middle) {
    CHSV randomColor (randColor,255,255);
    int s = false;
      for(int i=0; i < scale; i++){
        for(int a=0;a<4;a++){
          leds[middle+a] = randomColor;
          leds[middle-a] = randomColor;
        }
        leds[middle+i+4] = randomColor;
        leds[middle-i-4] = randomColor;
        if(s){
          FastLED.show();
          FastLED.delay(3);
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
/* EFEKTAI */
#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))


void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
}

void blank()
{
  fadeToBlackBy( leds, NUM_LEDS, 20); // This gives the goggles an "off" position
}

void rainbow() 
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
}

void rainbowWithGlitter() 
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}

void addGlitter( fract8 chanceOfGlitter) 
{
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void confetti() 
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16(13,0,NUM_LEDS);
  leds[pos] += CHSV( gHue, 255, 192);
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for( int i = 0; i < 8; i++) {
    leds[beatsin16(i+7,0,NUM_LEDS)] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}
/* !EFEKTAI */
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
