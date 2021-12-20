#pragma once

class Tape
{
  public:
    void Init();
    void Clear(Labels symbol = Labels::Blank);
    void Set(int idx, Labels symbol);
    Labels Get(int idx);
    void Show();
    uint32_t SymbolColour(Labels label);
    char SymbolLetter(Labels label);
    Labels SymbolLabel(char ch);

    void Push();
    void Pop();
    
    void DeSerialise(Reader& reader);
    void Serialise(Writer& writer);
  private:
    Labels _symbols[PixelRing::Count];
    Labels _copy[PixelRing::Count];
};

extern Tape tape;
