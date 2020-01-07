#include <avr/pgmspace.h>
#include <string.h>

#define LOGO_CLOCK_HEIGHT 16
#define LOGO_CLOCK_WIDTH 16
const static unsigned char PROGMEM logo_clock_bmp[] = 
{
  0x7F,0xFC,0xFF,0xFE,0xFE,0xFE,0xF0,0x1E,0xE3,0x0E,0xC3,0x06,0xC3,0x06,0xC3,0x06,
  0xC3,0x86,0xC1,0xC6,0xC0,0xE6,0xE0,0x6E,0x70,0x1C,0x7C,0x7C,0xFF,0xFE,0xC7,0xC6
};

#define CLOCK_IMG_HEIGHT 36
#define CLOCK_IMG_WIDTH 49
const static unsigned char PROGMEM clock_page_img[] = 
{
  0x00,0x00,0x00,0x0F,0xC0,0x00,0x00,0x00,0x00,0x0E,0x1F,0xE0,0x00,0x00,0x00,0x00,
  0x0E,0x3F,0xF0,0x00,0x00,0x00,0x00,0x0F,0xFF,0xF0,0x00,0x00,0x00,0x1F,0x8F,0xFF,
  0xF0,0x06,0x00,0x00,0x3F,0xF8,0x07,0xF1,0x83,0x00,0x00,0x3F,0xE7,0xF9,0xF0,0xC1,
  0x80,0x00,0x7F,0x9F,0xFE,0x60,0x60,0x80,0x00,0x7F,0x3F,0xFF,0xB0,0x30,0x80,0x00,
  0x7E,0x7F,0xFF,0xD8,0x10,0x80,0x00,0x7E,0xFE,0xFF,0xE8,0x11,0x80,0x00,0x3D,0xFE,
  0xFF,0xEC,0x11,0x80,0x03,0x3D,0xFF,0xFF,0xF4,0x10,0x00,0x42,0x0B,0xFF,0x7F,0xF6,
  0x30,0x00,0x46,0x1B,0xFF,0x7F,0xFE,0x20,0x00,0xC4,0x1F,0xFF,0x7F,0xFA,0x00,0x00,
  0x84,0x17,0xFF,0x78,0xFA,0x00,0x00,0x84,0x17,0xFF,0x47,0xFA,0x00,0x00,0x84,0x17,
  0xFF,0x3F,0xFA,0x00,0x00,0xC6,0x1F,0xFF,0xFF,0xFA,0x00,0x00,0x63,0x1B,0xFF,0xFF,
  0xFA,0x00,0x00,0x01,0x9B,0xFF,0xFF,0xFE,0x00,0x00,0x00,0x8B,0xFF,0xFF,0xF6,0x00,
  0x00,0x00,0x0D,0xFF,0xFF,0xF4,0x00,0x00,0x00,0x0D,0xFF,0xFF,0xEC,0x00,0x00,0x00,
  0x06,0xFF,0xFF,0xC8,0x00,0x00,0x00,0x03,0x7F,0xFF,0x98,0x00,0x00,0x00,0x01,0xBF,
  0xFF,0x38,0x00,0x00,0x00,0x01,0xDF,0xFE,0x7C,0x00,0x00,0x00,0x00,0x63,0xF9,0xFE,
  0x00,0x00,0x00,0x00,0x3C,0x0F,0x1E,0x00,0x00,0x00,0x00,0x7F,0xFC,0x0C,0x00,0x00,
  0x00,0x00,0x78,0x00,0x00,0x00,0x00,0x00,0x00,0x78,0x00,0x00,0x00,0x00,0x00,0x00,
  0x70,0x00,0x00,0x00,0x00,0x00,0x00,0x70,0x00,0x00,0x00,0x00
};

const char item_0[] PROGMEM = "set time";
const char item_1[] PROGMEM = "set date";
const char item_2[] PROGMEM = "set dow";
const char item_3[] PROGMEM = "set alarm clock";
const char item_4[] PROGMEM = "set audio";

const char* const menuItem[] PROGMEM = {
  item_0, item_1, item_2, item_3, item_4
};

enum page{mainP, itemP, timeP, dateP, dowP, clockP, audioP};

char buffer[20];
int item = 0;
int itemCount = sizeof(menuItem)/sizeof(menuItem[0]);
int timeItem = 0;
int audioItem = 0;
int clockItem = 0;

