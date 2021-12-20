#include <Arduino.h>
#include "ReadWrite.h"
#include "PixelRing.h"
#include "Tape.h"
#include "Machine.h"

Tape tape;

void Tape::Init()
{
  Clear();
  ::memset(_copy, 0, sizeof(_copy));
}

void Tape::Clear(Labels symbol)
{
  // set entire tape to symbol
  ::memset(_symbols, (int)symbol, sizeof(_symbols));
}

void Tape::Set(int idx, Labels symbol)
{
  // set display cell to symbol, 0 is at 12 O'Clock
  _symbols[idx] = symbol;
}

Labels Tape::Get(int idx)
{
  // get symbols at display cell, 0 is at 12 O'Clock
  return _symbols[idx];
}


void Tape::Show()
{
  // send the tape symbols to the physical device
  for (int idx = 0; idx < PixelRing::Count; idx++)
    pixelRing.SetPixel(idx, SymbolColour(_symbols[idx]));
}

uint32_t Tape::SymbolColour(Labels label)
{
  // the enums match
  return pixelRing.Colour( (label <= Labels::LastLetter)?(static_cast<ColourPalette>(label)) : ColourPalette::Black );
}

char Tape::SymbolLetter(Labels label)
{
  // returns the char corresponding to the label, blank->'x', a->'x' etc
  if (label <= Labels::LastLetter && label != Labels::Blank)
    return 'a' + Index(label) - 1;
  else
    return 'x';
}

Labels Tape::SymbolLabel(char ch)
{
  // returns the Label corresponding to the char, 'x'->Blank, 'a'->A etc
  // ' ' and '_' are Blank too
  if (ch == 'x' || ch == ' ' || ch == '_')
    return Labels::Blank;
  else if (SymbolLetter(Labels::A) <= ch && ch <= SymbolLetter(Labels::LastLetter))
    return Label(ch - 'a' + 1);
  else
    return Labels::Null;
}

void Tape::Push()
{
  // take a copy of the tape
  ::memcpy(_copy, _symbols, sizeof(_symbols));
}

void Tape::Pop()
{
  // restore the copy of the tape
  ::memcpy(_symbols, _copy, sizeof(_symbols));
}


void Tape::DeSerialise(Reader& reader)
{
  // read in the tape
  Clear();
  int idx = 0;
  do
  {
    char ch = reader.Read();
    if (ch == '\n')
    {
      break;
    }
    else
    {
      Machine::CheckSymbolColour(ch, reader);
      Labels sym = SymbolLabel(ch);
      if (sym != Labels::Null && idx < PixelRing::Count)
        _symbols[idx++] = sym;
    }
    reader.Next();
  } while (true);
}

void Tape::Serialise(Writer& writer)
{
  // write out the tape
  for (int idx = 0; idx < PixelRing::Count; idx++)
    writer.Write(SymbolLetter(_symbols[idx]));
  writer.Write('\n');
}
