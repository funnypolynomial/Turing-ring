#include <Arduino.h>
#include "PixelRing.h"
#include "Config.h"
#include "ReadWrite.h"
#include "BTN.h"
#include "Wheel.h"
#include "Tape.h"
#include "Machine.h"

Machine machine;

void Machine::Init()
{
  Clear();
}

int Machine::Run()
{
  // run the machine!
  tape.Push();
  Labels state = Labels::A;
  int head = 0;
  pixelRing.SetCursor(-1);
  bool halt = false;
  bool quit = false;
  while (!halt && !quit)
  {
    tape.Show();
    pixelRing.Update();
    // read
    Labels symbol = tape.Get(head);
    Machine::Instruction instruction;
    int prevHead = head;
    Labels prevState = state;
    GetInstruction(state, symbol, instruction);
    // write
    tape.Set(head, instruction._writeSymbol);
    // move
    head += instruction._moveHead;
    if (head < 0)
      head += PixelRing::Count;
    if (head >= PixelRing::Count)
      head -= PixelRing::Count;
    // new state
    state = instruction._newState;
    if (state == Labels::Halt)
      halt = true;
    else if (config._AmimateState)
    {
        // show the (prev) state while waiting
        pixelRing.SetPixel(prevHead, tape.SymbolColour(prevState));
        pixelRing.Update();
    }
      
    int delayCtr = config._MachineStepDelayMS;
    if (delayCtr == 0) delayCtr = 1;
    // deal with the speed factor, delay between steps, check the wheel for speed changes and the button to stop
    while (delayCtr--)
    {
      wheel.Update();
      int rotation = wheel.GetRotation();
      if (btn.CheckButtonPress())
      {
        // quit
        quit = true;
        break;
      }
      else if (rotation)
      {
        // change speed
        config._MachineStepDelayMS -= rotation*Config::StepDelayIncrementMS;
        if (config._MachineStepDelayMS < Config::MinStepDelayMS)
          config._MachineStepDelayMS = Config::MinStepDelayMS;
        if (config._MachineStepDelayMS > Config::MaxStepDelayMS)
          config._MachineStepDelayMS = Config::MaxStepDelayMS;
        config.Save();
        break;
      }
      delay(1);
    }
  }
  pixelRing.Flash();
  pixelRing.Update();
  return quit?0:head;
}

void Machine::Clear(Labels label)
{
  if (IsAState(label, false))
  {
    // reset the entire state to "not-defined"
    ::memset(_machineTable + Index(label) - 1, 0, sizeof(PackedState));
  }
  else if (label == Labels::Machine)
  {
    // reset the entire machine to "not-defined"
    ::memset(_machineTable, 0, sizeof(_machineTable));
  }
}

bool Machine::GetInstruction(Labels state, Labels symRead, Instruction& instruction, PackedState* pState)
{
  // Read the Instruction to execute when in state having read symRead
  // From pState is supplied, otherwise from _machineTable
  // True if OK
  if (IsAState(state, false) &&  // NOT Halt
      IsASymbol(symRead))
  {
    if (!pState)
      pState = _machineTable + (Index(state) - 1);
    int symIdx = Index(symRead);
    if (((pState->_instructionDirectionBits >> 2*symIdx) & 3UL) == 0)
    {
      // not-defined
      // Be pedantic and init to Blank/None/Halt.  
      // Hitting an undefined state will Halt, but will change the tape first. Possibly annoying. 
      // The alternative would be to init to symRead/None/Halt. But I don't
      instruction._writeSymbol = Labels::Blank;
      instruction._moveHead = 0;
      instruction._newState = Labels::Halt;
    }
    else
    {
      instruction._writeSymbol = Label(pState->_instructionWriteAndStates[symIdx] >> 4);
      instruction._moveHead = ((pState->_instructionDirectionBits >> 2*symIdx) & 3UL) - 1;
      if (instruction._moveHead == 2)
        instruction._moveHead = -1;
      instruction._newState = Label(pState->_instructionWriteAndStates[symIdx] & 0x0F);
    }
    return true;
  }
  return false;
}

bool Machine::PutInstruction(Labels state, Labels symRead, Instruction& instruction, PackedState* pState)
{
  // Write the Instruction to execute when in state having read symRead
  // To pState is supplied, otherwise to _machineTable
  // True if OK
  if (IsAState(state, false) && // NOT Halt
      IsASymbol(symRead))
  {
    bool writeState = !pState;
    if (writeState)
      pState = _machineTable + (Index(state) - 1);
    int symIdx = Index(symRead);
    byte writeAndStates = Index(instruction._writeSymbol) << 4;
    //  1/2/3 = N/R/L = 0/+1/-1
    uint32_t bits = (instruction._moveHead == -1)?3UL:(uint32_t)(instruction._moveHead + 1);
    pState->_instructionDirectionBits &= ~(3UL << 2*symIdx);
    pState->_instructionDirectionBits |=  (bits << 2*symIdx);
    writeAndStates |= Index(instruction._newState);
    pState->_instructionWriteAndStates[symIdx] = writeAndStates;
    if (writeState)
      _machineTable[Index(state) - 1] = *pState;
    return true;
  }
  return false;
}

void Machine::DeSerialise(Reader& reader)
{
  // read the machine and tape
  Clear();
  while (IsAState(StateLabel(reader.Read()), false))
    DeSerialiseState(reader);
  tape.DeSerialise(reader);
}


