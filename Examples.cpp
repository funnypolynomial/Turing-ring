#include <Arduino.h>
#include <avr/pgmspace.h>
#include "ReadWrite.h"
#include "PixelRing.h"
#include "Machine.h"
#include "Examples.h"

// a:just flips black to blue and vice versa
static const char pFlip[] PROGMEM     = "A=x:cRA,c:xRA\n"
                                        "xxxxxxxxxxxxcccccccccccc\n";

// b:cycle R/G/B
static const char pCycle[] PROGMEM    = "A=x:aRA,a:aNB,c:aRA\n"
                                        "B=a:bRB,b:bNC\n"
                                        "C=b:cRC,c:cNA\n"
                                        "\n";
// c:binary increment
static const char pInc[] PROGMEM      = "A=x:gRD,d:dNX,g:xRA\n" // read a 0, write a 1, Done, read a 1, write a 0, continue
                                        "D=x:xRD,d:dRA,g:gRD\n" // wrap around to stop symbol (d)
                                        "xxxxxxxxxxxxxxxxxxxxxxxd\n";

// d:duplicate: x, followed by N White ->x, followed by N White, x, followed by N White
// alternate format
static const char pDup[] PROGMEM      = "1=Bla:BlaR1,Whi:WhiR2\n"
                                        "2=Bla:RedR3,Whi:WhiR2\n"
                                        "3=Bla:BlaR3,Whi:BluR4,Red:RedR5,Blu:BluR3,Gre:GreR3\n"
                                        "4=Bla:GreR3,Whi:WhiR4,Red:RedR4,Blu:BluR4,Gre:GreR4\n"
                                        "5=Bla:BlaR5,Red:BlaR5,Blu:WhiR5,Gre:WhiR5,Whi:WhiR6\n"
                                        "6=Bla:BlaRX,Whi:WhiR6\n"
                                        " WhiWhiWhiWhiWhiWhi                 \n";
                                        
// e:binary add: 1=White/g, 0=Blue/c
static const char pAdd[] PROGMEM      = "A=x:xRB,c:cRA,g:gRA,d:dRA,h:hRA\n"
                                        "B=x:xLC,c:cRB,g:gRB,d:dRB,h:hRB\n"
                                        "C=c:xLD,g:xLE,x:xLJ\n"
                                        "D=x:xLF,c:cLD,g:gLD,d:dLD,h:hLD\n"
                                        "E=x:xLG,c:cLE,g:gLE,d:dLE,h:hLE\n"
                                        "F=c:dRA,g:hRA,x:dRA,d:dLF,h:hLF\n"
                                        "G=c:gNH,g:cLG,x:gNH,d:dLG,h:hLG\n"
                                        "H=d:dLI,h:hLI,x:xLI,c:cRH,g:gRH\n"
                                        "I=c:dRA,g:hRA\n"
                                        "J=d:cLJ,h:gLJ,x:xRX,c:cLJ,g:gLJ\n"
                                        "ccggcggc ccgcgcgg       \n"; // 00110110 00101011 -> 01100001
  
bool Examples::Load(int n)
{
  const char* pExample = NULL;
  if (n == 0)    pExample = pFlip;
  if (n == 1)    pExample = pCycle;
  if (n == 2)    pExample = pInc;
  if (n == 3)    pExample = pDup;
  if (n == 4)    pExample = pAdd;
  if (pExample != NULL)
  {
    PROGMEMReader reader;
    reader._basePtr = pExample;
    machine.DeSerialise(reader);
  }
  return false;
}
