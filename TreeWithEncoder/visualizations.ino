
  

  void Fire2012WithPalette()
  {  
    CRGB rootColor = CRGB(255,255,255);
    CRGB endColor = CRGB(255,255,255);
    rootColor.setHue(gHue);
    endColor.setHue(gHue+50);
    CRGBPalette16 gPal = CRGBPalette16( CRGB::Black, rootColor, endColor);
    // Array of temperature readings at each simulation cell
    static byte heat[NUM_LEDS];
  
    // Step 1.  Cool down every cell a little
    for ( int i = 0; i < NUM_LEDS; i++) {
      heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / NUM_LEDS) + 2));
    }
  
    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for ( int k = NUM_LEDS - 1; k >= 2; k--) {
      heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
    }
  
    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    //    if( random8() < SPARKING ) {
    //      int y = random8(2);
    //      heat[y] = qadd8( heat[y], random8(lowHeat,highHeat) );
    //    }
  
    // Step 3.  border based flickr
  
    int y = random8(4);
    heat[y] = qadd8( heat[y], highHeat );
  
  
    // Step 4.  Map from heat cells to LED colors
    for ( int j = 0; j < NUM_LEDS; j++) {
      // Scale the heat value from 0-255 down to 0-240
      // for best results with color palettes.
      byte colorindex = scale8( heat[j], 240);
      CRGB color = ColorFromPalette( gPal, colorindex);
      int pixelnumber;
      if ( gReverseDirection ) {
        pixelnumber = (NUM_LEDS - 1) - j;
      } else {
        pixelnumber = j;
      }
      leds[pixelnumber] = color;
    }
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
    if ( random8() < chanceOfGlitter) {
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
    int pos = beatsin16( 13, 0, NUM_LEDS - 1 );
    leds[pos] += CHSV( gHue, 255, 192);
  }
  
  void bpm()
  {
    // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
    uint8_t BeatsPerMinute = 62;
    CRGBPalette16 palette = PartyColors_p;
    uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
    for ( int i = 0; i < NUM_LEDS; i++) { //9948
      leds[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
    }
  }
  
  void juggle() {
    // eight colored dots, weaving in and out of sync with each other
    fadeToBlackBy( leds, NUM_LEDS, 20);
    byte dothue = 0;
    for ( int i = 0; i < 8; i++) {
      leds[beatsin16( i + 7, 0, NUM_LEDS - 1 )] |= CHSV(dothue, 200, 255);
      dothue += 32;
    }
  }
  
