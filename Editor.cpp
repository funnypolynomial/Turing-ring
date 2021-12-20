#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "ReadWrite.h"
#include "BTN.h"
#include "Wheel.h"
#include "PixelRing.h"
#include "Config.h"
#include "Tape.h"
#include "Machine.h"
#include "Examples.h"
#include "Programs.h"
#include "Editor.h"

// The main editor loop does EVERYTHING

Editor editor;

void Editor::Init()
{
}

void Editor::Edit()
{
  // runs the edit loop which includes running the machine
  _mode = Labels::Tape;
  while (true)
  {
    // wait for a click:
    if (_mode == Labels::Tape)
    {
      EditTape();
    }
    else if (_mode == Labels::Machine)
    {
      EditMachine();
    }
    else if (_mode == Labels::Menu)
    {
      EditMenu();
    }
    else if (_mode == Labels::Run)
    {
      SetTapeCursor(machine.Run());
      break;
    }
  }
}

void Editor::SetTapeCursor(int idx)
{
  // Set location of cursor on tape
  _tapeCursor = idx;
}

void Editor::EditTape()
{
  // the default editing mode, show the tape
  // selecting a pixel changes to a menu to set it's symbol and offer other actions
  tape.Show();
  pixelRing.SetCursor(_tapeCursor);
  // show that they're symbols and what else they can choose, eg Run
  uint32_t context = LABEL_BIT(Labels::Tape) + LABEL_BIT(Labels::Symbol);
  uint32_t allowed = LABEL_BIT(Labels::Run) + LABEL_BIT(Labels::Machine) + LABEL_BIT(Labels::Clear) + LABEL_BIT(Labels::Menu);
  bool longPress = true;
  while (true)
  {
    Labels pick = Pick(&longPress);
    if (longPress)
    {
      if (pick == Labels::Tape)
      {
        tape.Pop();
        _tapeCursor = 0;
        break;
      }
      else if (pick == Labels::Clear)
      {
        tape.Clear();
        _tapeCursor = 0;
        break;
      }
      else
      {
        Default(context, allowed, pick);
        break;
      }
    }
    int cell = pixelRing.LabelToDisplayPixel(pick); // clicked on a cell to edit
    // pick a symbol (or a menu):
    pick = PickSymbol(tape.Get(cell), context, allowed);
    if (Machine::IsASymbol(pick))
    {
      // symbol chosen, update the tape
      tape.Set(cell, pick); 
      tape.Show();
      // advance the cursor
      pixelRing.SetCursor(cell);
      pixelRing.MoveCursor(+1);
    }
    else if (pick == Labels::Clear)
    {
      tape.Clear();
      break;
    }
    else
    {
      Default(context, allowed, pick);
      break;
    }
  }
}

void Editor::EditMachine()
{
  // edit the machine
  // offer a list of states, picking one edits it
  // show that they're symbols and what else they can choose, eg Run
  uint32_t context = LABEL_BIT(Labels::Machine) + LABEL_BIT(Labels::State);
  uint32_t allowed = LABEL_BIT(Labels::Run) + LABEL_BIT(Labels::Tape) + LABEL_BIT(Labels::Clear) + LABEL_BIT(Labels::Menu);
  while (_mode == Labels::Machine)
  {
    // pick a state (or a menu):
    Labels pick = PickState(Labels::A, context, allowed);
    if (Machine::IsAState(pick, false))  // NOT Halt
    {
      // state chosen, edit it
      EditState(pick);
    }
    else if (pick == Labels::Clear)
    {
      machine.Clear();
    }
    else
    {
      if (!Default(context, allowed, pick))
      {
        _mode = Labels::Tape;
      }
      break;
    }
  }
}

void Editor::EditState(Labels state)
{
  // edit the specific state; the table of instructions
  // show that they're symbols and what else they can choose, eg Run
  uint32_t context = LABEL_BIT(Labels::Machine) + LABEL_BIT(Labels::Symbol);
  uint32_t allowed = LABEL_BIT(Labels::Run) + LABEL_BIT(Labels::Tape) + LABEL_BIT(Labels::Clear) + LABEL_BIT(Labels::Menu) + LABEL_BIT(Labels::State);
  while (_mode == Labels::Machine)
  {
    // pick a symbol (i.e an instruction) (or a menu):
    Labels pick = PickInstruction(state, context, allowed);
    if (Machine::IsASymbol(pick))
    {
      EditInstruction(state, pick);
    }
    else if (pick == Labels::State)
    {
      // go up
      break;
    }
    else if (pick == Labels::Clear)
    {
      machine.Clear(state);
    }
    else
    {
      Default(context, allowed, pick);
      break;
    }
  }
}

