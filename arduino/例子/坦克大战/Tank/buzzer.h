/**
 * 蜂鸣器头文件
 */
#ifndef __BUZZER_H_
#define __BUZZER_H_

#include "global.h"

typedef short Bell;

const PROGMEM Bell music[][10]
{
  {1200, 600, 1000,-1},
  {1600, 800, 1700,-1}
};

class GBuzzer
{
  private:
    Pin pin;
    Freq f;
    Col c;
  public:
    /************/
    GBuzzer(Pin pin);
    ~GBuzzer(void);
    void on(Freq f);
    void on(Freq f, T t);
    void off();
    void audioPlay(Row row, T t);
};

#endif
