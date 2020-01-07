/*
 *  AT_0
 * 主控: Arduino nano
 * 时钟模块: DS1302
 * 温湿度模块: DHT11
 * 音频模块: 蜂鸣器
 * 显示模块: OLED (I2C)
 */
#include <Arduino.h>
#include <Wire.h>
#include <EEPROM.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT11.h>
#include <DS1302.h>

#define KEY_NONE 0
#define DHT11PIN 2
#define BUZZERPIN 3
#define OLED_RESET 4

dht11 DHT11;
DS1302 rtc(5, 6, 7); //RST, DAT, CLK
Adafruit_SSD1306 display(OLED_RESET);

typedef uint8_t Key, Page;
typedef uint16_t Td, Tc, Ht;
enum addr{addrStart, addrBootTone, addrKeyTone, addrAlarms}; 

Key key_u = A6; //up
Key key_d = A3; //down
Key key_l = A7; //left
Key key_r = A2; //down
Key key_a = A1; //a
Key key_b = A0; //b

Key keyCodeFirst = KEY_NONE;
Key keyCodeSecond = KEY_NONE;
Key keyCode = KEY_NONE;

Page pageNum = 0;
Page pageNumMax = 10;

typedef struct htData{
  float ht_t; //温度
  float ht_h; //适度
}ht;
ht htdata;

typedef struct tData{
  Td td_year; //年
  Td td_month; //月
  Td td_date; //日
  Td td_hour; //时
  Td td_min; //分
  Td td_sec; //秒
  Td td_dow; //星期
}td;
td tdata;

typedef struct tClock{
  boolean tc_enable;
  Tc tc_hour;
  Tc tc_min;
  Tc tc_interval; //贪睡时间
}tc;
tc tclock;

boolean initSet = false;
boolean timeEnable = true;
boolean bootTone = false;
boolean keyTone = false;

int audioStart[3] = {1200, 600, 1000};
enum keyVal{UP = 1, DOWN, LEFT, RIGHT, AA, BB};

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

void dhtRead(ht * pht);
void rtcRead(td * ptd);
void initClock(tc * ptc);
void mainPage(ht * pht);

void setup()
{
  Serial.begin(9600);
  Wire.begin();
  pinMode(key_u, INPUT);
  pinMode(key_d, INPUT);
  pinMode(key_l, INPUT);
  pinMode(key_r, INPUT); 
  pinMode(key_a, INPUT);
  pinMode(key_b, INPUT);
  pinMode(DHT11PIN, INPUT);
  pinMode(BUZZERPIN, OUTPUT);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3c);
  display.clearDisplay();
  while(1);
  //检测EEPROM存储状态
  initSet = EEPROM.read(addrStart);
  if(initSet) initSetAll();
  //开关变量赋值
  bootTone = EEPROM.read(addrBootTone);
  keyTone = EEPROM.read(addrKeyTone);
  //读取闹钟设置值
  initClock(&tclock);
  //开机动画
  animationStart();
  if(bootTone) audioPlay(3, audioStart, 200);
  display.clearDisplay();
  display.display();
  
  // 设置时间后, 需要注释掉设置时间的代码,并重新烧录一次. 以免掉电重新执行setup中的时间设置函数.
  rtc.halt(false);
  rtc.writeProtect(false);
//  rtc.setDOW(SATURDAY);
//  rtc.setTime(00, 02, 00);
//  rtc.setDate(31, 12, 2016);
//  rtc.writeProtect(true);
}

void loop()
{
  /************************/
  //keyValPrint();
  /************************/
  dhtRead(&htdata);
  if(timeEnable) rtcRead(&tdata);
  /************************/
  switch(pageNum)
  {
    case 0: mainPage(&htdata); break;
    case 1: itemPage(); break;
    case 2: timeSetPage(); break;
    case 3: dateSetPage(); break;
    case 4: dowSetPage(); break;
    case 5: clockSetPage(); break;
    case 6: audioSetPage(); break;
  }
  display.display();
  display.clearDisplay();
  delay(50);
}

