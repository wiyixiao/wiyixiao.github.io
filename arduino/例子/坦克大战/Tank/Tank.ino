/*
 * 坦克大战游戏
 * 2019/8/18
 * Aries.hu
 * Arduino Nano
 * ::::::::::::::
 * LED、Arduino Nano、按键、蜂鸣器(未使用暂时)、OLED(I2C / 128x64)
 * 仅供参考，哈哈。。
 */

#include "global.h"
#include <U8g2lib.h>
#include <Wire.h>
#include "led.h"
#include "key.h"
#include "buzzer.h"

//#define DEBUG
//128 //屏幕宽度 
#define W (u8g2.getWidth()) 
//64 //屏幕高度
#define H (u8g2.getHeight()) 
//分数面板宽度//默认高度为屏幕高度
#define INFO_PANEL_W 32		
//坦克活动面板宽度//默认高度为屏幕高度
#define ACTIVE_WIN_W (W - INFO_PANEL_W) 
//坦克尺寸8*8
#define TANK_SIZE 8	
//子弹尺寸2*2
#define SHOT_SIZE 2
//屏幕上显示的最多敌方坦克数量 //6个//多了太挤。。。
#define ENEMY_TANK_MAX 4
//方向转换时间间隔
#define DIR_CHANGE_TIME_INTERVAL 1000
//坦克子弹发射时间间隔
#define SHOT_TIME_INTERVAL 1000
//最大生命数
#define MAX_LIFE 10
//坦克与子弹速度
#define MAX_V 1000
uint16_t
myTankV = 1000, 
enemyTankV = 15,
shotV = 55;
//分数
int score = 0;
//坦克最大复活次数(255)
uint8_t life = MAX_LIFE;
//按键对应引脚
int keys[6] = { A6, A2, A7, A3, A0, A1 }; //上、下、左、右、右上(AA)、右下(BB)
//第一次生成坦克
bool firstCreate = true;

GLed led(11);	//led灯
GKey key(keys); //按键
GBuzzer buzzer(7); //蜂鸣器//未使用
U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

typedef enum Direction {
	DIR_UP=0, DIR_DOWN, DIR_LEFT, DIR_RIGHT
}Dir;

static const PROGMEM uint8_t tankBitMap[][8] = {
	{ 0x18,0x18,0xDB,0xFF,0xFF,0xFF,0xFF,0xC3 }, //UP
	{ 0xC3,0xFF,0xFF,0xFF,0xFF,0xDB,0x18,0x18 }, //DOWN
	{ 0xFC,0xFC,0x78,0x7F,0x7F,0x78,0xFC,0xFC }, //LEFT
	{ 0x3F,0x3F,0x1E,0xFE,0xFE,0x1E,0x3F,0x3F }, //RIGHT
};
static const PROGMEM uint8_t bombBmp[8] = { 0x95,0x4A,0x29,0x94,0x2A,0x41,0x2A,0x95 }; //BOMB
static const PROGMEM uint8_t heartBmp[8] = { 0x00,0x00,0x24,0x7E,0x7E,0x3C,0x18,0x00 }; //HEART

typedef enum PageName
{
	START_PAGE, GAME_PAGE, END_PAGE, SETTING_PAGE
}PN;

PN page;
uint8_t menu_box_y = 20;
void(*pageFunc[])(KeyMap key) = { startPage,gamePage,endPage,settingPage };

void setup()
{
	Serial.begin(9600);
	led.setLevel(15); //设置Led灯亮度（0 ~ 255） //模拟量输出
	u8g2.begin();
}

void loop()
{
	u8g2.firstPage();
	do
	{
		pageFunc[(int)page]((KeyMap)key.press());

	} while (u8g2.nextPage());
}

/*******************************************页面************************************************/
void drawBorder(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
	w -= 1;
	h -= 1;
	u8g2.drawHLine(x, y, w);
	u8g2.drawHLine(x, y + h, w);
	u8g2.drawVLine(x, y, h);
	u8g2.drawVLine(x + w, y, y+h);
}

