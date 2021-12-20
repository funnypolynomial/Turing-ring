#include <Arduino.h>
#include "RTC.h"
#include "BTN.h"
#include "Wheel.h"
#include "PixelRing.h"
#include "Programs.h"

class Clock
{
  public:
    static void Run();
    static void Set();

  private:
    static const ColourPalette hourColour = ColourPalette::Blue;
    static const ColourPalette minuteColour = ColourPalette::Green;
    static const ColourPalette chapterColour = ColourPalette::Orange;
    static const int SecondsBetweenMinuteMarks = 150; // 2.5 minutes
};

void Clock::Run()
{
  // run the clock until the button is pressed
  unsigned long updateMS = millis();
  int lastMin = -1;
  pixelRing.Clear();
  pixelRing.SetCursor(-1);
  pixelRing.Show();
  bool blinkOn = false;
  while (!btn.CheckButtonPress())
  {
    unsigned long nowMS = millis();
    if (lastMin < 0 || (nowMS - updateMS) >= 1000UL)
    {
      rtc.ReadTime(true);
      updateMS = nowMS;
      lastMin = rtc.m_Minute;
      int hourIdx = 2*(rtc.m_Hour24 % 12);
      int minuteIdx = (rtc.m_Minute*60 + rtc.m_Second)/SecondsBetweenMinuteMarks;
      blinkOn = !blinkOn;
      pixelRing.Clear();

      // chapter ring
      for (int idx = 0; idx < PixelRing::Count; idx++)
        pixelRing.SetPixel(idx, pixelRing.Colour((idx % 2)?ColourPalette::Black:chapterColour));
      // hour hand, only 12 pixels
      pixelRing.SetPixel(hourIdx, pixelRing.Colour(hourColour));
      // minute hand, all 24 pixels
      if (blinkOn)
        pixelRing.SetPixel(minuteIdx,  pixelRing.Colour(minuteColour));
      else
      {
        // blank/background colour
        if (hourIdx == minuteIdx)
          pixelRing.SetPixel(minuteIdx, pixelRing.Colour(hourColour));
        else if (minuteIdx % 2)
          pixelRing.SetPixel(minuteIdx, pixelRing.Colour(ColourPalette::Black));
        else
          pixelRing.SetPixel(minuteIdx, pixelRing.Colour(chapterColour));
      }
      pixelRing.Show();
    }
  }
}

void Clock::Set()
{
  unsigned long updateMS = millis();
  unsigned long idleMS = millis();
  rtc.ReadTime(true);
  pixelRing.Clear();
  pixelRing.SetCursor(-1);
  pixelRing.Show();
  bool blinkOn = false;
  bool hour = true;
  bool updateDisplay = true;
  int hourIdx = 2*(rtc.m_Hour24 % 12);
  int minuteIdx = (rtc.m_Minute*60 + rtc.m_Second)/SecondsBetweenMinuteMarks;
  ColourPalette altChapterColour = ColourPalette::Aqua;
  while (true)
  {
    if (btn.CheckButtonPress())
    {
      idleMS = millis();
      if (hour)
      {
        hour = false;
        blinkOn = updateDisplay = true;
        altChapterColour = ColourPalette::Malachite;
      }
      else
      {
        break;
      }
    }
    wheel.Update();
    int rotation = wheel.GetRotation();
    if (rotation)
    {
      idleMS = millis();
      if (hour)
        hourIdx += 2*rotation;
      else
        minuteIdx += rotation;
      if (hourIdx < 0)
        hourIdx += PixelRing::Count;
      if (hourIdx >= PixelRing::Count)
        hourIdx -= PixelRing::Count;
      if (minuteIdx < 0)
        minuteIdx += PixelRing::Count;
      if (minuteIdx >= PixelRing::Count)
        minuteIdx -= PixelRing::Count;
      blinkOn = updateDisplay = true;
    }
    unsigned long nowMS = millis();
    if ((nowMS - idleMS) >= 60000UL)
    {
      // sitting idle
      return;
    }
    if (updateDisplay || (nowMS - updateMS) >= 500UL)
    {
      updateMS = nowMS;
      pixelRing.Clear();
      // chapter ring
      for (int idx = 0; idx < PixelRing::Count; idx++)
        pixelRing.SetPixel(idx, pixelRing.Colour((idx % 2)?ColourPalette::Black:altChapterColour));
      int idx = hour?hourIdx:minuteIdx;
      if (blinkOn)
        pixelRing.SetPixel(idx, pixelRing.Colour(hour?hourColour:minuteColour));
      else
      {
        // blank/background colour
        if (idx % 2)
          pixelRing.SetPixel(idx, pixelRing.Colour(ColourPalette::Black));
        else
          pixelRing.SetPixel(idx, pixelRing.Colour(altChapterColour));
      }
      updateDisplay = false;
      pixelRing.Show();
      blinkOn = !blinkOn;
    }
  }
  // SET
  rtc.ReadTime(true);
  rtc.m_Hour24 = hourIdx / 2;
  if (rtc.m_Hour24 == 0)
    rtc.m_Hour24 = 12;
  int seconds = minuteIdx*SecondsBetweenMinuteMarks;
  rtc.m_Minute = seconds / 60;
  rtc.m_Second = seconds % 60;
  rtc.WriteTime();
}

void Program(Labels label)
{
  if (label == Labels::Menu_ClockRun)
  {
    Clock::Run();
  }
  else if (label == Labels::Menu_ClockSet)
  {
    Clock::Set();
  }
}