String toWeek(int w)
{
  switch(w)
  {
    case 1: return "Mon."; break;
    case 2: return "Tues."; break;
    case 3: return "Wed."; break;
    case 4: return "Thu."; break;
    case 5: return "Fri."; break;
    case 6: return "Sat."; break;
    case 7: return "Sun."; break;
  }
}

//main page
void mainPage(ht * pht)
{
  pageSet(mainP);
  //set color
  display.setTextColor(WHITE);
  //set temp
  display.setTextSize(1);
  display.setCursor(5, 0);
  display.print("Temp:");
  display.print((int)pht->ht_t);
  display.print("C");
  //set Hum
  display.setCursor(80, 0);
  display.print("Hum:");
  display.print((int)pht->ht_h);
  display.print("%");
  //set time
  display.setTextSize(2);
  display.setCursor(17, 25);
  timeDisplay();
  //set date
  display.setTextSize(1);
  display.setCursor(5, 55);
  dateDisplay();
  //set week
  display.setCursor(95, 55);
  display.print(toWeek(tdata.td_dow));
  //set clock
  if(tclock.tc_enable) clockDisplay();
}

//item page
void itemPage()
{
  int h = 12;
  int i;
  display.setTextSize(1);
  pageSet(itemP);
  for(i=0;i<itemCount;i++)
  {
    if(item == i)
    {
      display.drawRect(0, i * h, display.width(), h - 2, WHITE);
    }
    display.setCursor(5, i * h + 1);
    strcpy_P(buffer, (char *)pgm_read_word(&(menuItem[i])));
    display.print(buffer);
  }
}

void timeSetPage()
{
  pageSet(timeP);
  display.setTextSize(1);
  display.setCursor(5, 5);
  display.print("Press button A");
  display.setTextSize(2);
  display.setCursor(17, 25);
  timeDisplay();
  switch(timeItem)
  {
    case 0:
      display.drawLine(15, 45, 40, 45, WHITE); //point0 , point1
      break;
    case 1:
      display.drawLine(51, 45, 76, 45, WHITE);
      break;
    case 2:
      display.drawLine(86, 45, 112, 45, WHITE);
      break;
  }  
}

void dateSetPage()
{
  pageSet(dateP);
  display.setTextSize(1);
  display.setCursor(5, 5);
  display.print("Press button A");
  display.setTextSize(2);
  display.setCursor(17, 25);
  dateDisplay();
  switch(timeItem)
  {
    case 0:
      display.drawLine(15, 45, 40, 45, WHITE); 
      break;
    case 1:
      display.drawLine(50, 45, 76, 45, WHITE);
      break;
    case 2:
      display.drawLine(86, 45, 114, 45, WHITE);
      break;
  }  
}

void dowSetPage()
{
  pageSet(dowP);
  display.setTextSize(1);
  display.setCursor(5, 5);
  display.print("Press button A");
  display.setTextSize(2);
  display.setCursor(45, 25);
  display.print(toWeek(tdata.td_dow));
}

void clockSetPage()
{
  pageSet(clockP);
  display.setCursor(20, 5);
  display.print("set hour:");
  display.print(tclock.tc_hour);
  display.setCursor(20, 20);
  display.print("set min:");
  display.print(tclock.tc_min);
  display.setCursor(20, 35);
  display.print("set interval:");
  display.print(tclock.tc_interval);
  display.setCursor(20, 50);
  display.print("set state:");
  if(tclock.tc_enable)
    display.print("on");
  else
    display.print("off");  
  switch(clockItem)
  {
    case 0: display.drawRect(5, 8, 10, 2, WHITE); break;
    case 1: display.drawRect(5, 23, 10, 2, WHITE); break;
    case 2: display.drawRect(5, 38, 10, 2, WHITE); break;
    case 3: display.drawRect(5, 53, 10, 2, WHITE); break;
  }
}

void audioSetPage()
{
  pageSet(audioP);
  display.setCursor(20, 15);
  display.print("Boot Sound:");
  if(bootTone) display.print("on"); else display.print("off");
  display.setCursor(20, 35);
  display.print("Key Sound:");
  if(keyTone) display.print("on"); else display.print("off");
  if(audioItem == 0)
    display.drawRect(5, 17, 10, 2, WHITE);
  else
    display.drawRect(5, 37, 10, 2, WHITE);
  
}