void startPage(KeyMap k)
{
	switch (k)
	{
	case KEY_UP:
		menu_box_y = 20;
		break;
	case KEY_DOWN:
		menu_box_y = 44;
		break;
	case KEY_AA:
		if (menu_box_y < 30)
		{
			page = GAME_PAGE;
		}
		else
		{
			page = SETTING_PAGE;
		}
		break;
	}

	u8g2.setFont(u8g2_font_6x12_me);
	u8g2.setDrawColor(1);
	u8g2.drawStr(25, 24, "Start Game");
	u8g2.drawStr(35, 48, "Option");
	u8g2.drawBox(100, menu_box_y, 8, 4);

	u8g2.sendBuffer();
}

void gamePage(KeyMap k)
{

	gameStart(k);

	u8g2.sendBuffer();

}

void endPage(KeyMap k)
{
	unsigned long t = millis() + 2000;

	while (millis() < t)
	{
		u8g2.firstPage();
		do
		{
			u8g2.drawStr(35, 24, "Game Over");
			u8g2.drawStr(10, 42, "Return in 2 seconds");
			u8g2.sendBuffer();
		} while (u8g2.nextPage());
	
	}
	page = START_PAGE;
}

void settingPage(KeyMap k)
{
	//可在此添加需要的设置
	//音效、、存储设置、游戏参数、、等等
	u8g2.drawStr(10, 16, "You can perfect it!");
	u8g2.drawStr(10, 32, "Press the <BB> key");
	u8g2.drawStr(10, 48, "Exit!");
	u8g2.sendBuffer();

	if (k == KEY_BB)
	{
		page = START_PAGE;
	}
	
}
/*******************************************子弹类************************************************/

class Shot
{
public:
	Shot();
	void
		setPos(uint8_t x, uint8_t y,Dir dir),
		init(),
		death(),
		move();
	uint8_t
		getX(),
		getY();
	bool isAlive();
private:
	struct ShotInfo
	{
		uint8_t x; //x坐标
		uint8_t y; //y坐标
		uint8_t dir; //方向
		uint16_t v; //速度//默认为坦克速度
		bool isLive; //是否存在
	};
	ShotInfo info;
	unsigned long t;
};

uint8_t Shot::getX()
{
	return info.x;
}

uint8_t Shot::getY()
{
	return info.y;
}

bool Shot::isAlive()
{
	return info.isLive;
}

void Shot::init()
{
	info.isLive = true;
}

Shot::Shot()
{

}

void Shot::setPos(uint8_t x, uint8_t y, Dir dir)
{
	info.x = x;
	info.y = y;
	info.v = shotV;
	info.dir = dir;
	t = millis() + info.v;
}

void Shot::move()
{
	if (info.x <= 0 || info.x > (ACTIVE_WIN_W - TANK_SIZE) + 4 || info.y <= 0 || info.y > (H - TANK_SIZE) + 4)
		death();

	if (millis() > t)
	{
		switch (info.dir)
		{
		case DIR_UP:
			info.y -= 1;
			break;
		case DIR_DOWN:
			info.y += 1;
			break;
		case DIR_LEFT:
			info.x -= 1;
			break;
		case DIR_RIGHT:
			info.x += 1;
			break;
		}
		t = millis() + (MAX_V / info.v);

	}
	
	if (info.isLive == false)
		return;

	switch (info.dir)
	{
	case DIR_UP:
		u8g2.drawBox(info.x, info.y, SHOT_SIZE, SHOT_SIZE);
		break;
	case DIR_DOWN:
		u8g2.drawBox(info.x, info.y, SHOT_SIZE, SHOT_SIZE);
		break;
	case DIR_LEFT:
		u8g2.drawBox(info.x, info.y, SHOT_SIZE, SHOT_SIZE);
		break;
	case DIR_RIGHT:
		u8g2.drawBox(info.x, info.y, SHOT_SIZE, SHOT_SIZE);
		break;
	}

}

