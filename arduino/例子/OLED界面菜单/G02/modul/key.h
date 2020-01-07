/**
 * 按键头文件
 */
#ifndef __KEY_H_
#define __KEY_H_

#include "../global.h"

#define KEY_NONE 0

typedef uint8_t Key;

class GKey
{
  private:
    typedef struct
    {
      Key key_array[6]; //0,1,2,3,4,5==>up,down,left,right,aa,bb
      Key key_map[6];
      Key keyCodeFirst;
      Key keyCodeSecond;
      Key keyCode;
    }Keys;
    
    Keys keys;
    Keys * p = &keys;  
	int
		curKey,
		lastKey,
		keyNum;
	unsigned long
		pressTime,
		longPressTime;
	boolean
		keyPress;

    int toDigital(Key key)
    {
      static int val = 300;
      if(analogRead(key) > val)
        return 1;
      else;
        return 0;
    }

	int keyRead(void)
	{
		p->keyCodeSecond = p->keyCodeFirst;
		for (int i = 0; i<keyNum; i++)
		{
			if (!toDigital(*(p->key_array + i)) && !keyPress) //触发
			{
				if (millis() - pressTime > 120)
				{
					keyPress = true;
					pressTime = millis();
					goto end;
				}
				else
				{
					p->keyCodeFirst = p->key_map[i];
					break;
				}

			}
			else
			{
				p->keyCodeFirst = KEY_NONE;
			}
		}

		if (p->keyCodeSecond == p->keyCodeFirst)
		{
			p->keyCode = p->keyCodeFirst;
		}
		else
			p->keyCode = KEY_NONE;

		end:if (keyPress)
		{
			keyPress = false;
			pressTime = millis();
			if (p->keyCode > 0)
				return p->keyCode;
		}
		return KEY_NONE;
	}
    
  public:
    GKey(int * keyArray) 
    {
		keyNum = sizeof(p->key_array) / sizeof(p->key_array[0]);
		for(int i=0;i<keyNum;i++)
		{
			*(p->key_array+i) = *(keyArray+i);
			*(p->key_map+i) = (i+1);
			//设置引脚模式
			pinMode(*(p->key_array+i), INPUT);
		}  
		p->keyCodeFirst = KEY_NONE;
		p->keyCodeSecond = KEY_NONE;
		p->keyCode = KEY_NONE;

		pressTime = millis();
		keyPress = false;
    };
    
    ~GKey(){};

	int press()
	{
		return keyRead();
	}

	int click()
	{
	
	}
};

#endif