void Editor::EditInstruction(Labels state, Labels symbol)
{
  // edit a specific instruction
  uint32_t context = LABEL_BIT(Labels::Instruction);
  uint32_t allowed = LABEL_BIT(Labels::Run) + LABEL_BIT(Labels::Tape) + LABEL_BIT(Labels::Machine) + LABEL_BIT(Labels::Symbol);
  Machine::Instruction instruction;
  machine.GetInstruction(state, symbol, instruction);
  pixelRing.Clear();
  SetCursor(Labels::WriteSymbol);
  while (true)
  {
    ShowMenu(context, allowed);
    ShowInstruction(symbol, instruction);
    ShowSymbol(symbol);
    Labels pick = Pick();
    if (pick == Labels::WriteSymbol)
    {
      pick = PickSymbol(instruction._writeSymbol, LABEL_BIT(Labels::Instruction) + LABEL_BIT(Labels::Symbol), 0);
      if (Machine::IsASymbol(pick))
        instruction._writeSymbol = pick;
      SetCursor(Labels::WriteSymbol);
    }
    else if (pick == Labels::NewState)
    {
      pick = PickState(instruction._newState, LABEL_BIT(Labels::Instruction) + LABEL_BIT(Labels::State), 0);
      if (Machine::IsAState(pick, true))
        instruction._newState = pick;
      SetCursor(Labels::NewState);
    }
    else if (pick == Labels::MoveRight)
    {
      instruction._moveHead = +1;
    }
    else if (pick == Labels::MoveLeft)
    {
      instruction._moveHead = -1;
    }
    else if (pick == Labels::MoveNone)
    {
      instruction._moveHead = 0;
    }
    else if (pick == Labels::Symbol)
    {
      // go up
      machine.PutInstruction(state, symbol, instruction);
      break;
    }
    else
    {
      machine.PutInstruction(state, symbol, instruction);
      Default(context, allowed, pick);
      break;
    }
  }
}

void Editor::EditMenu()
{
  // Offer extras
  // Menu.A.S load example #S from PROGMEM
  // Menu.B.S load from EEPROM slot #S
  // Menu.C.S save to EEPROM slot #S
  // Menu.D read from Serial
  // Menu.E write to Serial
  // Menu.F Clock
  // Menu.G Set time
  // Menu.H Brightness
  // Menu.I Off
  uint32_t context = LABEL_BIT(Labels::Menu) + LABEL_BIT(Labels::State);
  uint32_t allowed = 0;
  pixelRing.Clear();
  ShowMenu(context, allowed);
  SetCursor(Labels::Menu);
  uint32_t secondLevelPickColour = pixelRing.Colour(ColourPalette::Malachite);
  while (true)
  {
    Labels pick = PickN(Index(Labels::Menu_Last) - Index(Labels::Menu_First) + 1, context, allowed);
    if (pick == Labels::Menu)
    {
      break;
    }
    else if (pick == Labels::Menu_LoadPROGMEM)
    {
      // load example N from PROGMEM
      Labels pick = PickN(Examples::Count, LABEL_BIT(Labels::Menu) + LABEL_BIT(Labels::Symbol), 0, secondLevelPickColour);
      if (machine.IsASymbol(pick))
      {
        Examples::Load(static_cast<int>(pick) - 1);
      }
      break;
    }
    else if (pick == Labels::Menu_LoadEEPROM)
    {
      // load machine from EEPROM slot N
      Labels pick = PickN(config._NumberOfMachineSlotsInEEPROM, LABEL_BIT(Labels::Menu) + LABEL_BIT(Labels::Symbol), 0, secondLevelPickColour);
      if (machine.IsASymbol(pick))
      {
        config.LoadSlot(Index(pick) - 1);
      }
      break;
    }
    else if (pick == Labels::Menu_SaveEEPROM)
    {
      // save machine to EEPROM slot N
      Labels pick = PickN(config._NumberOfMachineSlotsInEEPROM, LABEL_BIT(Labels::Menu) + LABEL_BIT(Labels::Symbol), 0, secondLevelPickColour);
      if (machine.IsASymbol(pick))
      {
        config.SaveSlot(Index(pick) - 1);
      }
      break;
    }
    else if (pick == Labels::Menu_ReadSerial)
    {
      // read from Serial
      SerialReader reader;
      // show we're waiting
      pixelRing.Clear();
      ShowMenu(LABEL_BIT(Labels::Menu), 0);
      SetCursor(Labels::Null);
      pixelRing.Show();
      machine.DeSerialise(reader);
      break;
    }
    else if (pick == Labels::Menu_WriteSerial)
    {
      // write to Serial
      SerialWriter writer;
      machine.Serialise(writer);
      break;
    }
    else if (pick == Labels::Menu_Brightness)
    {
      // set brightness
      SetBrightness();
      break;
    }
    else if (pick == Labels::Menu_Speed)
    {
      // set speed
      SetSpeed();
      break;
    }
    else if (pick == Labels::Menu_Off)
    {
      // blank the ring and wait
      SetCursor(Labels::Null);
      pixelRing.Clear();
      pixelRing.Show();
      while (true)
      {
        wheel.Update();
        if (wheel.GetRotation() != 0)
          break;
        if (btn.CheckButtonPress())
          break;
      }
      break;
    }
    else
    {
      // run a program Clock, Set
      Program(pick);
      break;
    }
  }
  _mode = Labels::Tape;
}