void Shot::death()
{
	info.isLive = false;
}

/*******************************************坦克类************************************************/
//每个坦克发射的子弹
typedef struct ShotListNode
{
	int id;
	Shot shot;
	ShotListNode *next;
}ShotListNode;

class Tank
{
public:
	Tank();
	Tank(uint8_t x, uint8_t y, uint16_t v, Dir dir);
	~Tank() {};
	void
		stop(),
		move(Dir dir),
		shot(),
		death(),
		displayShot(),
		setDir(Dir dir),
		setX(uint8_t x),
		setY(uint8_t y),
		init(uint8_t x, uint8_t y, uint16_t v, Dir dir);

	uint8_t
		getX(),
		getY();
	uint16_t
		getV();
	Dir
		getDir();
	bool
		isAlive();

	int
		getShotNum();

	ShotListNode *  
		getShotListNode();

private:
	struct TankInfo
	{
		uint8_t x; //x坐标
		uint8_t y; //y坐标
		Dir dir; //方向
		uint16_t v; //速度///暂定为时间间隔 us
		bool isLive; //是否死亡
		long shot_num; //子弹总数量
		ShotListNode *head;
	};
	TankInfo info;

	unsigned long t;
};

Tank::Tank()
{
	info.isLive = true;
}

void Tank::setX(uint8_t x)
{
	info.x = x;
}

void Tank::setY(uint8_t y)
{
	info.y = y;
}

uint8_t Tank::getX()
{
	return info.x;
}

uint8_t Tank::getY()
{
	return info.y;
}

uint16_t Tank::getV()
{
	return info.v;
}

int Tank::getShotNum()
{
	return info.shot_num;
}

void Tank::setDir(Dir dir)
{
	info.dir = dir;
}

Dir Tank::getDir()
{
	return info.dir;
}

ShotListNode * Tank::getShotListNode()
{
	return info.head; //返回子弹链表头节点
}


bool Tank::isAlive()
{
	return info.isLive;
}

Tank::Tank(uint8_t x, uint8_t y, uint16_t v, Dir dir)
{
	init(x, y, v, dir);
}

void Tank::move(Dir dir)
{
	info.dir = dir;

	if (millis() > t)
	{
		//到达时间，刷新位置
		switch (dir)
		{
		case DIR_UP:
			info.y -= 1;
			break;
		case DIR_DOWN:
			info.y += 1;
			break;
		case DIR_LEFT:
			info.x -= 1;
			break;
		case DIR_RIGHT:
			info.x += 1;
			break;
		}

		t = millis() + (MAX_V / info.v);
	}

	if (info.x <= 0)
		info.x = 1;
	else if (info.x > (ACTIVE_WIN_W - TANK_SIZE))
		info.x = (ACTIVE_WIN_W - TANK_SIZE);
	if (info.y <= 0)
		info.y = 1;
	else if (info.y > (H - TANK_SIZE))
		info.y = (H - TANK_SIZE);

	switch (dir)
	{
	case DIR_UP:
		u8g2.drawXBMP(info.x, info.y, TANK_SIZE, TANK_SIZE, tankBitMap[info.dir]);
		break;
	case DIR_DOWN:
		u8g2.drawXBMP(info.x, info.y, TANK_SIZE, TANK_SIZE, tankBitMap[info.dir]);
		break;
	case DIR_LEFT:
		u8g2.drawXBMP(info.x, info.y, TANK_SIZE, TANK_SIZE, tankBitMap[info.dir]);
		break;
	case DIR_RIGHT:
		u8g2.drawXBMP(info.x, info.y, TANK_SIZE, TANK_SIZE, tankBitMap[info.dir]);
		break;
	}
}

