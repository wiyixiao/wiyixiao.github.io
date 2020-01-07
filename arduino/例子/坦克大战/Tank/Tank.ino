/*
 * ̹�˴�ս��Ϸ
 * 2019/8/18
 * Aries.hu
 * Arduino Nano
 * ::::::::::::::
 * LED��Arduino Nano��������������(δʹ����ʱ)��OLED(I2C / 128x64)
 * �����ο�����������
 */

#include "global.h"
#include <U8g2lib.h>
#include <Wire.h>
#include "led.h"
#include "key.h"
#include "buzzer.h"

//#define DEBUG
//128 //��Ļ��� 
#define W (u8g2.getWidth()) 
//64 //��Ļ�߶�
#define H (u8g2.getHeight()) 
//���������//Ĭ�ϸ߶�Ϊ��Ļ�߶�
#define INFO_PANEL_W 32		
//̹�˻�����//Ĭ�ϸ߶�Ϊ��Ļ�߶�
#define ACTIVE_WIN_W (W - INFO_PANEL_W) 
//̹�˳ߴ�8*8
#define TANK_SIZE 8	
//�ӵ��ߴ�2*2
#define SHOT_SIZE 2
//��Ļ����ʾ�����з�̹������ //6��//����̫��������
#define ENEMY_TANK_MAX 4
//����ת��ʱ����
#define DIR_CHANGE_TIME_INTERVAL 1000
//̹���ӵ�����ʱ����
#define SHOT_TIME_INTERVAL 1000
//���������
#define MAX_LIFE 10
//̹�����ӵ��ٶ�
#define MAX_V 1000
uint16_t
myTankV = 1000, 
enemyTankV = 15,
shotV = 55;
//����
int score = 0;
//̹����󸴻����(255)
uint8_t life = MAX_LIFE;
//������Ӧ����
int keys[6] = { A6, A2, A7, A3, A0, A1 }; //�ϡ��¡����ҡ�����(AA)������(BB)
//��һ������̹��
bool firstCreate = true;

GLed led(11);	//led��
GKey key(keys); //����
GBuzzer buzzer(7); //������//δʹ��
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
	led.setLevel(15); //����Led�����ȣ�0 ~ 255�� //ģ�������
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

/*******************************************ҳ��************************************************/
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
	//���ڴ������Ҫ������
	//��Ч�����洢���á���Ϸ���������ȵ�
	u8g2.drawStr(10, 16, "You can perfect it!");
	u8g2.drawStr(10, 32, "Press the <BB> key");
	u8g2.drawStr(10, 48, "Exit!");
	u8g2.sendBuffer();

	if (k == KEY_BB)
	{
		page = START_PAGE;
	}
	
}
/*******************************************�ӵ���************************************************/

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
		uint8_t x; //x����
		uint8_t y; //y����
		uint8_t dir; //����
		uint16_t v; //�ٶ�//Ĭ��Ϊ̹���ٶ�
		bool isLive; //�Ƿ����
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

/*******************************************̹����************************************************/
//ÿ��̹�˷�����ӵ�
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
		uint8_t x; //x����
		uint8_t y; //y����
		Dir dir; //����
		uint16_t v; //�ٶ�///�ݶ�Ϊʱ���� us
		bool isLive; //�Ƿ�����
		long shot_num; //�ӵ�������
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
	return info.head; //�����ӵ�����ͷ�ڵ�
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
		//����ʱ�䣬ˢ��λ��
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
	//������״̬
	info.isLive = false;
	//��ʾ��ըЧ��
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

//���
void Tank::shot()
{
	//�����ӵ�
	Shot _shot;
	//��ʼ���ӵ�
	_shot.init();
	//�����ӵ�λ�ü�����
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
	//ÿ����һ�Σ��ӵ�����һ
	if (info.head == NULL)
	{
		info.head = new ShotListNode;
		//����ӵ�
		info.head->shot = _shot;
		//�����ӵ�id
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
	//�ӵ�������һ
	info.shot_num++;
}

//��ʾ�ӵ�
void Tank::displayShot()
{
	int delete_id = 0;
	ShotListNode * pr = info.head;
	while (pr != NULL)
	{
		//����ӵ��Ƿ�����
		if (pr->shot.isAlive() == false)
		{
			//Serial.println(pr->id);
			delete_id = pr->id;
		}

		pr->shot.move();
		pr = pr->next;
	}

	//ɾ���ӵ�
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
		//û���ҵ�Ҫɾ���Ľڵ�
		;
	}
}

/*******************************************�ҵ�̹��************************************************/

Tank myTank((ACTIVE_WIN_W - TANK_SIZE) / 2, (H - TANK_SIZE) / 2, myTankV, DIR_UP);

void createMyTank()
{
	myTank.init((ACTIVE_WIN_W - TANK_SIZE) / 2, (H - TANK_SIZE) / 2, myTankV, DIR_UP);
}

/*******************************************�з�̹��************************************************/
uint8_t add_id;
//�з�̹��
struct EnemyTankListNode
{
	Tank tank;
	uint8_t id;
	EnemyTankListNode *next;
};
EnemyTankListNode * head = nullptr;

//��ȡ��ǰ�з�̹������
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