Labels Editor::Pick(bool* longPress)
{
  // updates the wheel, pixels and the cursor waiting for a button press, returns the label selected
  // if longPress is non-NULL, will set to true if button was held
  while (true)
  {
    wheel.Update();
    pixelRing.Update();
    pixelRing.MoveCursor(wheel.GetRotation());
    if (btn.CheckButtonPress())
    {
      if (longPress)
      {
        // interested in a long press
        int ctr = LongPressDurationMS;
        *longPress = false;
        while (ctr--)
        {
          if (!btn.IsDown())  // released early, not a long press
            break;
          delay(1);
        }
        if (btn.IsDown())
        {
          *longPress = true;  // held down long enough
        }
      }
      return pixelRing.LabelAtCursor();
    }
  }
  return Labels::Null;
}

Labels Editor::PickSymbol(Labels defaultLabel, uint32_t context, uint32_t allowed)
{
  // return a symbol (or a menu item)
  pixelRing.Clear();
  ShowSymbols();
  ShowMenu(context, allowed);
  SetCursor((defaultLabel != Labels::Blank)?defaultLabel:Labels::A);
  return Pick();
}

Labels Editor::PickN(int Count, uint32_t context, uint32_t allowed, uint32_t colour)
{
  // pick from the first N symbols, coloured green
  pixelRing.Clear();
  if (!colour)
    colour = pixelRing.Colour(MenuAllowedColour);
  int n = 0;
  for (int lbl = Index(Labels::A); lbl <= Index(Labels::LastLetter); lbl++, n++)
  {
    Labels label = Label(lbl);
    if (n < Count)
      pixelRing.SetPixel(pixelRing.LabelToDisplayPixel(label), colour);
  }
  ShowMenu(context, allowed);
  SetCursor(Labels::A);
  Labels pick = Pick();
  if (machine.IsASymbol(pick))
  {
    n = Index(pick) - Index(Labels::A);
    if (n < 0 || n >= Count)
      pick = Labels::Null;
  }
  return pick;
}

Labels Editor::PickState(Labels defaultLabel, uint32_t context, uint32_t allowed)
{
  // return a state (or a menu item)
  pixelRing.Clear();
  ShowSymbols();
  ShowMenu(context, allowed);
  SetCursor(defaultLabel);
  return Pick();
}

