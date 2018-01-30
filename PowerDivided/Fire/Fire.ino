#include <Wire.h>

#include <FastLED.h>

#define DEBUG     true
#define WDEBUG false
#include <Console.h>


#define LED_PIN     5
#define COLOR_ORDER GRB
#define CHIPSET     LPD8806
#define NUM_LEDS    46
#define DATA_PIN 8
#define CLOCK_PIN 9

#define BRIGHTNESS  3
#define FRAMES_PER_SECOND 60

bool gReverseDirection = false;

CRGB leds[NUM_LEDS];

// COOLING: How much does the air cool as it rises?
// Less cooling = taller flames.  More cooling = shorter flames.
// Default 55, suggested range 20-100 
int COOLING = 30;

// SPARKING: What chance (out of 255) is there that a new spark will be lit?q
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.
int SPARKING = 120;


// Fire2012 with programmable Color Palette
//
// This code is the same fire simulation as the original "Fire2012",
// but each heat cell's temperature is translated to color through a FastLED
// programmable color palette, instead of through the "HeatColor(...)" function.
//
// Four different static color palettes are provided here, plus one dynamic one.
// 
// The three static ones are: 
//   1. the FastLED built-in HeatColors_p -- this is the default, and it looks
//      pretty much exactly like the original Fire2012.
//
//  To use any of the other palettes below, just "uncomment" the corresponding code.
//
//   2. a gradient from black to red to yellow to white, which is
//      visually similar to the HeatColors_p, and helps to illustrate
//      what the 'heat colors' palette is actually doing,
//   3. a similar gradient, but in blue colors rather than red ones,
//      i.e. from black to blue to aqua to white, which results in
//      an "icy blue" fire effect,
//   4. a simplified three-step gradient, from black to red to white, just to show
//      that these gradients need not have four components; two or
//      three are possible, too, even if they don't look quite as nice for fire.
//
// The dynamic palette shows how you can change the basic 'hue' of the
// color palette every time through the loop, producing "rainbow fire".

CRGBPalette16 gPal;
byte freqResponse[9];

void setup() {
  

  
  delay(1000); // sanity delay
  //FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  //FastLED.addLeds<LPD8806, 5, A4,COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<CHIPSET, DATA_PIN, CLOCK_PIN,COLOR_ORDER>(leds, NUM_LEDS); //.setCorrection( TypicalLEDStrip )
  FastLED.setBrightness( BRIGHTNESS );

if(DEBUG) 
{ 
  Serial.begin(115200);
Serial.println("Serial Connected! ");
}

 if (WDEBUG)
  {
    Bridge.begin();
    Console.begin();
//    while (!Console) {
//      ; // wait for Console port to connect.
//    }
    Console.println("You're connected to the Console!!!!");
  }

  
  // This first palette is the basic 'black body radiation' colors,
  // which run from black to red to bright yellow to white.
 // gPal = HeatColors_p;
  
  // These are other ways to set up the color palette for the 'fire'.
  // First, a gradient from black to red to yellow to white -- similar to HeatColors_p
     //gPal = CRGBPalette16( CRGB::Black, CRGB::Red, CRGB::Yellow, CRGB::White);
  
  // Second, this palette is like the heat colors, but blue/aqua instead of red/yellow
  //   gPal = CRGBPalette16( CRGB::Black, CRGB::Blue, CRGB::Aqua,  CRGB::White);
  
  // Third, here's a simpler, three-step gradient, from black to red to white
     //gPal = CRGBPalette16( CRGB::Black, CRGB::Green, CRGB::White);

     //gPal = CRGBPalette16( CRGB::Black, CRGB::Blue, 0xFF77100);  //CRGB::Red, 0xFF8800);
    //gPal = CRGBPalette16( CRGB::Black, CRGB::Red, 0xFF8800);  //CRGB::Red, 0xFF8800);
    gPal = CRGBPalette16( CRGB::Black, 0x00FF88, CRGB::Red);  //CRGB::Red, 0xFF8800);
    
Wire.begin();                // join i2c bus with address #9
  //Wire.onReceive(receiveEvent); // register event
  
}
//
//const int sampleWindow = 50; // Sample window width in mS (50 mS = 20Hz)
//unsigned int sample;

int highHeat = 255;
int lowHeat = 160;