void Tank::death()
{
	//置死亡状态
	info.isLive = false;
	//显示爆炸效果
	u8g2.drawXBMP(info.x, info.y, TANK_SIZE, TANK_SIZE, bombBmp);
}

void Tank::init(uint8_t x, uint8_t y, uint16_t v, Dir dir)
{
	t = millis() + info.v;
	info.x = x;
	info.y = y;
	info.v = v;
	info.dir = dir;
	info.isLive = true;
	info.head = nullptr;
	info.shot_num = 0;
	u8g2.drawXBMP(info.x, info.y, TANK_SIZE, TANK_SIZE, tankBitMap[info.dir]);
}

void Tank::stop()
{
	u8g2.drawXBMP(info.x, info.y, TANK_SIZE, TANK_SIZE, tankBitMap[info.dir]);
}

//射击
void Tank::shot()
{
	//创建子弹
	Shot _shot;
	//初始化子弹
	_shot.init();
	//设置子弹位置及方向
	switch (info.dir)
	{
	case DIR_UP:
		_shot.setPos(info.x + 3, info.y + 2, info.dir);
		break;
	case DIR_DOWN:
		_shot.setPos(info.x + 3, info.y + 8, info.dir);
		break;
	case DIR_LEFT:
		_shot.setPos(info.x - 2, info.y + 3, info.dir);
		break;
	case DIR_RIGHT:
		_shot.setPos(info.x + 8, info.y + 3, info.dir);
		break;
	}
	//每发射一次，子弹数加一
	if (info.head == NULL)
	{
		info.head = new ShotListNode;
		//添加子弹
		info.head->shot = _shot;
		//设置子弹id
		info.head->id = info.shot_num;
		info.head->next = nullptr;

	}
	else
	{
	 	ShotListNode * pr = info.head;
		ShotListNode *pNew = new ShotListNode;

		pNew->shot = _shot;
		pNew->id = info.shot_num;
		pNew->next = nullptr;

		while (pr->next != NULL)
		{
			pr = pr->next;
		}
		pr->next = pNew;	
	}
	//子弹数量加一
	info.shot_num++;
}

//显示子弹
void Tank::displayShot()
{
	int delete_id = 0;
	ShotListNode * pr = info.head;
	while (pr != NULL)
	{
		//检测子弹是否死亡
		if (pr->shot.isAlive() == false)
		{
			//Serial.println(pr->id);
			delete_id = pr->id;
		}

		pr->shot.move();
		pr = pr->next;
	}

	//删除子弹
	ShotListNode * pb, *pf;
	pb = info.head;
	if (info.head == NULL)
	{
		return;
	}
	while (pb->id != delete_id && pb->next != NULL)
	{
		pf = pb;
		pb = pb->next;
	}
	if (pb->id == delete_id)
	{
		if (pb == info.head)
		{
			info.head = pb->next;
		}
		else
		{
			pf->next = pb->next;
		}
		free(pb);
		//if (delete_id == 0)
		//	return;
		//else
		//	info.shot_num--;
	}
	else
	{
		//没有找到要删除的节点
		;
	}
}

/*******************************************我的坦克************************************************/

Tank myTank((ACTIVE_WIN_W - TANK_SIZE) / 2, (H - TANK_SIZE) / 2, myTankV, DIR_UP);

void createMyTank()
{
	myTank.init((ACTIVE_WIN_W - TANK_SIZE) / 2, (H - TANK_SIZE) / 2, myTankV, DIR_UP);
}

/*******************************************敌方坦克************************************************/
uint8_t add_id;
//敌方坦克
struct EnemyTankListNode
{
	Tank tank;
	uint8_t id;
	EnemyTankListNode *next;
};
EnemyTankListNode * head = nullptr;

//获取当前敌方坦克数量
uint8_t returnEnemyNum()
{
	uint8_t num = 0;
	EnemyTankListNode * pr = head;
	while (pr != NULL)
	{
		pr = pr->next;
		num++;
	}
	return num;
}