Labels Editor::PickInstruction(Labels state, uint32_t context, uint32_t allowed)
{
  // return a symbol (or a menu item)
  pixelRing.Clear();
  ShowSymbols();
  ShowMenu(context, allowed);
  SetCursor(Labels::A);
  ShowInstruction(state, pixelRing.LabelAtCursor());
  while (true)  // live update of the instruction
  {
    wheel.Update();
    pixelRing.Update();
    if (pixelRing.MoveCursor(wheel.GetRotation()))
    {
      // blink
      ShowInstruction(state, Labels::Null);
      pixelRing.Update();
      delay(25);
      ShowInstruction(state, pixelRing.LabelAtCursor());
    }
    if (btn.CheckButtonPress())
      return pixelRing.LabelAtCursor();
  }
  return Labels::Null;
}

void Editor::ShowSymbols()
{
  // show the available symbols
  for (int lbl = Index(Labels::A); lbl <= Index(Labels::LastLetter); lbl++)
  {
    Labels label = Label(lbl);
    pixelRing.SetPixel(pixelRing.LabelToDisplayPixel(label), tape.SymbolColour(label));
  }
  pixelRing.SetPixel(pixelRing.LabelToDisplayPixel(Labels::Blank), tape.SymbolColour(Labels::Blank));
}

void Editor::ShowSymbol(Labels sym)
{
  // show just one symbol/state
  for (int lbl = Index(Labels::A); lbl <= Index(Labels::LastLetter); lbl++)
  {
    Labels label = Label(lbl);
    pixelRing.SetPixel(pixelRing.LabelToDisplayPixel(label), pixelRing.Colour(ColourPalette::Black));
  }
  pixelRing.SetPixel(pixelRing.LabelToDisplayPixel(sym), tape.SymbolColour(sym));
}

void Editor::ShowMenu(uint32_t context, uint32_t allowed)
{
  // show the context (current) menu items white, allowed (available) green. the rest black
  uint32_t menuBitMask = LABEL_BIT(Labels::Clear) +  LABEL_BIT(Labels::Tape) +  LABEL_BIT(Labels::Instruction) +  LABEL_BIT(Labels::State) +  LABEL_BIT(Labels::Machine) +  LABEL_BIT(Labels::Symbol) +  LABEL_BIT(Labels::Run) + LABEL_BIT(Labels::Menu);
  for (int lbl = 0; lbl < 32; lbl++)
  {
    if (context & (1UL << lbl))
      pixelRing.SetPixel(pixelRing.LabelToDisplayPixel(Label(lbl)), pixelRing.Colour(MenuContextColour));
    else
      if (menuBitMask & (1UL << lbl))
        pixelRing.SetPixel(pixelRing.LabelToDisplayPixel(Label(lbl)), pixelRing.Colour(ColourPalette::Black));
    if (allowed & (1UL << lbl))
      pixelRing.SetPixel(pixelRing.LabelToDisplayPixel(Label(lbl)), pixelRing.Colour(MenuAllowedColour));
  }
}

void Editor::ShowInstruction(Labels state, Labels sym)
{
  // show the Instruction (write, move, new state) when in the given state and reading the given symbol 
  Machine::Instruction instruction;
  if (Machine::IsASymbol(sym) && machine.GetInstruction(state, sym, instruction))
  {
    ShowInstruction(sym, instruction);
  }
  else
  {
    ShowInstruction(Labels::Null, instruction); // blank
  }
}

void Editor::ShowInstruction(Labels sym, Machine::Instruction& instruction)
{
  // show the given Instruction (write, move, new state) when the given symbol 
  uint32_t Write = tape.SymbolColour(Labels::Blank);
  uint32_t Left = pixelRing.Colour(ColourPalette::Black);
  uint32_t None = pixelRing.Colour(ColourPalette::Black);
  uint32_t Right = pixelRing.Colour(ColourPalette::Black);
  uint32_t NewState = tape.SymbolColour(Labels::Blank);
  if (Machine::IsASymbol(sym))
  {
    Write = tape.SymbolColour(instruction._writeSymbol);
    if (instruction._moveHead == 0)
    {
      None = pixelRing.Colour(ColourPalette::White);
      // make an undefined instruction more obvious, make no move grey
      if (instruction._writeSymbol == Labels::Blank && instruction._newState == Labels::Halt)
      {
        None = pixelRing.Colour(ColourPalette::Grey);
      }
    }
    else if (instruction._moveHead == -1)
      Left = pixelRing.Colour(ColourPalette::White);
    else
      Right = pixelRing.Colour(ColourPalette::White);
    NewState = tape.SymbolColour(instruction._newState);
  }  
  pixelRing.SetPixel(pixelRing.LabelToDisplayPixel(Labels::WriteSymbol), Write);
  pixelRing.SetPixel(pixelRing.LabelToDisplayPixel(Labels::MoveLeft), Left);
  pixelRing.SetPixel(pixelRing.LabelToDisplayPixel(Labels::MoveNone), None);
  pixelRing.SetPixel(pixelRing.LabelToDisplayPixel(Labels::MoveRight), Right);
  pixelRing.SetPixel(pixelRing.LabelToDisplayPixel(Labels::NewState), NewState);
}

