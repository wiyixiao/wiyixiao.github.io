/**
 * G02
 * 测试
 * 2019/8/12
 * Aries.hu
 */

//相关头文件
#include "global.h"
#include "modul.h"
#include "menu.h"

#define OLED_RESET 4

//按键定义
int keys[6] = {A6, A2, A7, A3, A0, A1};

//相关菜单定义
const char page0[][20] PROGMEM = { "Name:G02-Maze","Enter","Options","Score","About" }; //主菜单显示
const char page1[][20] PROGMEM = { "Home","Music:ON","Music:OFF","Easy","Nornal","Hard"}; //选项菜单

//页面事件定义
void(*PageKeyFunc[][3])()
{
	{keyUpFunc,keyDownFunc,mainAAFunc},
	{ keyUpFunc,keyDownFunc,optionAAFunc }
};

//页面选项处理函数定义
void(*mainPageFunc[])() = { nameFunc,enterFunc,optionFunc,scoreFunc,aboutFunc };
void(*optionPageFunc[])() = { homeFunc, musicOnFunc, musicOffFunc,easyFunc,nornalFunc,hardFunc };

GLed led(11);
GKey key(keys);
GBuzzer buzzer(7);
GOled oled;
GMenu menu(&oled);

//初始化
void setup() {
  Serial.begin(9600);
  buzzer.audioPlay(a, 200);
  led.setLevel(20);
  oled.begin();

  //创建菜单
  menu.creadMenu(page0, main, 5);
  menu.creadMenu(page1, option, 6);
}

//主循环
void loop()
{
	KeyMap val = (KeyMap)key.press();
	if (val == UP || val == DOWN || val == AA)
	{
		PageKeyFunc[menu.getPage()][keySet(val)]();
	}
}

/***************************************以下为页面中事件处理函数************************************************/
int keySet(KeyMap val)
{
	switch (val)
	{
	case UP: return 0; break;
	case DOWN: return 1; break;
	case AA: return 2; break;  //设定为事件函数的索引
	//case BB: return 3; break;
	//case LEFT: return 4; break;
	//case RIGHT: return 5; break;
	}
}

//上下键公用函数
void keyUpFunc()
{
	menu.itemChanged(-1);
}

void keyDownFunc()
{
	menu.itemChanged(1);
}

//每个页面的AA键处理函数
void mainAAFunc()
{
	mainPageFunc[menu.getItem()]();
}

void optionAAFunc()
{
	optionPageFunc[menu.getItem()]();
}
//每个页面的BB键处理函数
//暂时不设置

/***************************************需要做的事情************************************************/
//定义主页面每个选项处理函数
void nameFunc()
{
	Serial.println(F("Name:G02-Maze"));
}
void enterFunc()
{
	Serial.println(F("Enter"));
	myGame();
}
void optionFunc()
{
	Serial.println(F("Options"));
	menu.enterPage(option);
}
void scoreFunc()
{
	Serial.println(F("Score"));
}
void aboutFunc()
{
	Serial.println(F("About"));
}


//定义Option页面选项处理函数
void homeFunc()
{
	Serial.println(F("Home"));
	menu.enterPage(main);
}
void musicOnFunc()
{
	Serial.println(F("Music:ON"));
}
void musicOffFunc()
{
	Serial.println(F("Music:OFF"));
}
void easyFunc()
{
	Serial.println(F("Easy"));
}
void nornalFunc()
{
	Serial.println(F("Nornal"));
}
void hardFunc()
{
	Serial.println(F("Hard"));
}


void myGame()
{
	oled.clear();
	oled.setCursor(10, 10);
	oled.print("DRAW SHAPE...");
	oled.display();

	//类似于又进入一个loop循环，退出循环时指定页面ID即可，menu.h只提供菜单功能，应该是这样子。。。
	for(;;)
	{
		int i = key.press();

		switch (i)
		{
		case UP:
			oled.clear();
			oled.setCursor(10, 10);
			oled.drawCircle(64, 32, 20, WHITE); //绘制圆
			break;
		case DOWN:
			oled.clear();
			oled.setCursor(10, 10);
			oled.drawRect(16, 16, 96, 32, WHITE);
			break;
		}

		//设置退出条件
		if (i == BB)
		{
			menu.enterPage(main);
			break;	
		}

		delay(10);
		oled.display();
	
	}
}
