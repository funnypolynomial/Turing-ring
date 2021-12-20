#pragma once
#include <avr/pgmspace.h>

//////////////////////////////////////////////////
// read a sequence of chars from PROGMEM *or* Serial
struct Reader
{
  virtual char Read() = 0;
  virtual void Next() { _idx++; };
  int _idx = 0;
};

struct PROGMEMReader: Reader
{
  char Read() override {return pgm_read_byte_near(_basePtr + _idx);}
  const char* _basePtr = NULL;
};

struct SerialReader: Reader
{
  char Read() override
  { 
    if (_char == -1) 
      Next();
    return _char;
  }
  
  void Next() override
  {  
    do
      _char = Serial.read();
    while (_char == -1);
  }
  int _char = -1;
};

//////////////////////////////////////
// write a sequence of chars to Serial
struct Writer
{
  virtual void Write(char ch) = 0;
  int _idx = 0;
};

struct SerialWriter: Writer
{
  void Write(char ch) override { Serial.write(ch); }
};
