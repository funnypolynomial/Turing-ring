#pragma once

class Editor
{
  public:
    void Init();
    void Edit();
    
    void SetTapeCursor(int idx);
  private:
    const ColourPalette MenuContextColour = ColourPalette::White;
    const ColourPalette MenuAllowedColour = ColourPalette::Green;
    const int LongPressDurationMS = 1000; // 1sec
    
    void EditTape();
    void EditMachine();
    void EditState(Labels state);
    void EditInstruction(Labels state, Labels symbol);
    void EditMenu();
    
    Labels Pick(bool* longPress = NULL);
    Labels PickSymbol(Labels defaultLabel, uint32_t context, uint32_t allowed);
    Labels PickN(int Count, uint32_t context, uint32_t allowed, uint32_t colour = 0UL);
    Labels PickState(Labels defaultLabel, uint32_t context, uint32_t allowed);
    Labels PickInstruction(Labels state, uint32_t context, uint32_t allowed);

    void ShowSymbols();
    void ShowSymbol(Labels sym);
    void ShowMenu(uint32_t context, uint32_t allowed);
    void ShowAllowed(uint32_t allowed);
    void ShowInstruction(Labels state, Labels sym);
    void ShowInstruction(Labels sym, Machine::Instruction& instruction);
    void SetCursor(Labels label);
    bool Default(uint32_t context, uint32_t allowed, Labels pick);
    void SetBrightness();
    void SetSpeed();

    Labels _mode = Labels::Tape;
    int _tapeCursor = 0;
};

extern Editor editor;