//创建坦克
void creatEnemyTank()
{
	uint8_t num = returnEnemyNum();
	//Serial.println(num);
	//获取数量//小于最大数量时添加
	if (num < ENEMY_TANK_MAX)
	{
		//创建坦克
		Tank enemyTank;
		if (firstCreate)
		{
			//控制初始显示间距
			uint8_t gap, padding, x;
			padding = 10;
			gap = (ACTIVE_WIN_W - (padding * 2) - ENEMY_TANK_MAX * TANK_SIZE) / (ENEMY_TANK_MAX - 1);
			x = add_id == 0 ? padding : add_id * (TANK_SIZE + gap) + padding;

			enemyTank.init(x, 10, enemyTankV, DIR_DOWN);
			enemyTank.shot();
			firstCreate = !firstCreate; //false
#ifdef  DEBUG
			firstCreate = !firstCreate; //true
#endif //  DEBUG
		}
		else
		{
			//非调试状态时随机设置坦克创建的位置//会在反方向边缘随机出现新的坦克
			uint8_t x, y;
			Dir dir = myTank.getDir();
			switch (dir)
			{
			case DIR_UP:
				x = random(1, ACTIVE_WIN_W - TANK_SIZE);
				y = H - TANK_SIZE;
				break;
			case DIR_DOWN:
				x = random(1, ACTIVE_WIN_W - TANK_SIZE);
				y = 1;
				break;
			case DIR_LEFT:
				y = random(1, H - TANK_SIZE);
				x = ACTIVE_WIN_W - TANK_SIZE;
				break;
			case DIR_RIGHT:
				y = random(1, H - TANK_SIZE);
				x = 1;
				break;
			}
			enemyTank.init(x, y, enemyTankV, dir);
		}

		if(head == NULL)
		{
			head = new EnemyTankListNode;
			head->id = add_id;
			head->tank = enemyTank;
			head->next = nullptr;
		}
		else
		{
			EnemyTankListNode * pr = head;
			EnemyTankListNode * pNew = new EnemyTankListNode;

			pNew->id = add_id;
			pNew->tank = enemyTank;
			pNew->next = nullptr;

			while (pr->next != NULL)
			{
				pr = pr->next;
			}
			pr->next = pNew;
		}
		add_id++;

	}
	else
	{
		add_id = 0;
	}
}

//敌方坦克方向随机改变
unsigned long dir_change_t_interval = millis() + DIR_CHANGE_TIME_INTERVAL; 
void enmyTankChangeDir()
{	
	//超时换方向
	if (millis() > dir_change_t_interval)
	{
		EnemyTankListNode * p = head;
		while (p != NULL)
		{
			//Serial.print(p->tank.getX());
			//Serial.print(",");
			//Serial.println(p->tank.getY());
			p->tank.setDir((Dir)(random(0, 4)));
			p = p->next;
		}
		//Serial.println("-----------");
		dir_change_t_interval = millis() + DIR_CHANGE_TIME_INTERVAL;
	}
}

unsigned long shot_t_interval = millis() + SHOT_TIME_INTERVAL;
void enemyTankShot()
{
	//超时发射子弹
	if (millis() > shot_t_interval)
	{
		//Serial.println("begin shot");
		EnemyTankListNode * p = head;
		while (p != NULL)
		{
			p->tank.shot();
			p = p->next;
		}
		shot_t_interval = millis() + SHOT_TIME_INTERVAL;
	}
}

