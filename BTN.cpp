#include <Arduino.h>
#include "BTN.h"

// buttons

BTN btn;

#define HOLD_TIME_MS 50

void BTN::Init(int Pin, int ClosedState)
{
  m_iPin = Pin;
  m_iClosedState = ClosedState;
  m_iPrevReading = !m_iClosedState;
  m_iPrevState = m_iClosedState;
  m_iTransitionTimeMS = millis();
  pinMode(m_iPin, INPUT_PULLUP);
}

bool BTN::CheckButtonPress()
{
  // debounced button, true if button pressed
  int ThisReading = digitalRead(m_iPin);
  if (ThisReading != m_iPrevReading)
  {
    // state change, reset the timer
    m_iPrevReading = ThisReading;
    m_iTransitionTimeMS = millis();
  }
  else if (ThisReading != m_iPrevState &&
           (millis() - m_iTransitionTimeMS) >= HOLD_TIME_MS)
  {
    // a state other than the last one and held for long enough
    m_iPrevState = ThisReading;
    return (ThisReading == m_iClosedState);
  }
  return false;
}

bool BTN::IsDown()
{
  // non-debounced, instantaneous reading
  return digitalRead(m_iPin) == m_iClosedState;
}

bool BTN::CheckButtonToggled()
{
  // debounced button, true if button state changed
  int ThisReading = digitalRead(m_iPin);
  if (ThisReading != m_iPrevReading)
  {
    // state change, reset the timer
    m_iPrevReading = ThisReading;
    m_iTransitionTimeMS = millis();
  }
  else if (ThisReading != m_iPrevState &&
           (millis() - m_iTransitionTimeMS) >= HOLD_TIME_MS)
  {
    // a state other than the last one and held for long enough
    m_iPrevState = ThisReading;
    return true;
  }
  return false;
}

void BTN::SetButtonToggled(bool toggled)
{
  // init the toggle state
  m_iPrevState = m_iPrevReading = toggled;
  m_iTransitionTimeMS = millis();
}
