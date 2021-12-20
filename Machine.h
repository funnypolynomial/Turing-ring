#pragma once

class Machine
{
  public:
    static const int NumberOfSymbols = (int)(Labels::LastLetter) + 1; // Blank is included
    static const int NumberOfStates = (int)(Labels::LastLetter);      // Halt is not uncluded
    
    struct Instruction  // a more convenient representation than packed, for editing etc
    {
      Labels _writeSymbol = Labels::Blank;
      char _moveHead = 0; // -1 = Left
      Labels _newState = Labels::Halt;
    };
    
    struct PackedState  // *PACKED* representation has max of 16 states or symbols, but we only use 0..10
    {
      PackedState() { memset(_instructionWriteAndStates, 0, sizeof(_instructionWriteAndStates));}
      // Note that the default-constructed state has all instructions set to "not-defined" (move=0).
      // When they're loaded as an Instruction it is updated to [write blank, don't move, Halt]
      uint32_t _instructionDirectionBits = 0UL;             // 2 bits per instruction, 1/2/3 = N/R/L = 0/+1/-1, indexed by symbol read, 0=Not defined/Halt
      byte     _instructionWriteAndStates[NumberOfSymbols]; // packed as 0bssssSSSS, s=symbol to write, S=State to change to, indexed by symbol read
    };
    
    void Init();
    int Run();  // returns the head position at Halt, or 0 if quit
    void Clear(Labels label = Labels::Machine); // Machine or a State

    bool GetInstruction(Labels state, Labels symRead, Instruction& instruction, PackedState* pState = NULL);
    bool PutInstruction(Labels state, Labels symRead, Instruction& instruction, PackedState* pState = NULL);

    void DeSerialise(Reader& reader);
    void Serialise(Writer& writer);

    byte* GetPackedData(int& bytes);

    static bool IsAState(Labels state, bool allowHalt);
    static bool IsASymbol(Labels sym);
    
    static void CheckSymbolColour(char& ch, Reader& reader);
  private:
    Labels StateLabel(char ch);
    char StateLetter(Labels label);
    char DirectionLetter(char dir);
    bool DeSerialiseState(Reader& reader);
    bool DeSerialiseState(Reader& reader, Labels& state, bool allowHalt);
    bool DeSerialiseSymbol(Reader& reader, Labels& symbol);
    bool DeSerialiseDirection(Reader& reader, char& dirn);
    bool DeSerialiseDelimeter(Reader& reader, char ch);
    
    PackedState _machineTable[NumberOfStates];  // no Halt, 0th is A
};

extern Machine machine;
