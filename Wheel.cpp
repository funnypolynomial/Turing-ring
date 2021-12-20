#include <Arduino.h>
#include <RotaryEncoder.h>
#include "Pins.h"
#include "Wheel.h"

// Encoder is https://www.jaycar.co.nz/rotary-encoder-with-pushbutton/p/SR1230

Wheel wheel;
RotaryEncoder encoder(PIN_ENCODER_A, PIN_ENCODER_B, RotaryEncoder::LatchMode::TWO03);

void Wheel::Init()
{
  attachInterrupt(PIN_ENCODER_A, isr, CHANGE);
  attachInterrupt(PIN_ENCODER_B, isr, CHANGE);
}

void Wheel::Update()
{
  encoder.tick();  
}

int Wheel::GetRotation() // -1, 0 or +1
{
  RotaryEncoder::Direction dir = encoder.getDirection();
  if (dir == RotaryEncoder::Direction::CLOCKWISE)
    return +1;
  else if (dir == RotaryEncoder::Direction::COUNTERCLOCKWISE)
    return -1;
  return 0;
}

// This interrupt routine will be called on any change of one of the input signals
void Wheel::isr()
{
  encoder.tick();
}
