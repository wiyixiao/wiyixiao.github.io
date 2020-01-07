/**
 * G02
 * 2019/8/13
 * Aries.hu
 */

 //相关头文件
#include "global.h"
#include "modul.h"
#include "menu.h"

#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

U8G2_SSD1306_128X64_NONAME_1_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);

//按键定义
int keys[6] = { A6, A2, A7, A3, A0, A1 };

//相关菜单定义
const char page0[][20] PROGMEM = { "Name:G02","Enter","Options","Score","About" }; //主菜单显示
const char page1[][20] PROGMEM = { "Home","Music:ON","Music:OFF","Easy","Nornal","Hard" }; //选项菜单

//页面事件定义
void(*PageKeyFunc[][3])()
{
	{keyUpFunc, keyDownFunc, mainAAFunc},
	{ keyUpFunc,keyDownFunc,optionAAFunc }
};

//页面选项处理函数定义
void(*mainPageFunc[])() = { nameFunc,enterFunc,optionFunc,scoreFunc,aboutFunc };
void(*optionPageFunc[])() = { homeFunc, optionFunc, musicOffFunc,easyFunc,nornalFunc,hardFunc };

GLed led(11);
GKey key(keys);
GBuzzer buzzer(7);
GMenu menu(&u8g2);

//初始化
void setup() {
	Serial.begin(9600);
	buzzer.audioPlay(a, 200);
	led.setLevel(20);
	u8g2.begin();

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

}
void enterFunc()
{

	gameStart();
}
void optionFunc()
{

	menu.enterPage(option);
}
void scoreFunc()
{

}
void aboutFunc()
{

}


//定义Option页面选项处理函数
void homeFunc()
{

	menu.enterPage(main);
}
void musicOnFunc()
{

}
void musicOffFunc()
{

}
void easyFunc()
{

}
void nornalFunc()
{

}
void hardFunc()
{

}

/***************************************Game************************************************/
#define W (u8g2.getWidth()) //128 //屏幕宽度 
#define H (u8g2.getHeight()) //64 //屏幕高度
#define INFO_PANEL_W 32		//分数面板宽度
#define ACTIVE_WIN_W (W - INFO_PANEL_W) //坦克活动面板宽度
#define TANK_SIZE 8		//坦克尺寸8*8

static const PROGMEM uint8_t tankUp[8] = { 0x18,0x18,0xDB,0xFF,0xFF,0xFF,0xFF,0xC3 };			//上
static const PROGMEM uint8_t tankDown[8] = { 0xC3,0xFF,0xFF,0xFF,0xFF,0xDB,0x18,0x18 };		//下
static const PROGMEM uint8_t tankLeft[8] = { 0x3F,0x3F,0x1E,0xFE,0xFE,0x1E,0x3F,0x3F };		//左
static const PROGMEM uint8_t tankRight[8] = { 0xFC,0xFC,0x78,0x7F,0x7F,0x78,0xFC,0xFC };	//右

//static const PROGMEM uint8_t tankBitMap[][8] = {
//	{ 0x18,0x18,0xDB,0xFF,0xFF,0xFF,0xFF,0xC3 }, //UP
//	{ 0xC3,0xFF,0xFF,0xFF,0xFF,0xDB,0x18,0x18 }, //DOWN
//	{ 0x3F,0x3F,0x1E,0xFE,0xFE,0x1E,0x3F,0x3F }, //LEFT
//	{ 0xFC,0xFC,0x78,0x7F,0x7F,0x78,0xFC,0xFC } //RIGHT
//};

/*******************************************坦克类************************************************/
typedef enum Direction {
	DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT
}Dir;

class Tank
{
public:
	Tank(uint16_t x, uint16_t y, Dir dir);
	~Tank() {};

	void
		init(),
		move(Dir dir),
		setVelocity(uint16_t v),
		clearActiveWin(),
		death();

private:
	struct TankInfo
	{
		uint16_t x; //x坐标
		uint16_t y; //y坐标
		Dir dir; //方向
		uint16_t v; //速度
		bool isLive; //是否死亡
	};
	TankInfo *info;
};

Tank::Tank(uint16_t x, uint16_t y, Dir dir)
{
	info->x = x;
	info->y = y;
	info->dir = dir;
}

void Tank::move(Dir dir)
{
	if (info->x <= 0)
		info->x = 1;
	else if (info->x > (ACTIVE_WIN_W - TANK_SIZE))
		info->x = (ACTIVE_WIN_W - TANK_SIZE);
	if (info->y <= 0)
		info->y = 1;
	else if (info->y > (H - TANK_SIZE))
		info->y = (H - TANK_SIZE);

	//oled.fillRect(info->x, info->y, (info->x + TANK_SIZE), (info->y + TANK_SIZE), BLACK);
	//oled.display();

	switch (dir)
	{
	case DIR_UP:
		info->y -= 1;
		//oled.drawBitmap(info->x, info->y, tankUp, TANK_SIZE,TANK_SIZE, WHITE);
		break;
	case DIR_DOWN:
		info->y += 1;
		//oled.drawBitmap(info->x, info->y, tankDown, TANK_SIZE, TANK_SIZE, WHITE);
		break;
	case DIR_LEFT:
		info->x -= 1;
		//oled.drawBitmap(info->x, info->y, tankLeft, TANK_SIZE, TANK_SIZE, WHITE);
		break;
	case DIR_RIGHT:
		info->x += 1;
		//oled.drawBitmap(info->x, info->y, tankRight, TANK_SIZE, TANK_SIZE, WHITE);
		break;
	}
	delayMicroseconds(info->v);
	//oled.display();
}

void Tank::setVelocity(uint16_t v)
{
	info->v = v;
}

void Tank::clearActiveWin()
{
	//u8g2.fillRect(1, 1, ACTIVE_WIN_W-1, H-2, BLACK); //减 1，避免擦除边框线
	//oled.display();
}

void Tank::death()
{
	//显示爆炸特效

	//置死亡状态
	info->isLive = false;
}

void Tank::init()
{
	clearActiveWin();
	//u8g2.drawBitmap(info->x,info->y,tankUp,TANK_SIZE,TANK_SIZE);
	u8g2.display();
}

/*******************************************。。。。************************************************/
//Tank myTank((ACTIVE_WIN_W - TANK_SIZE) / 2, (H - TANK_SIZE) / 2,DIR_UP);
Tank myTank(10, 10, DIR_UP);

void gameInit()
{
	u8g2.clear();
	//绘制活动框
	//u8g2.drawBox(0, 0, ACTIVE_WIN_W, H);
	//绘制信息面板框
	//u8g2.drawBox(ACTIVE_WIN_W, 0, INFO_PANEL_W, H);
	//初始化我的坦克
	myTank.setVelocity(5);
	myTank.init();
	//初始化敌人坦克

}

void gameStart()
{
	gameInit();
	//初始化完成，开始按键控制
	for (;;)
	{
		int k = key.press();

		switch (k)
		{
		case UP:
			myTank.move(DIR_UP);
			break;
		case DOWN:
			myTank.move(DIR_DOWN);
			break;
		case LEFT:
			myTank.move(DIR_LEFT);
			break;
		case RIGHT:
			myTank.move(DIR_RIGHT);
			break;
		}

		//设置退出条件
		if (k == BB)
		{
			menu.enterPage(main);
			break;
		}

		u8g2.display();

	}
}