//检测碰撞
void crashDetect( struct EnemyTankListNode *p) //ex,ey敌方坦克左上顶点坐标
{
	uint8_t
		mx = myTank.getX(), my = myTank.getY(), //我的坦克
		ex = p->tank.getX(), ey = p->tank.getY(); //敌方坦克

	//ok
	if (ey + TANK_SIZE < my) //上边未相交
	{
		;
	}
	else if ((ey + TANK_SIZE) >= my && (ey + (TANK_SIZE )) < (my + TANK_SIZE)) //上边相交
	{
		if (ex >= (mx - 2) && ex < (mx + TANK_SIZE + 2))
		{

			p->tank.setY(max(1,my - TANK_SIZE));
			p->tank.stop();
		}
	}

	//ok
	if (ex + TANK_SIZE <= mx) //左边未相交
	{
		;
	}
	else if (ex + TANK_SIZE >=mx && ex + (TANK_SIZE) < mx + TANK_SIZE) //左边相交
	{
		if (ey >= my - 2 && ey < my + TANK_SIZE + 2)
		{
			p->tank.setX(max(1,mx - TANK_SIZE));
			p->tank.stop();
		}
	}

	//ok
	if (ey >= my + TANK_SIZE) //下边未相交
	{
		;
	}
	else if (ey >= my &&  ey < my + TANK_SIZE) //下边相交
	{
		if (ex >= mx - 2 && ex < mx + TANK_SIZE + 2)
		{
			p->tank.setY(min(my + TANK_SIZE,H-TANK_SIZE));
			p->tank.stop();
		}
	}


	//ok
	if (ex >= mx + TANK_SIZE) //右边未相交
	{
		;
	}
	else if (ex > mx && ex <= mx + TANK_SIZE) //右边相交
	{
		if (ey >= my-2 && ey < my + TANK_SIZE+2)
		{
			p->tank.setX(min(mx + TANK_SIZE,ACTIVE_WIN_W-TANK_SIZE));
			p->tank.stop();
		}

	}
}

void displayEnemyTank()
{
	EnemyTankListNode * p = head;

	while (p != NULL)
	{
		//检测敌方坦克是否与我的坦克碰撞
		crashDetect(p);
#ifdef DEBUG
		//静止调试
		p->tank.stop();
#else
		//运动
		p->tank.move(p->tank.getDir());
#endif
		p = p->next;
	}

	//每2秒换一次方向
	enmyTankChangeDir();
}

void displayEnemyTankShot()
{
	EnemyTankListNode * p = head;
	while (p != NULL)
	{
		p->tank.displayShot();
		//打印每个坦克当前子弹数量
		//Serial.print(p->id);
		//Serial.println(p->tank.getShotNum());
		p = p->next;
	}
	//Serial.println("****************");
	//每500ms发射一次子弹
	enemyTankShot();
}

//此处有些问题，头节点似乎没有删除。。。。就这样吧 ( ^ _ ^ )!!!!!
void deleteTank(EnemyTankListNode * phead, int id)
{
	EnemyTankListNode * pb, *pf;
	pb = phead;
	if (phead == NULL)
	{
		return;
	}
	//while (pb->id != id && pb->next != NULL)
	while (pb->next != NULL)
	{
		pf = pb;
		pb = pb->next;

		if (pb->id == id)
		{
			if (pb == phead)
			{
				phead = pb->next;
			}
			else
			{
				pf->next = pb->next;
			}
			free(pb);
			break;
			//Serial.println(phead->id);
		}
		else
		{
			//没有找到要删除的节点
			;
		}
	}
}

bool areaDetect(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)
{
	uint8_t x_max, y_max;
	x_max = x2 + TANK_SIZE;
	y_max = y2 + TANK_SIZE;

	if (x1 >= x2 && x1 < x_max && y1 >= y2 && y1 < y_max)
		return true;
	else
		return false;
}