void Editor::SetCursor(Labels label)
{
  // set the cursor at the label
  pixelRing.SetCursor((label != Labels::Null)?pixelRing.LabelToDisplayPixel(label):-1);
}

bool Editor::Default(uint32_t /*context*/, uint32_t allowed, Labels pick)
{
  // default handling of a pick at label
  if (allowed & LABEL_BIT(pick))
  {
    // menu chosen, change mode
    _mode = pick;
    return true;
  }
  return false;
}

void Editor::SetBrightness()
{
  int brite = config._PixelBrightness;
  uint16_t dHue = 0xFFFF/PixelRing::Count;
  for (int idx = 0; idx < PixelRing::Count; idx++)
  {
    pixelRing.SetPixel(idx, Adafruit_NeoPixel::ColorHSV(idx*dHue));
  }
  pixelRing.SetCursor(brite/Config::StepBrightness - 1);
  while (true)
  {
    pixelRing.Brightness(brite);
    pixelRing.Update();
    wheel.Update();
    int rot = wheel.GetRotation();
    if (rot)
    {
      brite += rot*Config::StepBrightness;
      if (brite < Config::MinBrightness)
        brite = Config::MinBrightness;
      if (brite > Config::MaxBrightness)
        brite = Config::MaxBrightness;
      pixelRing.SetCursor(brite/Config::StepBrightness - 1);
    }
    else if (btn.CheckButtonPress())
      break;
  }
  config._PixelBrightness = brite;
  config.Save();
}

void Editor::SetSpeed()
{
  // speed (AND animation mode)
  int DelayMS = config._MachineStepDelayMS;
  int steps = (Config::MaxStepDelayMS - Config::MinStepDelayMS)/Config::StepDelayIncrementMS;
  uint16_t dHueBlue = 0xAAAA; // blue
  uint16_t dHue = (0xFFFF - dHueBlue)/(steps - 1); // blue-red
  pixelRing.Clear();
  for (int idx = 0; idx < steps; idx++)
  {
    pixelRing.SetPixel(idx, Adafruit_NeoPixel::ColorHSV(dHueBlue + idx*dHue));
  }
  pixelRing.SetPixel(pixelRing.LabelToDisplayPixel(Labels::State), pixelRing.Colour(MenuAllowedColour));  // animate state
  int csr = steps - DelayMS/Config::StepDelayIncrementMS - 1;
  pixelRing.SetCursor(csr);
  while (true)
  {
    pixelRing.Update();
    wheel.Update();
    int rot = wheel.GetRotation();
    if (rot)
    {
      csr += rot;
      if (csr < 0) csr = PixelRing::Count - 1;
      if (csr >= PixelRing::Count) csr = 0;
      pixelRing.SetCursor(csr);
    }
    else if (btn.CheckButtonPress())
      break;
  }
  DelayMS = Config::MinStepDelayMS + (steps - csr - 1)*Config::StepDelayIncrementMS;
  if (csr == pixelRing.LabelToDisplayPixel(Labels::State))
  {
    // toggle animate state
    config._AmimateState = !config._AmimateState;
    config.Save();
  }
  else if (Config::MinStepDelayMS <= DelayMS && DelayMS <= Config::MaxStepDelayMS)
  {
    // save speed
    config._MachineStepDelayMS = DelayMS;
    config.Save();
  }
}
