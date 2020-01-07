#include "led.h"

GLed::GLed(Pin pin)
{
  this->led_pin = pin;
  pinMode(led_pin, OUTPUT);
}

GLed::~GLed(void)
{
  
}

void GLed::setLevel(Level value)
{
  light_level = value;
}

void GLed::on()
{
  if(light_level > 0)
  {
    analogWrite(led_pin, light_level);  
  }
  else
  {
    digitalWrite(led_pin, HIGH);
  }
}

void GLed::on(T t)
{
  if(light_level > 0)
  {
    analogWrite(led_pin, light_level);  
  }
  else
  {
    digitalWrite(led_pin, HIGH); 
  }
  delay(t);
}

void GLed::off()
{
  digitalWrite(led_pin, LOW);  
}

void GLed::off(T t)
{
  digitalWrite(led_pin, LOW);
  delay(t);  
}

//呼吸灯 f:亮度改变间隔
void GLed::bln(Freq f)
{
  if(light_level > 0)
  {
    int i = 0, delta = 1;
    lp:while(1){
      i+=delta;
      analogWrite(led_pin, i);
      delay(f);  
      
      if(delta == 1)
      {
        if(i>light_level)
          goto reverse;
      }
      else if(delta == -1)
      {
        if(i<=0)
          return;
      }
    }
    reverse:delta = ~delta+1;
    goto lp;
  }
  else
  {
    return;  
  } 
}