void loop()
{
  // read pot values
  //COOLING = map(analogRead(A1),0,1023,0,255);
  //if(DEBUG) Serial.print(COOLING);
  //if(DEBUG) Serial.print(" ");
  int sensitivity = map(analogRead(A2),0,1023,0,255);
  COOLING = map(sensitivity,0,255, 100,20);
  //SPARKING = 
  //if(DEBUG) Serial.print(SPARKING);
  //if(DEBUG) Serial.print(" ");
  int brightnessPot = map(analogRead(A3),0,1023,0,255);
  FastLED.setBrightness( brightnessPot );
  //if(DEBUG) Serial.println(brightnessPot);



byte firstcolor = map(analogRead(A1),0,1023,0,255);
byte secondcolor = map(analogRead(A0),0,1023,0,255);
    if(DEBUG) 
{ 
Serial.print(firstcolor);
Serial.print("   ");
Serial.print(secondcolor);
Serial.print("   ");
Serial.print(COOLING);
Serial.print("   ");
Serial.print(SPARKING);
Serial.print("   ");
Serial.println(brightnessPot);

}



gPal = CRGBPalette16( CRGB::Black, CHSV(firstcolor, 255, 255), CHSV(secondcolor, 255, 255));  //CRGB::Red, 0xFF8800);
  
  // read values from slave fft device:
  Wire.requestFrom(9, 8);    // request 6 bytes from slave device #8
  byte index = 0;
  int sum = 0;
    while(Wire.available() > 0 && index < 8)
    {
      freqResponse[index] = Wire.read();
      sum += freqResponse[index];
      index++;
    }
    freqResponse[8] = sum/8;

    
    if(DEBUG) 
{ 
Serial.println(freqResponse[0]);
}


   
  // Add entropy to random number generator; we use a lot of it.
  random16_add_entropy( random());

  // Fourth, the most sophisticated: this one sets up a new palette every
  // time through the loop, based on a hue that changes every time.
  // The palette is a gradient from black, to a dark color based on the hue,
  // to a light color based on the hue, to white.
  //
     //static uint8_t hue = 0;
     //hue++;
     //CRGB darkcolor  = CHSV(hue,255,192); // pure hue, three-quarters brightness
     //CRGB lightcolor = CHSV(hue,128,255); // half 'whitened', full brightness
     //gPal = CRGBPalette16( CRGB::Black, darkcolor, lightcolor, CRGB::White);


  Fire2012WithPalette(); // run simulation frame, using palette colors
  
  FastLED.show(); // display this frame
  FastLED.delay(1000 / FRAMES_PER_SECOND);
}








// Fire2012 by Mark Kriegsman, July 2012
// as part of "Five Elements" shown here: http://youtu.be/knWiGsmgycY
//// 
// This basic one-dimensional 'fire' simulation works roughly as follows:
// There's a underlying array of 'heat' cells, that model the temperature
// at each point along the line.  Every cycle through the simulation, 
// four steps are performed:
//  1) All cells cool down a little bit, losing heat to the air
//  2) The heat from each cell drifts 'up' and diffuses a little
//  3) Sometimes randomly new 'sparks' of heat are added at the bottom
//  4) The heat from each cell is rendered as a color into the leds array
//     The heat-to-color mapping uses a black-body radiation approximation.
//
// Temperature is in arbitrary units from 0 (cold black) to 255 (white hot).
//
// This simulation scales it self a bit depending on NUM_LEDS; it should look
// "OK" on anywhere from 20 to 100 LEDs without too much tweaking. 
//
// I recommend running this simulation at anywhere from 30-100 frames per second,
// meaning an interframe delay of about 10-35 milliseconds.
//
// Looks best on a high-density LED setup (60+ pixels/meter).
//
//
// There are two main parameters you can play with to control the look and
// feel of your fire: COOLING (used in step 1 above), and SPARKING (used
// in step 3 above).
//



void Fire2012WithPalette()
{
// Array of temperature readings at each simulation cell
  static byte heat[NUM_LEDS];



  // Step 1.  Cool down every cell a little
    for( int i = 0; i < NUM_LEDS; i++) {
      heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / NUM_LEDS) + 2));
    }
  
    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for( int k= NUM_LEDS - 1; k >= 2; k--) {
      heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
    }
    
//    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
//    if( random8() < SPARKING ) {
//      int y = random8(9,12);
//      heat[y] = qadd8( heat[y], random8(freqResponse[8]/2,freqResponse[8]) );
//    }
        // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if( random8() < SPARKING ) 
    {
       for( int k = 0; k < 9; k++) {
      heat[k] = qadd8( heat[k], freqResponse[k] );

    }
     // int y = random8(0,9);
      //heat[y] = qadd8( heat[y], random8(freqResponse[y],freqResponse[y]) );
       
    }
    

    // Step 3.  border based flickr

//      int y = random8(4);
//      heat[y] = qadd8( heat[y], highHeat );

//    // Step 4.  Map from heat cells to LED colors
//    for( int k = 0; k < 9; k++) {
//      // Scale the heat value from 0-255 down to 0-240
//      // for best results with color palettes.
//      byte colorindex = scale8( map(freqResponse[k],0,9,0,255), 240);
//      CRGB color = ColorFromPalette( gPal, colorindex);
//      leds[k] = color;
//    }
//    
//
//    // Step 4.  Map from heat cells to LED colors
//    for( int j = 9; j < NUM_LEDS; j++) {
//      // Scale the heat value from 0-255 down to 0-240
//      // for best results with color palettes.
//      byte colorindex = scale8( heat[j], 240);
//      CRGB color = ColorFromPalette( gPal, colorindex);
//      leds[j] = color;
//    }

    // Step 4.  Map from heat cells to LED colors
//    for( int k = 0; k < 9; k++) {
//      // Scale the heat value from 0-255 down to 0-240
//      // for best results with color palettes.
//      byte colorindex = scale8( map(freqResponse[k],0,9,0,255), 240);
//      CRGB color = ColorFromPalette( gPal, colorindex);
//      leds[k] = color;
//    }
    

    // Step 4.  Map from heat cells to LED colors
    for( int j = 0; j < NUM_LEDS; j++) {
      // Scale the heat value from 0-255 down to 0-240
      // for best results with color palettes.
      byte colorindex = scale8( heat[j], 240);
      CRGB color = ColorFromPalette( gPal, colorindex);
      leds[j] = color;
    }

    
}