//����̹��
void creatEnemyTank()
{
	uint8_t num = returnEnemyNum();
	//Serial.println(num);
	//��ȡ����//С���������ʱ���
	if (num < ENEMY_TANK_MAX)
	{
		//����̹��
		Tank enemyTank;
		if (firstCreate)
		{
			//���Ƴ�ʼ��ʾ���
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
			//�ǵ���״̬ʱ�������̹�˴�����λ��//���ڷ������Ե��������µ�̹��
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

//�з�̹�˷�������ı�
unsigned long dir_change_t_interval = millis() + DIR_CHANGE_TIME_INTERVAL; 
void enmyTankChangeDir()
{	
	//��ʱ������
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
	//��ʱ�����ӵ�
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

//�����ײ
void crashDetect( struct EnemyTankListNode *p) //ex,ey�з�̹�����϶�������
{
	uint8_t
		mx = myTank.getX(), my = myTank.getY(), //�ҵ�̹��
		ex = p->tank.getX(), ey = p->tank.getY(); //�з�̹��

	//ok
	if (ey + TANK_SIZE < my) //�ϱ�δ�ཻ
	{
		;
	}
	else if ((ey + TANK_SIZE) >= my && (ey + (TANK_SIZE )) < (my + TANK_SIZE)) //�ϱ��ཻ
	{
		if (ex >= (mx - 2) && ex < (mx + TANK_SIZE + 2))
		{

			p->tank.setY(max(1,my - TANK_SIZE));
			p->tank.stop();
		}
	}

	//ok
	if (ex + TANK_SIZE <= mx) //���δ�ཻ
	{
		;
	}
	else if (ex + TANK_SIZE >=mx && ex + (TANK_SIZE) < mx + TANK_SIZE) //����ཻ
	{
		if (ey >= my - 2 && ey < my + TANK_SIZE + 2)
		{
			p->tank.setX(max(1,mx - TANK_SIZE));
			p->tank.stop();
		}
	}

	//ok
	if (ey >= my + TANK_SIZE) //�±�δ�ཻ
	{
		;
	}
	else if (ey >= my &&  ey < my + TANK_SIZE) //�±��ཻ
	{
		if (ex >= mx - 2 && ex < mx + TANK_SIZE + 2)
		{
			p->tank.setY(min(my + TANK_SIZE,H-TANK_SIZE));
			p->tank.stop();
		}
	}


	//ok
	if (ex >= mx + TANK_SIZE) //�ұ�δ�ཻ
	{
		;
	}
	else if (ex > mx && ex <= mx + TANK_SIZE) //�ұ��ཻ
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
		//���з�̹���Ƿ����ҵ�̹����ײ
		crashDetect(p);
#ifdef DEBUG
		//��ֹ����
		p->tank.stop();
#else
		//�˶�
		p->tank.move(p->tank.getDir());
#endif
		p = p->next;
	}

	//ÿ2�뻻һ�η���
	enmyTankChangeDir();
}

void displayEnemyTankShot()
{
	EnemyTankListNode * p = head;
	while (p != NULL)
	{
		p->tank.displayShot();
		//��ӡÿ��̹�˵�ǰ�ӵ�����
		//Serial.print(p->id);
		//Serial.println(p->tank.getShotNum());
		p = p->next;
	}
	//Serial.println("****************");
	//ÿ500ms����һ���ӵ�
	enemyTankShot();
}

//�˴���Щ���⣬ͷ�ڵ��ƺ�û��ɾ������������������ ( ^ _ ^ )!!!!!
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
			//û���ҵ�Ҫɾ���Ľڵ�
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
	//������Ƿ��л���̹��
	ShotListNode * pr = myTank.getShotListNode();

	while (pr != NULL)
	{
		//��ȡÿ���ӵ�������
		uint8_t
			x = pr->shot.getX(),
			y = pr->shot.getY();

		//��ȡÿ������̹�˵�����λ��
		EnemyTankListNode * p = head;
		while (p != NULL)
		{
			//�жϵ���̹���Ƿ񱻻���
			uint8_t
				ex = p->tank.getX(),
				ey = p->tank.getY();

			if(areaDetect(x,y,ex,ey) == true)
			{
				//���е���̹��//������1
				score++;
				//led����˸
				led.bln(1);
				//����̹����ʧ
				p->tank.death();
				//�ӵ���ʧ
				pr->shot.death();
				//ɾ��������̹�˽ڵ�
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

		////�ж��ҵ�̹���Ƿ񱻻���
		ShotListNode * pEnemyShot = pEnemy->tank.getShotListNode();
		while (pEnemyShot != NULL)
		{
			//����ÿ��̹�˷�����ÿ���ӵ�������
			uint8_t
				x = pEnemyShot->shot.getX(),
				y = pEnemyShot->shot.getY();

			//�ж��ҵ�̹���Ƿ񱻻���
			uint8_t
				mx = myTank.getX(),
				my = myTank.getY();

			if (areaDetect(x, y, mx, my) == true)
			{
				//�ҵ�̹�˱�����
				//����ֵ��һ
				life--;
				//�ӵ�����
				pEnemyShot->shot.death();
#ifdef DEBUG
				//
#else
				//�ҵ�̹������
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
					//��Ϸ����,ת�ý���ҳ��
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


/*******************************************�������ƣ�������************************************************/
void gameInit()
{
	//���Ʊ߿򼰷ָ���
	drawBorder(0, 0, W, H);
	u8g2.drawVLine(ACTIVE_WIN_W, 0, H);
	//��ӵз�̹��
	creatEnemyTank();
	displayEnemyTank();

	//�ӵ��ƶ�
	myTank.displayShot();
	displayEnemyTankShot();

	//����Ƿ��л���̹�ˣ����к���isLive״̬Ϊfalse
	myHitDetect();
	enemyHitDetect();
	//��ʾ����
	displayScore();
}

unsigned long my_shot_t_interval = millis() + (SHOT_TIME_INTERVAL / 10); 
void gameStart(KeyMap k)
{
	gameInit();
	//��ʼ����ɣ���ʼ��������
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
			//���ⳤ�������������ӵ�
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
