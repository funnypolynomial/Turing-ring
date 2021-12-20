#include <Arduino.h>
#include <EEPROM.h>
#include "ReadWrite.h"
#include "PixelRing.h"
#include "Tape.h"
#include "Machine.h"
#include "Config.h"

Config config;

// First two bytes identify a valid config in EEPROM
const char* Sig = "tm";
// First 8 bytes reserved for settings, the rest is for saved Machines+Tape
const int OffsetToSlot0 = 8;

void Config::Init()
{
  // init to defaults, if no stored config, create it
  int idx = 0;
  int packedbytes;
  machine.GetPackedData(packedbytes);
  _SlotSize = packedbytes + PixelRing::Count;
  _NumberOfMachineSlotsInEEPROM = (EEPROM.length() - OffsetToSlot0)/_SlotSize;
  if (!(EEPROM[idx++] == Sig[0] && EEPROM[idx++] == Sig[1]))
  {
    // config is not there, init/create it
    _MachineStepDelayMS = (MaxStepDelayMS + MinStepDelayMS)/2;
    _PixelBrightness = StepBrightness;
    _AmimateState = true;
    // zero the machines in EEPROM
    idx = OffsetToSlot0;
    for (int mch = 0; mch < _NumberOfMachineSlotsInEEPROM; mch++)
    {
      for (int packed = 0; packed < packedbytes; packed++)
        EEPROM[idx++] = 0;
      for (int tape = 0; tape < PixelRing::Count; tape++)
        EEPROM[idx++] = 0;
    }
    Save();
  }
}

void Config::Load()
{
  // load the config setting(s)
  int idx = 0;
  if (EEPROM[idx++] == Sig[0] && EEPROM[idx++] == Sig[1])
  {
    // config is there
    _MachineStepDelayMS = EEPROM[idx++];
    _PixelBrightness = EEPROM[idx++];
    _AmimateState = EEPROM[idx++];
  }
}

void Config::Save()
{
  // load the config setting(s)
  int idx = 0;
  EEPROM[idx++] = Sig[0];
  EEPROM[idx++] = Sig[1];
  EEPROM[idx++] = _MachineStepDelayMS;
  EEPROM[idx++] = _PixelBrightness;
  EEPROM[idx++] = _AmimateState?1:0;
}

void Config::LoadSlot(int slot)
{
  // get the machine and tape from slot in EEPROM
  int packedBytes = 0;
  byte* pMch = machine.GetPackedData(packedBytes);
  int idx = OffsetToSlot0 + _SlotSize*slot;
  if (0 <= slot && slot < _NumberOfMachineSlotsInEEPROM && pMch && packedBytes)
  {
    for (int packed = 0; packed < packedBytes; packed++)
      *(pMch++) = EEPROM[idx++];
    for (int t = 0; t < PixelRing::Count; t++)
    {
      byte b = EEPROM[idx++];
      tape.Set(t, Label(b));
    }
  }
}

void Config::SaveSlot(int slot)
{
  // put the machine and tape to slot in EEPROM
  int packedBytes = 0;
  byte* pMch = machine.GetPackedData(packedBytes);
  int idx = OffsetToSlot0 + _SlotSize*slot;
  if (0 <= slot && slot < _NumberOfMachineSlotsInEEPROM && pMch && packedBytes)
  {
    for (int packed = 0; packed < packedBytes; packed++)
      EEPROM[idx++] = *(pMch++);
    for (int t = 0; t < PixelRing::Count; t++)
      EEPROM[idx++] = static_cast<byte>(tape.Get(t));
  }
}
