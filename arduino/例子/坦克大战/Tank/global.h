#ifndef __GLOBAL_H
#define __GLOBAL_H

#include <Arduino.h>

typedef uint8_t Pin; 
typedef unsigned long T, Freq;
typedef short Row, Col, Num;

const uint8_t page_item_length = 20;

enum Music
{
  a,b,c,d,e,f,g,h,i,j,k,l,m,n
};

enum KeyMap
{
  KEY_UP=1,
  KEY_DOWN,
  KEY_LEFT,
  KEY_RIGHT,
  KEY_AA,
  KEY_BB  
};

#endif