void Machine::Serialise(Writer& writer)
{
  // serialize the machine (and tape)
  for (int state = 0; state < NumberOfStates; state++)
  {
    bool empty = true;
    bool first = true;
    for (int symbol = 0; symbol <= Index(Labels::LastLetter); symbol++)
    {
      Labels sym = Label(symbol);
      Instruction instruction;
      if (GetInstruction(Label(state + 1), sym, instruction))
      {
        if (instruction._writeSymbol == Labels::Blank && 
            instruction._moveHead    == 0             && 
            instruction._newState    == Labels::Halt)
          ; // don't write default/not-defined instruction
        else
        {
          if (empty)
          {
            writer.Write('A' + state);
            writer.Write('=');
          }
          if (!first)
            writer.Write(',');
          first = empty = false;
          writer.Write(tape.SymbolLetter(sym));
          writer.Write(':');
          writer.Write(tape.SymbolLetter(instruction._writeSymbol));
          writer.Write(DirectionLetter(instruction._moveHead));
          writer.Write(StateLetter(instruction._newState));
        }
      }
    }
    if (!empty)
      writer.Write('\n');
  }
  tape.Serialise(writer);
}

bool Machine::DeSerialiseState(Reader& reader)
{
  // "A=a:bLC,b:cLD...\n"
  Labels state = Labels::Null;
  if (DeSerialiseState(reader, state, false) && DeSerialiseDelimeter(reader, '='))
  {
    while (reader.Read() != '\n')
    {
      Instruction instr;
      Labels readSymbol = Labels::Null;
      if (DeSerialiseSymbol(reader, readSymbol) &&
          DeSerialiseDelimeter(reader, ':') &&
          DeSerialiseSymbol(reader, instr._writeSymbol) &&
          DeSerialiseDirection(reader, instr._moveHead) &&
          DeSerialiseState(reader, instr._newState, true) && // can Halt
          DeSerialiseDelimeter(reader, ','))
      {
        PutInstruction(state, readSymbol, instr);
      }
    }
    reader.Next();
    return true;
  }
  return false;
}

byte* Machine::GetPackedData(int& bytes)
{
  // return a pointer to, and the size of, the machine table
  bytes = (int)sizeof(_machineTable);
  return (byte*)(&_machineTable);
}


bool Machine::IsAState(Labels state, bool allowHalt)
{
  // true if state is 'A'..<last state>.  If allowHalt, true if state is Halt too
  return (Labels::A <= state && state <= Labels::LastLetter) || (allowHalt && state == Labels::Halt);
}

bool Machine::IsASymbol(Labels sym)
{
  // true is sym is 'a'..<last symbol> or blank
  return (Labels::A <= sym && sym <= Labels::LastLetter) || sym == Labels::Blank;
}

Labels Machine::StateLabel(char ch)
{
  // returns the Label corresponding to the char, 'X'->Halt, 'A'->A etc
  // also allows '0'->Halt, '1'->A etc
  if (ch == 'X' || ch == '0')
    return Labels::Halt;
  else if (StateLetter(Labels::A) <= ch && ch <= StateLetter(Labels::LastLetter))
    return Label(ch - 'A' + 1);
  else if ('1' <= ch && ch <= '9')
    return Label(ch - '1' + 1);
  else
    return Labels::Null;
}

char Machine::StateLetter(Labels label)
{
  // returns the char corresponding to the label, Halt->'X', A->'A' etc
  if (label <= Labels::LastLetter && label != Labels::Halt)
    return 'A' + Index(label) - 1;
  else
    return 'X';
}

char Machine::DirectionLetter(char dir)
{
  // returns L/N/R for -1/0/+1, R means Clockwise
  if (dir < 0)
    return 'L';
  else if (dir > 0)
    return 'R';
  else
    return 'N';
}

bool Machine::DeSerialiseState(Reader& reader, Labels& state, bool allowHalt)
{
  // convert an upper-case letter (or digit) to a state label
  // if allowHalt, 'X' is Halt
  if (reader.Read() == '\n') return false;
  char ch = reader.Read();
  state = StateLabel(ch);
  if (IsAState(state, allowHalt))
  {
    reader.Next();
    return true;
  }
  return false;
}

const char* pColourMap = "BlaRedGreBluYelAquFucWhiOraMalPur";
void Machine::CheckSymbolColour(char& ch, Reader& reader)
{
  if ('A' <= ch && ch <= 'Z')
  {
    char colour[4];
    colour[0] = ch;
    reader.Next();
    colour[1] = reader.Read();
    reader.Next();
    colour[2] = reader.Read();
    colour[3] = 0;
    const char* pMatch = strstr(pColourMap, colour);
    if (pMatch)
      ch = (pMatch == pColourMap)?'x':('a' + (pMatch - pColourMap)/3 - 1);
  }
}

bool Machine::DeSerialiseSymbol(Reader& reader, Labels& symbol)
{
  // convert a lower-case letter to a symbol label
  // ' ', or '_' are the same as 'x' (blank)
  // Also allows colour names, first three letters, first capital, see pColourMap
  if (reader.Read() == '\n') return false;
  char ch = reader.Read();
  CheckSymbolColour(ch, reader);
  reader.Next();
  symbol = tape.SymbolLabel(ch);
  return IsASymbol(symbol);
}

bool Machine::DeSerialiseDirection(Reader& reader, char& dirn)
{
  // read a direction char
  if (reader.Read() == '\n') return false;
  char ch = reader.Read();
  reader.Next();
  dirn = 99;
  if (ch == '<' || ch == '-' || ch == 'L')
    dirn = -1;
  else if (ch == '>' || ch == '+' || ch == 'R')
    dirn = +1;
  else if (ch == '=' || ch == '|' || ch == 'N')
    dirn = 0;
  return dirn != 99;
}

bool Machine::DeSerialiseDelimeter(Reader& reader, char ch)
{
  // read optional delimeter
  if (reader.Read() == ch)
    reader.Next();
  return true;
}
