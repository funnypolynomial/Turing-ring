#pragma once

class BTN
{
  public:
    void Init(int Pin, int ClosedState = LOW);  // pin pulled LOW when pressed
    bool CheckButtonPress();
    bool IsDown();
    bool CheckButtonToggled();  // true if state changed, check IsDown
    void SetButtonToggled(bool toggled);
    
  private:
    int m_iPin;
    int m_iPrevReading;
    int m_iPrevState;
    unsigned long m_iTransitionTimeMS;
    int m_iClosedState;
};

extern BTN btn;
