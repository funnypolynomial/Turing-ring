#pragma once

#define PIN_ENCODER_A   2
#define PIN_ENCODER_B   3
#define PIN_ENCODER_BTN A2
#define PIN_PIXEL_RING  A3
#define PIN_RTC_SDA     A4
#define PIN_RTC_SCL     A5

/*
                              *** Schematic ***
                                       USB
                                  +---||||---+               Pwr
                                  |   Nano   |             +----+
  +-----------+                   |          |      [+5V]--+ 5V |<===
  |  LED Ring |                   |          |      [Gnd]--+ Gnd|<===
  |           |                   |          |             +----+
  |       5V  +--[+5V]            |          |
  |       Gnd +--[Gnd]     [BTN]--+ A2       |         +------------+
  |       DIN +--[RNG]     [RNG]--+ A3       |         |  Rotary/PB |
  +-----------+            [SDA]--+ A4       |         |            |
                           [SCL]--+ A5       |  [BTN]--+ Button     |
                           [+5V]--+ 5V    D3 +---------+ ENC_B      |
                           [Gnd]--+ Gnd   D2 +---------+ ENC_A   Gnd+--[Gnd]
                                  |          |         +------------+
                                  +----------+ 
                                  
                 +------------+
                 | RTC/DS3231 |       
                 |  {0x68}    |       
                 |        5V  +--[+5V]
                 |        Gnd +--[Gnd]
                 |        SDA +--[SDA]
                 |        SCL +--[SCL]
                 +------------+      
   
   Notes:
   [Labels] are mutually connected.
   LED Ring (from Zafar, as prize for solving a TM puzzle) seems to be this one https://grobotronics.com/led-ring-24-x-ws2812-5050-rgb.html?sl=en (smaller than a Jaycar ring)
   Rotary encoder: https://www.jaycar.co.nz/rotary-encoder-with-pushbutton/p/SR1230
*/
