#pragma once

// keep settings and saved Machines in EEPROM
class Config
{
  public:
    void Init();
    void Load();
    void Save();
    
    void LoadSlot(int slot);
    void SaveSlot(int slot);

    int _NumberOfMachineSlotsInEEPROM; // machine & tape

    static const int MaxStepDelayMS = 100;
    static const int MinStepDelayMS = 0;
    static const int StepDelayIncrementMS = 10;
    static const int MinBrightness = 5;
    static const int StepBrightness = 5;
    static const int MaxBrightness = StepBrightness*PixelRing::Count;

    // saved in config bytes:
    int _MachineStepDelayMS = (MaxStepDelayMS + MinStepDelayMS)/2;
    int _PixelBrightness = MinBrightness;
    bool _AmimateState = true;
    
  private:
    int _SlotSize;
};

extern Config config;
