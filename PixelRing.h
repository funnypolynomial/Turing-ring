#pragma once

// colours/states/symbols
enum class ColourPalette
{
  // NOTE: these enum values match up with the State/Symbol enum values in Labels
  Black,
  Red,  
  Green,  
  Blue,   
  Yellow, 
  Aqua,   
  Fuchsia,
  White,

  Orange, 
  Malachite, 
  Purple,
  
  Grey
};

// Labels/names of the pixels. Clockwise
enum class Labels    
{
  // Blank Symbol (black), Halt State, just to the LEFT of 12 O'Clock
  Blank, Halt=Blank,
  
  // The State/Symbol list runs from 12 O'Clock:
  A,                    Menu_LoadPROGMEM = A,   Menu_First        = A,
  B,                    Menu_LoadEEPROM  = B,
  C,                    Menu_SaveEEPROM  = C,
  D,                    Menu_ReadSerial  = D,
  E,                    Menu_WriteSerial = E,
  F,                    Menu_ClockRun    = F,
  G,                    Menu_ClockSet    = G,
  H,                    Menu_Brightness  = H,
  I,                    Menu_Speed       = I,
  J,                    Menu_Off         = J,   Menu_Last        = J,
  LastLetter = J,

  // The Instruction area is centred around 6 O'Clock [Write] [Move:Left|None|Right] [New State]

  NewState,
  MoveRight,
  MoveNone,    // 6 O'Clock
  MoveLeft,
  WriteSymbol,

  // Commands
  Menu,
  Clear,
  Instruction,
  Symbol,
  State,
  Machine,
  Tape,
  Run,

  Null
};

#define LABEL_BIT(_label) (uint32_t)(1UL << ((int)_label))

// Updates LEDs, controls brightness, maintains cursor etc
class PixelRing
{
  public:
    static const int Count = 24;
    void Init();
    void Update();

    void Flash(uint32_t colour = 0, int durationMS = 100);
    void Clear(uint32_t colour = 0);
    void SetPixel(int idx, uint32_t colour);
    void SetCursor(int idx);
    bool MoveCursor(int dirn);
    int GetCursor();
    void Brightness(uint8_t brite);

    void Show();

    uint32_t Colour(ColourPalette palIndex);
    int LabelToDisplayPixel(Labels label);
    Labels PhysicalPixelToLabel(int n);
    Labels LabelAtCursor();
    char* LabelStr(Labels label);
    
    // Display index (Idx) vs Physical Pixel (N)
    int ToPhysicalPixel(int Idx);
    int FromPhysicalPixel(int N);
  private:
    
    const int TopPhysicalPixelN = 15;  // the one at 12 O'Clock
    uint32_t _pixels[Count];  // [0] is pixel #0
    bool _changed = true;   // update needed
    bool _cursorOn = false; // true if there's a blinking cursor
    int  _cursorN = -1;     // physical number of cursor pixel on ring
    byte _blinkCtr = 0;  // colour blink mod = 0, 1, black blink mod = 0
    uint32_t _blinkTimerMS = 0;
    const uint32_t blinkPeriodMS = 250;  // quarter-second blink update
    
};

extern int Index(Labels label);
extern Labels Label(int index);

extern PixelRing pixelRing;