void ringBellPage()
{
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(30, 15);
  display.drawBitmap(40, 15, clock_page_img, CLOCK_IMG_WIDTH, CLOCK_IMG_HEIGHT, WHITE); //49 * 36 //宽高
  display.display();
}

void pageSet(int page)
{
  if(keyRead())
  {
    delay(65);
    switch(keyRead())
    {
      case UP:
        audioPlay(500);
        switch(page)
        {
          case mainP:
            break;
          case itemP:
            item = item <= 0 ? itemCount - 1 : item-=1;
            break;
          case timeP:
            switch(timeItem)
            {
              case 0:
                if(tdata.td_hour < 23) tdata.td_hour += 1; else tdata.td_hour = 0;
                break;
              case 1:
                if(tdata.td_min < 59) tdata.td_min += 1; else tdata.td_min = 0;
                break;
              case 2:
                if(tdata.td_sec < 59) tdata.td_sec += 1; else tdata.td_sec = 0;
                break;  
            }
            break;
          case dateP:
            switch(timeItem)
            {
              case 0:
                if(tdata.td_date < 31) tdata.td_date += 1; else tdata.td_date = 1;
                break;
              case 1:
                if(tdata.td_month < 12) tdata.td_month += 1; else tdata.td_month = 1;
                break;
              case 2:
                if(tdata.td_year < 99) tdata.td_year += 1; else tdata.td_year = 0;
                break;  
            }
            break;
          case dowP:
            if(tdata.td_dow < 7) tdata.td_dow += 1; else tdata.td_dow = 1;
            break;
          case clockP:
            if(clockItem > 0) clockItem -= 1; else clockItem = 3;
            break;
          case audioP:
            audioItem = 0;
            break;
        }
        break;
      case DOWN:
        audioPlay(500);
        switch(page)
        {
          case mainP:
            break;
          case itemP:
            item = item >= itemCount - 1 ? 0 : item+=1;
            break;
          case timeP:
            switch(timeItem)
            {
              case 0:
                if(tdata.td_hour > 0) tdata.td_hour -= 1; else tdata.td_hour = 23;
                break;
              case 1:
                if(tdata.td_min > 0) tdata.td_min -= 1; else tdata.td_min = 59;
                break;
              case 2:
                if(tdata.td_sec > 0) tdata.td_sec -= 1; else tdata.td_sec = 59;
                break;  
            }
            break;
          case dateP:
            switch(timeItem)
            {
              case 0:
                if(tdata.td_date > 1) tdata.td_date -= 1; else tdata.td_date = 31;
                break;
              case 1:
                if(tdata.td_month > 1) tdata.td_month -= 1; else tdata.td_month = 12;
                break;
              case 2:
                if(tdata.td_year > 0) tdata.td_year -= 1; else tdata.td_year = 99;
                break;  
            }
            break;
          case dowP:
            if(tdata.td_dow > 1) tdata.td_dow -= 1; else tdata.td_dow = 7;
            break;
          case clockP:
            if(clockItem < 3) clockItem += 1; else clockItem = 0;
            break;
          case audioP:
            audioItem = 1;
            break;
        }
        break;
      case LEFT:
        audioPlay(500);
        switch(page)
        {
          case mainP:
            break;
          case itemP:
            break;
          case timeP: case dateP:
            if(timeItem > 0) timeItem -= 1;
            break;
          case clockP:
            switch(clockItem)
            {
              case 0:
                if(tclock.tc_hour > 0) tclock.tc_hour -= 1; else tclock.tc_hour = 23;
                break;
              case 1:
                if(tclock.tc_min > 0) tclock.tc_min -= 1; else tclock.tc_min = 59;
                break;
              case 2:
                if(tclock.tc_interval > 5) tclock.tc_interval -= 1; else tclock.tc_interval = 30;
                break;
              case 3:
                tclock.tc_enable ^= 1;
                break;  
            }
            break;
          case audioP:
            break;
        }
        break;
      case RIGHT:
        audioPlay(500);
        switch(page)
        {
          case mainP:
            break;
          case itemP:
            break;
          case timeP: case dateP:
            if(timeItem < 2) timeItem += 1;
            break;
          case clockP:
            switch(clockItem)
            {   
              case 0:
                if(tclock.tc_hour < 23) tclock.tc_hour += 1; else tclock.tc_hour = 0;
                break;
              case 1:
                if(tclock.tc_min < 59) tclock.tc_min += 1; else tclock.tc_min = 0;
                break;
              case 2:
                if(tclock.tc_interval < 30) tclock.tc_interval += 1; else tclock.tc_interval = 5;
                break;
              case 3:
                tclock.tc_enable ^= 1;
                break;  
            }
            break;
          case audioP:
            break;
        }
        break;
      case BB:
        audioPlay(500);
        switch(page)
        {
          case mainP:
            break;
          case itemP:
            item = 0;
            pageNum -= 1;
            break;
          case timeP: case dateP: case dowP: case audioP:
            pageNum -= (item + 1);
            timeEnable = true;
            break;
          case clockP:
            pageNum -= (item + 1);
            EEPROM.write(addrAlarms, tclock.tc_enable);
            EEPROM.write(addrAlarms + 1, tclock.tc_hour);
            EEPROM.write(addrAlarms + 2, tclock.tc_min);
            EEPROM.write(addrAlarms + 3, tclock.tc_interval);
            //重新读取设置
            initClock(&tclock);
            timeEnable = true;
            break;
        }
        break;
      case AA:
        audioPlay(500);
        switch(page)                                                                                                                                                                                                                                                                                                          
        {
          case mainP:
            pageNum += 1;
            initPage();
            break;
          case itemP:
            if(pageNum <= pageNumMax) pageNum = 2 + item;
            timeEnable = false;
            initPage();
            return;
            break;
          case timeP:
            pageNum -= (item + 1);
            timeEnable = true;
            //set
            rtc.setTime(tdata.td_hour, tdata.td_min, tdata.td_sec);
            break;
          case dateP:
            pageNum -= (item + 1);
            timeEnable = true;
            //set
              rtc.setDate(tdata.td_date, tdata.td_month, (tdata.td_year + 2000));
            break;
          case dowP:
            pageNum -= (item + 1);
            rtc.setDOW(tdata.td_dow);
            break;
          case clockP:
            break;
          case audioP:
            if(audioItem == 0)
            {
              bootTone ^= 1;
              EEPROM.write(addrBootTone, bootTone);
            }
            else 
            {
              keyTone ^= 1;
              EEPROM.write(addrKeyTone, keyTone);
            }
            break;
        }
        break;
    }
  }
}

