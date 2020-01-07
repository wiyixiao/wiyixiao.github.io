/**
 * OLED头文件
 */

#ifndef __OLED_H_
#define __OLED_H_

#if ARDUINO >= 100
 #include "Arduino.h"
 #include "Print.h"
#else
 #include "WProgram.h"
#endif

#include "../I2c/Wire.h"
#include "../128x64/ZtLib.h"
#include "../128x64/SSD1306.h"

class GOled:public ZtLib, public SSD1306, public Print
{
  public:
    GOled();
    ~GOled();
    void
      begin(void),
      display(void),
      clear(void),
      drawPixel(int16_t x, int16_t y, uint16_t color),
      drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color),
      drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color),
      drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color),
      drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color),
      fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color),
      drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color),
      drawCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername,
                uint16_t color),
      fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color),
      fillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername,
                  int16_t delta, uint16_t color),
      drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                  int16_t x2, int16_t y2, uint16_t color),
      fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                  int16_t x2, int16_t y2, uint16_t color),
      drawRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h,
                  int16_t radius, uint16_t color),
      fillRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h,
                  int16_t radius, uint16_t color),
      drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap,
                  int16_t w, int16_t h, uint16_t color),
      drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color,
                  uint16_t bg, uint8_t size),
      setCursor(int16_t x, int16_t y),
      fillScreen(uint16_t color),
      setTextColor(uint16_t c),
      setTextColor(uint16_t c, uint16_t bg),
      setTextSize(uint8_t s),
      setTextWrap(boolean w),
      setRotation(uint8_t r);
    uint8_t getRotation(void);
    int16_t
      height(void),
      width(void);

  #if ARDUINO >= 100
    virtual size_t write(uint8_t);
  #else
    virtual void   write(uint8_t);
  #endif
   protected:
    const int16_t
      WIDTH, HEIGHT;   
    int16_t
      _width, _height, 
      cursor_x, cursor_y;
    uint16_t
      textcolor, textbgcolor;
    uint8_t
      textsize,
      rotation;
    boolean
      wrap; 
};

#endif
