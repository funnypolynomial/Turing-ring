#pragma once

// Handles rotary encoder wheel
class Wheel
{
  public:
    void Init();
    void Update();
    int GetRotation();
  private:
    static void isr();
};

extern Wheel wheel;
