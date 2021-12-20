#include <Arduino.h>
#include <avr/pgmspace.h>
#include "Pins.h"
#include "PixelRing.h"
#include "Config.h"
#include <Adafruit_NeoPixel.h>  // https://raw.githubusercontent.com/adafruit/Adafruit_NeoPixel/master/Adafruit_NeoPixel.h

// NeoPixel ring is https://www.jaycar.co.nz/duinotech-rgb-led-circular-board/p/XC4385
PixelRing pixelRing;

static const uint32_t _ColourPaletteRGB[]  // https://en.wikipedia.org/wiki/Web_colors#Basic_colors
{
  0x000000, // black
  0xFF0000, // red
  0x00FF00, // green
  0x0000FF, // blue
  0xFFFF00, // yellow
  0x00FFFF, // aqua
  0xFF00FF, // fuchsia
  0xFFFFFF, // white
  
  // These are not very distict as Symbols but function as additional States:
  0xFF4000, // orange
  0x00FF40, // malachite https://www.htmlcsscolor.com/hex/00FF40
  0x4000FF, // purple

  0x404040  // grey
};

Adafruit_NeoPixel neoPixels(PixelRing::Count, PIN_PIXEL_RING, NEO_GRB + NEO_KHZ800);

void PixelRing::Init()
{
  neoPixels.begin();
  Brightness(config._PixelBrightness);
  Clear();
}

void PixelRing::Update()
{
  // called regularly, updates pixels as needed, blinks cursor
  if (_changed)
  {
    _blinkTimerMS = millis();
    _blinkCtr = 0;
  }
  else if (_cursorOn)
  {
    uint32_t NowMS = millis();
    if ((NowMS - _blinkTimerMS) >= blinkPeriodMS)
    {
      _blinkTimerMS = NowMS;
      _blinkCtr++;
      _changed = true;
    }
  }
  if (_changed)
  {
    Show();
    _changed = false;
  }
}

void PixelRing::Flash(uint32_t colour, int durationMS)
{
  // briefly flash the ring all the same colour, acknowledgment etc
  for (int n = 0; n < Count; n++)
    neoPixels.setPixelColor(n, colour); 
  neoPixels.show();
  delay(durationMS);
}

void PixelRing::Clear(uint32_t colour)
{
  // set all the pixels to colour
  for (int n = 0; n < Count; n++)
    _pixels[n] = colour;
  _changed = true;
}

void PixelRing::SetPixel(int idx, uint32_t colour)
{
  // sets the Display pixel (0 is 12 O'Clock) to the colour, a packed 0xRRGGBB
  _pixels[ToPhysicalPixel(idx)] = colour;
  _changed = true;
}

void PixelRing::SetCursor(int idx)
{
  // sets the cursor at the idx'th Display pixel
  // -1 turns the cursor off
  _cursorOn = (idx >= 0);
  _cursorN = _cursorOn?ToPhysicalPixel(idx):-1;
  _changed = true;
}

bool PixelRing::MoveCursor(int dirn)
{
  // if it's enabled, moves the cursor one pixel in the given direction, +1 is clockwise
  if (_cursorOn && dirn)
  {
    _cursorN += dirn;
    if (_cursorN >= Count)
      _cursorN -= Count;
    else if (_cursorN < 0)
      _cursorN += Count;
    _changed = true;
    return true;
  }
  return false;
}

int PixelRing::GetCursor()
{
  // returns the Display pixel of the cursor, or -1
  return _cursorOn?FromPhysicalPixel(_cursorN):-1;
}

void PixelRing::Brightness(uint8_t brite)
{
  neoPixels.setBrightness(brite);
}

void PixelRing::Show()
{
  // send the pixel data to the ring
  for (int n = 0; n < Count; n++)
    if (n == _cursorN && (_blinkCtr % 4) < 1)
      // blink a colour to black/colour 25%/75%, blink black to grey/black 25%/75%
      neoPixels.setPixelColor(n, _ColourPaletteRGB[(int)(_pixels[n]?ColourPalette::Black:ColourPalette::Grey)]); 
    else
      neoPixels.setPixelColor(n, _pixels[n]);   
  neoPixels.show();
}

uint32_t PixelRing::Colour(ColourPalette palIndex)
{
  // look up the colour
  return _ColourPaletteRGB[static_cast<int>(palIndex)];
}

int PixelRing::LabelToDisplayPixel(Labels label)
{
  // returns the display index of the pixel corresponding to the label
  // 'A' goes at 12 O'Clock
  int idx = Index(label) - 1;
  if (idx < 0)
    idx += Count;
  return idx;
}

Labels PixelRing::PhysicalPixelToLabel(int n)
{
  // Returns the label corresponding to the physical pixel index
  // 'A' goes at 12 O'Clock
  int idx = FromPhysicalPixel(n) + 1;
  if (idx >= Count)
    idx -= Count;
  return Label(idx);
}

Labels PixelRing::LabelAtCursor()
{
  // label at cursor location
  return PhysicalPixelToLabel(_cursorN);
}

int PixelRing::ToPhysicalPixel(int Idx)
{
  // given a Display index returns the corresponding physical pixel number
  int n = Idx + TopPhysicalPixelN;
  if (n >= Count)
    n -= Count;
  return n;
}
 
int PixelRing::FromPhysicalPixel(int N)
{
  // given a physical pixel number returns corresponding Display index
  int idx = N - TopPhysicalPixelN;
  if (idx < 0)
    idx += Count;
  return idx;
}

// label lists are simple PROGMEM strings, null delimited
static const char pLabelStrings[] PROGMEM =
{
  "Blank/Halt\0"
  "A\0"
  "B\0"
  "C\0"
  "D\0"
  "E\0"
  "F\0"
  "G\0"
  "H\0"
  "I\0"
  "J\0"
  "K\0"
  "NewState\0"
  "MoveRight\0"
  "MoveNone\0"
  "MoveLeft\0"
  "WriteSymbol\0"
  "Clear\0"
  "Tape\0"
  "Instruction\0"
  "State\0"
  "Machine\0"
  "Symbol\0"
  "Run\0"
  "Menu\0"
};

static char labelBuffer[16];
char* PixelRing::LabelStr(Labels label)
{
  // copy the text for the label'th item in pLabelStrings into buffer and return it
  int idx = Index(label);
  const char* pLabel = pLabelStrings;
  while (idx)
    if (pgm_read_byte_near(pLabel++) == 0)
      idx--;
  strcpy_P(labelBuffer, pLabel);
  return labelBuffer;
}

int Index(Labels label) { return static_cast<int>(label); }
Labels Label(int index) { return static_cast<Labels>(index); }