//+++++++++++++++++++++++++++++++++++++
int toDigital(uint8_t pin)
{
  if(analogRead(pin) > 300)
    return 1;
  else
    return 0;
}

int keyRead()
{
  keyCodeSecond = keyCodeFirst;
  if(!toDigital(key_u))
    keyCodeFirst = UP;
  else if(!toDigital(key_d))
    keyCodeFirst = DOWN;
  else if(!toDigital(key_l))
    keyCodeFirst = LEFT;
  else if(!toDigital(key_r))
    keyCodeFirst = RIGHT;
  else if(!toDigital(key_a))
    keyCodeFirst = AA;
  else if(!toDigital(key_b))
    keyCodeFirst = BB;
  else
    keyCodeFirst = KEY_NONE;
    
  if(keyCodeSecond == keyCodeFirst)
    keyCode = keyCodeFirst;
  else
    keyCode = KEY_NONE;
  
  if(keyCode > 0)
    return keyCode;
   else
    return -1;
}

void keyValPrint()
{
  Serial.print("\t");
  Serial.print(toDigital(key_u)); Serial.print("\n\n\n");
  Serial.print(toDigital(key_l)); Serial.print("\t\t");
  Serial.print(toDigital(key_r)); Serial.print("\t\t");
  Serial.print(toDigital(key_a)); Serial.print("\t");
  Serial.print(toDigital(key_b)); Serial.print("\n\n\n\t");
  Serial.print(toDigital(key_d)); Serial.print("\n\n");
  for(int i=0;i<60;i++) Serial.print("*");
  Serial.print("\n");
  delay(500);
}

void audioPlay(int frequency){if(keyTone){tone(BUZZERPIN, frequency); delay(50); noTone(BUZZERPIN);}}
void audioPlay(int num, int * pf, int time)
{
  int i;
  for(i = 0;i<num;i++)
  {
    tone(BUZZERPIN, *(pf + i));
    delay(time);
  }
  noTone(BUZZERPIN);
}

void animationStart(void) 
{
  for (int16_t i=0; i<display.height(); i+=2) {
    display.drawCircle(display.width()/2, display.height()/2, i, WHITE);
    display.display();
  }
  display.display();
}

void dhtRead(ht * pht)
{
  int chk = DHT11.read(DHT11PIN);
  
  /*
  switch (chk)
  {
    case DHTLIB_OK: 
                Serial.println("OK"); 
                break;
    case DHTLIB_ERROR_CHECKSUM: 
                Serial.println("Checksum error"); 
                break;
    case DHTLIB_ERROR_TIMEOUT: 
                Serial.println("Time out error"); 
                break;
    default: 
                Serial.println("Unknown error"); 
                break;
  }
  */
  
  pht -> ht_t = (float)DHT11.temperature;
  pht -> ht_h = (float)DHT11.humidity;
  
  /*
  Serial.print("temperature:\b");
  Serial.print(dht_t);
  Serial.print("\bhumidity:\b");
  Serial.println(dht_h);
  */
}

void rtcRead(td * ptd)
{
  Time t = rtc.getTime();
  ptd -> td_year =  t.year - 2000; //年份减2000换算为小于100数值
  ptd -> td_month = t.mon;
  ptd -> td_date = t.date;
  ptd -> td_hour = t.hour;
  ptd -> td_min = t.min;
  ptd -> td_sec = t.sec;
  ptd -> td_dow = t.dow;
}

void initSetAll()
{
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 20);
  display.print("Please wait for device reset...");
  for (int i = 0 ; i < EEPROM.length() ; i++) {
    EEPROM.write(i, 0);
  }
  display.clearDisplay();
}
void initClock(tc * ptc)
{
  ptc->tc_enable = EEPROM.read(addrAlarms);
  ptc->tc_hour = EEPROM.read(addrAlarms + 1);
  ptc->tc_min = EEPROM.read(addrAlarms + 2);
  Tc _interval = EEPROM.read(addrAlarms + 3);
  ptc->tc_interval = _interval == 0 ? 5 : _interval;
}
