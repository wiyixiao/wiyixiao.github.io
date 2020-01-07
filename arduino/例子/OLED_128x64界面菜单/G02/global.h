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
  UP=1,
  DOWN,
  LEFT,
  RIGHT,
  AA,
  BB  
};

typedef enum MenuPage
{
	main,option
}Page;

#endif
