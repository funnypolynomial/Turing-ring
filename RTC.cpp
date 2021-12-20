#include "arduino.h"
#include "Wire.h"
#include "RTC.h"

// real time clock
RTC rtc;

#define RTC_DS1307_I2C_ADDRESS 0x68

RTC::RTC():
    m_Hour24(0),
    m_Minute(0),
    m_Second(0),
    m_DayOfWeek(1), 
    m_DayOfMonth(1),
    m_Month(1),
    m_Year(12)
{
}

void RTC::setup(void)
{
  Wire.begin();
  ReadTime(true);
}

byte RTC::BCD2Dec(byte BCD)
{
  return (BCD/16*10) + (BCD & 0x0F);
}

byte RTC::Dec2BCD(byte Dec)
{
  return (Dec/10*16) + (Dec % 10);
}

void RTC::ReadTime(bool Full)
{
  if (Full)
  {
    // from register 0
    Wire.beginTransmission(RTC_DS1307_I2C_ADDRESS);
    Wire.write((byte)0x00);
    Wire.endTransmission();
   
    Wire.requestFrom(RTC_DS1307_I2C_ADDRESS, 7);
   
    m_Second = BCD2Dec(Wire.read() & 0x7F);  // high bit is CH (Clock Halt)
  }
  else
  {
    // just the minutes and hours
    // from register 1
    Wire.beginTransmission(RTC_DS1307_I2C_ADDRESS);
    Wire.write((byte)0x01);
    Wire.endTransmission();
   
    Wire.requestFrom(RTC_DS1307_I2C_ADDRESS, 2);
  }
  m_Minute = BCD2Dec(Wire.read());
  byte Register2 = Wire.read();
  if (Register2 & 0x40)  // 12/24 hr
  {
    // 12 hr mode, bit 6=PM
    m_Hour24 = BCD2Dec(Register2 & 0x3F);
    if (Register2 & 0x20)
    {
      m_Hour24 += 12;
      if (m_Hour24 > 23) m_Hour24 = 0;
    }
  }
  else
  {
    // 24 hour mode
    m_Hour24 = BCD2Dec(Register2 & 0x3F);
  }
  
  if (Full)
  {
    m_DayOfWeek  = BCD2Dec(Wire.read());
    m_DayOfMonth = BCD2Dec(Wire.read());
    m_Month      = BCD2Dec(Wire.read());
    m_Year       = BCD2Dec(Wire.read());
  }
}

byte RTC::ReadSecond(void)
{
  // from register 01
  Wire.beginTransmission(RTC_DS1307_I2C_ADDRESS);
  Wire.write(0x00);
  Wire.endTransmission();
 
  Wire.requestFrom(RTC_DS1307_I2C_ADDRESS, 1);
 
  m_Second = BCD2Dec(Wire.read());
  return m_Second;
}

byte RTC::ReadMinute(void)
{
  // from register 01
  Wire.beginTransmission(RTC_DS1307_I2C_ADDRESS);
  Wire.write(0x01);
  Wire.endTransmission();
 
  Wire.requestFrom(RTC_DS1307_I2C_ADDRESS, 1);
 
  return BCD2Dec(Wire.read());
}

void RTC::WriteTime(void)
{
   Wire.beginTransmission(RTC_DS1307_I2C_ADDRESS);
   Wire.write((byte)0x00);
   Wire.write(Dec2BCD(m_Second));
   Wire.write(Dec2BCD(m_Minute));
   Wire.write(Dec2BCD(m_Hour24));  // 24 hr mode
   Wire.write(Dec2BCD(m_DayOfWeek));
   Wire.write(Dec2BCD(m_DayOfMonth));
   Wire.write(Dec2BCD(m_Month));
   Wire.write(Dec2BCD(m_Year));
   Wire.endTransmission();
}

byte RTC::ReadByte(byte Index)
{
  Wire.beginTransmission(RTC_DS1307_I2C_ADDRESS);
  Wire.write(Index);
  Wire.endTransmission();
 
  Wire.requestFrom(RTC_DS1307_I2C_ADDRESS, 1);
  return Wire.read();
}

void RTC::WriteByte(byte Index, byte Value)
{
   Wire.beginTransmission(RTC_DS1307_I2C_ADDRESS);
   Wire.write(Index);
   Wire.write(Value);
   Wire.endTransmission();
}

byte RTC::ReadTemperature()
{
  // NOT for DS1307
  byte Temperature = ReadByte(0x11);
  if (Temperature & 0x80)
  {
    // -ve -> 0
    Temperature = 0;
  }
  else if (rtc.ReadByte(0x12) & 0x80)
  {
    // fractional part >=0.5; round
    Temperature++;
  }
  return Temperature;
}  

#define rollIn(_value, _bits) theSeed = (theSeed << _bits) | _value

unsigned long RTC::getSeed()
{
  unsigned long theSeed = 0;
  rollIn(m_Year, 6);
  rollIn(m_Month, 4);
  rollIn(m_DayOfMonth, 5);
  rollIn(m_DayOfWeek, 3);
  rollIn(m_Hour24, 4);
  rollIn(m_Minute, 5);
  rollIn(m_Second, 5);
  return theSeed;
}
