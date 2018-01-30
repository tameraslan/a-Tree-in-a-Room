  // a Tree in a Room
  // Tree Generic, with rotating encoder with click and internal color display
  
  #include <PWM.h>
  
  #include <FastLED.h>
  #include "ClickEncoderMod.h"
  #include <TimerOne.h>
  #define DEBUG false
  
  
  // tree properties:
  #define TRUNKHEIGHT 27
  // led properties:
  #define LED_PIN     11
  #define CHIPSET     WS2812B
  #define COLOR_ORDER GRB
  #define NUM_LEDS TRUNKHEIGHT
  //#define FRAMES_PER_SECOND 60
  int FPS = 60;
  // LED object constructions
  CRGB leds[NUM_LEDS];
  //CRGBPalette16 gPal;
  int ledBrightness = 255;
  
  
  
  // fire algo coef:
  int highHeat = 255;
  int lowHeat = 160;
  bool gReverseDirection = false;
  // COOLING: How much does the air cool as it rises? Less cooling = taller flames.  More cooling = shorter flames.
  // Default 55, suggested range 20-100
  #define COOLING  55
  //int cooling = 127;
  // SPARKING: What chance (out of 255) is there that a new spark will be lit? Higher chance = more roaring fire.  Lower chance = more flickery fire.
  // Default 120, suggested range 50-200.
  #define SPARKING 50
  //int sparking = 200;

  
  // Interface objects and variables
  ClickEncoderMod *encoder;
  int16_t last, value;
  void timerIsr()
  {
    encoder->service();
  }
  // control states:
  int knobStateCount = 3;
  enum knobState {
    brightness = 0,
    firstHue = 1,
    velocity = 2,
    //flicker = 3,
    //cool = 4
  };
  knobState currentKnobState = 0;



  int32_t frequency = 35; //frequency (in Hz)

  
  void setup()
  {

    delay(1000); // sanity delay
    
    InitTimersSafe(); 
    
    
    bool success = SetPinFrequencySafe(10, frequency);
    success &= SetPinFrequencySafe(9, frequency);
    success &= SetPinFrequencySafe(3, frequency);


    
    encoder = new ClickEncoderMod(A0, A3, 7, 1, LOW);
  
    Timer1.initialize(1000);
    Timer1.attachInterrupt(timerIsr);
  
    last = -1;
  
    delay(300); // sanity delay
    if (DEBUG) Serial.begin(9600);
  
    FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
    // define initial palette: fire colors
    
    //gPal = CRGBPalette16( CRGB::Black, firstColor, secondColor);
    pinMode(3, OUTPUT);
    pinMode(9, OUTPUT);
    pinMode(10, OUTPUT);

    TCCR2B = (TCCR2B & 0b11111000) | 0x05;
  }
  
  
  // List of patterns to cycle through.  Each is defined as a separate function below.
  typedef void (*SimplePatternList[])();
  SimplePatternList gPatterns = {Fire2012WithPalette, rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm };
  uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
  uint8_t gHue = 0; // rotating "base color" used by many of the patterns
  
  
  
  
  //long previousMillis = 0;        // will store last time LED was updated
  long pressTime = 0;
  long interval = 3000; 
  bool rotation = false;
  void loop()
  {
  
  
    // Interface controls
    // button reading
    ClickEncoderMod::Button b = encoder->getButton();
  
    if (b != ClickEncoderMod::Open)
    {
      //Serial.print("Button: ");
  
      #define VERBOSECASE(label) case label: Serial.println(#label); break;
  
      switch (b)
      {
          //VERBOSECASE(ClickEncoderMod::Pressed);
          VERBOSECASE(ClickEncoderMod::Held)
          //VERBOSECASE(ClickEncoderMod::Released)
        case ClickEncoderMod::Pressed:
          // record pressing 
          pressTime = millis();
          if (DEBUG)
          {
            Serial.print("ClickEncoderMod::Pressed at ");
            Serial.println(pressTime);
          }
          break;
        case ClickEncoderMod::Released:
          // compare to press time
          if (DEBUG)
              {
                Serial.print("ClickEncoderMod::Released at ");
                Serial.println(millis());
              }
          if (millis()-pressTime >= interval)
            {
              if (DEBUG)
              {
               
                Serial.println("Rotation enabled");
              }
              // enable hue change. 
              rotation = !rotation; 
            }
            
          break;
          
        case ClickEncoderMod::DoubleClicked:
          if (DEBUG)
          {
            Serial.print("ClickEncoderMod::DoubleClicked");
            Serial.println("  changing viz ");
          }
          nextPattern();
          break;
  
        case ClickEncoderMod::Clicked:
          currentKnobState = currentKnobState + 1;
          if (currentKnobState >= knobStateCount)
          {
            currentKnobState = 0;
          }
          if (DEBUG)
          {
            Serial.print("ClickEncoderMod::Clicked");
            Serial.print(" current knob state: ");
            Serial.println(currentKnobState);
          }
          break;
      }
    }
  
    // read encode change: If difference in encoder, than non-zero value.
    // if no change: returns zero.
    int encoderChangeValue = 0;
    encoderChangeValue = encoder->getValue();
  
    if ( encoderChangeValue != 0)
    {
      // encoer returns three values for each click.
  
      // Implemetn code here to remove encoder noise:
      // noise appears in last values, when rotation happens.
      // it is usually the opposite value of the previous 3-4 valiues.

      if (DEBUG)
      {
        Serial.print("Encoder value: ");
        Serial.print(encoderChangeValue);
        Serial.print("  ");
      }
  
  
      switch (currentKnobState)
      {
        case brightness:
          ledBrightness = ledBrightness + encoderChangeValue;
          if (ledBrightness > 255) ledBrightness = 255;
          if (ledBrightness < 0) ledBrightness = 0;
  
          if (DEBUG)
          {
            Serial.print("Brightness value: ");
            Serial.print(ledBrightness);
            Serial.print("  ");
          }
  
          FastLED.setBrightness(ledBrightness);
  
          break;
  
        case firstHue:
          
            gHue += encoderChangeValue;
            if (DEBUG)
            {
              Serial.print("Hue value: ");
              Serial.print(gHue);
              Serial.print("  ");
            }
            //CHSV hsv( 0, 0, 0);
            //hsv = rgb2hsv_approximate(firstColor);
            //hsv.hue += encoder->getValue();
            // if (DEBUG) Serial.println(hsv.hue);
            //hsv2rgb_rainbow( hsv, firstColor);
          
          break;

        case velocity:

            // limit between 5 and 1000;
            FPS = FPS + encoderChangeValue;
            //constrain(FPS,5,500);
            if (FPS < 5) FPS = 5;
            if (FPS > 500) FPS = 500;
            if(DEBUG) 
            {
              Serial.print("FPS value: ");
              Serial.print(FPS); 
            }
          
          break;
  

      }
      //gPal = CRGBPalette16( CRGB::Black, firstColor, secondColor);
      //gpal = CHSVPalette16(0,0,0,);
      if(DEBUG) Serial.println();
    }

    // Also show the hue value on the rotater leds:
    //CHSV hsvTemp( gHue, 255, ledBrightness);
    //LEDS are N-drops, must be negtive! 
    //CRGB rgbButton = CRGB( 0, 0, 0);
    //CRGB rgbButton = CRGB( 255, 255, 255);
    //hsv2rgb_rainbow( hsvTemp, rgbTemp);
    //
    //rgbButton.setHSV( gHue,255,ledBrightness);
    //rgbButton.setHSV( gHue,255,255); 
    //rgbButton.setHue( gHue);
    //rgbButton.maximizeBrightness();

    CRGB rgbButton = leds[0];
    rgbButton %= ledBrightness;

    
//    // Write red value:
//    Serial.print(rgbButton.r);
//    Serial.print(" ");
//    Serial.print(rgbButton.g);
//    Serial.print(" ");
//    Serial.print(rgbButton.b);
//    Serial.print(" ");
    int catR = 255-int(rgbButton.r);
    int catG = 255-int(rgbButton.g);
    int catB = 255-int(rgbButton.b);
    //Serial.println(catR);
    //analogWrite(6, 255-int(rgbButton.b));
    //analogWrite(9, 255-int(rgbButton.g));
    //analogWrite(10, catR);
//    analogWrite(6, 255-leds[0].b);
//    analogWrite(9, 255-leds[0].g);
//    analogWrite(10, 255-leds[0].r);
    // map knobval
    //highHeat = 5;
    //lowHeat = 0;
  pwmWrite(10, catR);
  pwmWrite(9, catG);
  pwmWrite(3, catB);
  
  
  
    /*
      int outputValue = int(map(volts, 0, 3.3, 0, 255));
      if (volts>3.22) analogWrite(13, 255);
      else analogWrite(13, 0);
    */
  
  
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
  
    // Call the current pattern function once, updating the 'leds' array
    gPatterns[gCurrentPatternNumber]();
  
    //Fire2012WithPalette(); // run simulation frame, using palette colors
  
    FastLED.show(); // display this frame
    //FastLED.delay(1000 / FRAMES_PER_SECOND);
    FastLED.delay(1000 / FPS);
    if (rotation) 
    {
      EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
    }

    
  }
  
  
  #define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))
  
  void nextPattern()
  {
    // add one to the current pattern number, and wrap around at the end
    gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
  }
  

