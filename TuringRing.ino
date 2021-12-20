// Libraries:
#include <RotaryEncoder.h>  // https://raw.githubusercontent.com/mathertel/RotaryEncoder/master/src/RotaryEncoder.h
                            // http://www.mathertel.de/Arduino/RotaryEncoderLibrary.aspx

#include <Adafruit_NeoPixel.h>  // https://raw.githubusercontent.com/adafruit/Adafruit_NeoPixel/master/Adafruit_NeoPixel.h

#include "ReadWrite.h"
#include "RTC.h"
#include "BTN.h"
#include "Pins.h"
#include "Wheel.h"
#include "PixelRing.h"
#include "Tape.h"
#include "Machine.h"
#include "Examples.h"
#include "Editor.h"
#include "Config.h"
#include "Programs.h"

// Turing Ring
// A Turing Machine with a circular tape (NeoPixel LED ring)
// https://en.wikipedia.org/wiki/Turing_machine
// Inspired by the work of Zafar Iqbal (https://zaf.io)
// Mark Wilson 2021
 
// *NOTE to self*: My Nano has "Old bootloader"

// ***** See readme.txt for details about how this works. *****

void Photogenic()
{
  // non-blinking for photos
  pixelRing.Clear();
  pixelRing.SetPixel(0, pixelRing.Colour(ColourPalette::Blue));
  pixelRing.Update();
  while(true)
    ;
}

void Splash()
{
  // Colourful startup sequence
  // r, g, b
  pixelRing.Clear(pixelRing.Colour(ColourPalette::Red));
  pixelRing.Update();
  delay(500);
  pixelRing.Clear(pixelRing.Colour(ColourPalette::Green));
  pixelRing.Update();
  delay(500);
  pixelRing.Clear(pixelRing.Colour(ColourPalette::Blue));
  pixelRing.Update();
  delay(500);
  // spin a colour wheel
  uint16_t dHue = 0xFFFF/PixelRing::Count;
  for (int rot = 0; rot <= PixelRing::Count; rot++)
  {
    for (int idx = 0; idx < PixelRing::Count; idx++)
    {
      int pixel = idx + rot;
      while (pixel >= PixelRing::Count)
        pixel -= PixelRing::Count;
      pixelRing.SetPixel(pixel, Adafruit_NeoPixel::ColorHSV(idx*dHue));
    }
    pixelRing.Update();
    delay(rot?50:1000);
  }
  delay(1000);
}

void setup()
{   
  Serial.begin(38400);
  Serial.println("Turing-ring");
  pixelRing.Init();
  btn.Init(PIN_ENCODER_BTN);
  wheel.Init();
  if (btn.IsDown())
    Photogenic();
  else
    Splash();
  rtc.setup();
  tape.Init();
  editor.Init();
  machine.Init();
  config.Init();
  config.Load();
}

void loop()
{
  editor.Edit();
}
