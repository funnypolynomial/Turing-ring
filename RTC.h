#ifndef rtc_h
#define rtc_h

// Talk to the DS1307/DS3232

// 0x14..0x3F should be safe to read/write on either:
#define RTC_RAM_BASE_INDEX 0x14

class RTC
{
  public:
    RTC();
    void setup();
    byte BCD2Dec(byte BCD);
    byte Dec2BCD(byte Dec);
    void ReadTime(bool Full);
    byte ReadSecond(void);
    byte ReadMinute(void);
    void WriteTime(void);
    byte ReadByte(byte Index);
    void WriteByte(byte Index, byte Value);
    
    byte ReadTemperature();
    
    unsigned long getSeed();
    
    byte m_Hour24;      // 0..23
    byte m_Minute;      // 0..59
    byte m_Second;      // 0..59
    byte m_DayOfWeek;   // 1..7 
    byte m_DayOfMonth;  // 1..31
    byte m_Month;       // 1..12
    byte m_Year;        // 0..99
};

extern RTC rtc;


#endif
