#include "buzzer.h"

GBuzzer::GBuzzer(Pin pin)
{
  this->pin = pin;
  this->c = 10;
}

GBuzzer::~GBuzzer(void){ ; };

void GBuzzer::on(Freq f)
{
  tone(pin, f);
}

void GBuzzer::on(Freq f, T t)
{
  tone(pin, f);
  delay(t);  
}

void GBuzzer::off()
{
  noTone(pin);  
}

void GBuzzer::audioPlay(Row row, T t)
{
  for(byte i=0;i<this->c;i++)
  {
    Bell b = pgm_read_word(&music[row][i]);
    if(b<=0) break;
    tone(pin, b);
    delay(t);
    off();
  }
}