void clockDisplay()
{
  display.drawBitmap(70, 50, logo_clock_bmp, LOGO_CLOCK_WIDTH, LOGO_CLOCK_HEIGHT, WHITE);
  if((tdata.td_hour == tclock.tc_hour) && (tdata.td_min == tclock.tc_min)) //判断时分相等
  {
    int i = 0;
    ringBellPage();
    //播放闹铃
    while(keyRead() != BB)
    { 
      tone(BUZZERPIN, pgm_read_word_near(tune + i));
      delay(240 * (float)pgm_read_word_near(durt + i) / 10);
      noTone(BUZZERPIN);
      if(i < length)
      {
        i++;
      }
      else
      {
        delay(1000); //stop 1s
        i = 0;
      }
    }
    tclock.tc_min += tclock.tc_interval;
    if(tclock.tc_min > 59)
    {
      tclock.tc_min = 0;
      tclock.tc_enable ^= 1;
    }
  }
}

void initPage()
{
  display.clearDisplay();
  display.display();
  display.setTextColor(WHITE);
  display.setTextSize(1);
}

void timeDisplay()
{
  if(tdata.td_hour < 10)
  {
    display.print("0");
    display.print(tdata.td_hour);
  }
  else
    display.print(tdata.td_hour);
  display.print(":");
  if(tdata.td_min < 10)
  {
    display.print("0");
    display.print(tdata.td_min);
  }
  else
    display.print(tdata.td_min);
  display.print(":");
  if(tdata.td_sec < 10)
  {
    display.print("0");
    display.print(tdata.td_sec);
  }
  else
    display.print(tdata.td_sec);
}

void dateDisplay()
{
  if(tdata.td_date < 10)
  {
    display.print("0");
    display.print(tdata.td_date);
  }
  else
    display.print(tdata.td_date);
  display.print("/");
  if(tdata.td_month < 10)
  {
    display.print("0");
    display.print(tdata.td_month);
  }
  else
    display.print(tdata.td_month);
  display.print("/");
  if(tdata.td_year < 10)
  {
    display.print("0");
    display.print(tdata.td_year);
  }
  else
    display.print(tdata.td_year);
}