unsigned long display_bomb_time = millis() + 100;
void myHitDetect()
{
	//检测我是否有击中坦克
	ShotListNode * pr = myTank.getShotListNode();

	while (pr != NULL)
	{
		//获取每个子弹的坐标
		uint8_t
			x = pr->shot.getX(),
			y = pr->shot.getY();

		//获取每个敌人坦克的坐标位置
		EnemyTankListNode * p = head;
		while (p != NULL)
		{
			//判断敌人坦克是否被击中
			uint8_t
				ex = p->tank.getX(),
				ey = p->tank.getY();

			if(areaDetect(x,y,ex,ey) == true)
			{
				//击中敌人坦克//分数加1
				score++;
				//led灯闪烁
				led.bln(1);
				//设置坦克消失
				p->tank.death();
				//子弹消失
				pr->shot.death();
				//删除被击中坦克节点
				add_id = p->id;
				deleteTank(head, p->id);
				return;
			}

			p = p->next;
		}

		//Serial.println(x);
		pr = pr->next;
	}
	//Serial.println(F("--------"));
}

void enemyHitDetect()
{
	EnemyTankListNode * pEnemy = head;

	while (pEnemy !=NULL)
	{

		////判断我的坦克是否被击中
		ShotListNode * pEnemyShot = pEnemy->tank.getShotListNode();
		while (pEnemyShot != NULL)
		{
			//敌人每个坦克发出的每个子弹的坐标
			uint8_t
				x = pEnemyShot->shot.getX(),
				y = pEnemyShot->shot.getY();

			//判断我的坦克是否被击中
			uint8_t
				mx = myTank.getX(),
				my = myTank.getY();

			if (areaDetect(x, y, mx, my) == true)
			{
				//我的坦克被击中
				//生命值减一
				life--;
				//子弹死亡
				pEnemyShot->shot.death();
#ifdef DEBUG
				//
#else
				//我的坦克死亡
				myTank.death();
				//create
				createMyTank();
#endif // DEBUG
				if (life == 0)
				{
#ifdef DEBUG
					life = MAX_LIFE;
					score = 0;
#else
					//游戏结束,转置结束页面
					page = END_PAGE;
					life = MAX_LIFE;
					score = 0;
#endif // DEBUG
				}
				return;
			}

			pEnemyShot = pEnemyShot->next;
		}

		pEnemy = pEnemy->next;
	}
}

#define INFO_POS (ACTIVE_WIN_W + 2)

void displayScore()
{
	u8g2.drawXBMP(INFO_POS, 8, 8, 8, heartBmp);
	u8g2.setCursor(INFO_POS, 28);
	u8g2.print(life);
	u8g2.drawXBMP(INFO_POS, 36, TANK_SIZE, TANK_SIZE, tankBitMap[DIR_UP]);
	u8g2.setCursor(INFO_POS, 58);
	u8g2.print(score);
}


/*******************************************按键控制，主流程************************************************/
void gameInit()
{
	//绘制边框及分割线
	drawBorder(0, 0, W, H);
	u8g2.drawVLine(ACTIVE_WIN_W, 0, H);
	//添加敌方坦克
	creatEnemyTank();
	displayEnemyTank();

	//子弹移动
	myTank.displayShot();
	displayEnemyTankShot();

	//检测是否有击中坦克，击中后置isLive状态为false
	myHitDetect();
	enemyHitDetect();
	//显示分数
	displayScore();
}

unsigned long my_shot_t_interval = millis() + (SHOT_TIME_INTERVAL / 10); 
void gameStart(KeyMap k)
{
	gameInit();
	//初始化完成，开始按键控制
	switch (k)
	{
	case KEY_UP:
		myTank.move(DIR_UP);
		break;
	case KEY_DOWN:
		myTank.move(DIR_DOWN);
		break;
	case KEY_LEFT:
		myTank.move(DIR_LEFT);
		break;
	case KEY_RIGHT:
		myTank.move(DIR_RIGHT);
		break;
	case KEY_AA:
		if (millis() > my_shot_t_interval)
		{
			//避免长按键连续发射子弹
			myTank.shot();
			my_shot_t_interval = millis() + (SHOT_TIME_INTERVAL / 10);
		}
		break;
	case KEY_BB:
		page = START_PAGE;
		break;
	default:
		myTank.stop();
		break;
	}
}
