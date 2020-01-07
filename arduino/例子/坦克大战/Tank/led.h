/**
 * led头文件
 */
#ifndef __LED_H_
#define __LED_H_

#include "global.h"

typedef uint8_t Level; //0-255

class GLed
{
  private:
    Pin led_pin;
    Level light_level;
    Freq f;
  public:
    GLed(Pin pin);
    ~GLed(void);
    void setLevel(Level value);
    void on();
    void on(T t);
    void off();
    void off(T t);
    void bln(Freq f);
};

#endif
