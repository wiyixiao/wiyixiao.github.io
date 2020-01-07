#include "menu.h"

#define ITEM_MAX (4)

GMenu::GMenu(U8G2 *oled)
{
	this->oled = oled;
	item_h = 12; //默认高度
	item_w = oled->getWidth(); //默认宽度
	curItem = 0;
	itemSelect = 0;
}

GMenu::~GMenu()
{

}

//创建菜单，Num：菜单项数量
void GMenu::creadMenu(const char(*p)[page_item_length],Page id, Num num)
{
	//判断当前页面数量
	if (pageNum == 0)
	{
		//首次添加
		head = new pageListNode;
		head->page = p;
		head->pageID = id;
		head->items = num;
		head->next = nullptr;

		curPage = head->pageID;

		//显示页面
		itemNum = head->items;
		page = head->page;
		drawMenu();
	}
	else
	{
		//追加页面
		pageListNode *pr = head;
		pageListNode *pNew = new pageListNode;

		if (pNew == NULL)
		{
			//节点分配失败
			;
		}
		pNew->page = p;
		pNew->pageID = id;
		pNew->items = num;
		pNew->next = nullptr;

		//添加
		while (pr->next != NULL)
		{
			pr = pr->next;
		}
		pr->next = pNew;
		//pr = pNew;
	}

	//页面数量增加
	pageNum++;

}

void GMenu::drawMenu()
{
	int i, len, which;

	//清空屏幕
	oled->clear();

	//设置字体颜色
	oled->setFont(u8g_font_courB12);
	

	if (itemSelect < 0)
		itemSelect = 0;
	else if (itemSelect >= (itemNum-ITEM_MAX-1))
		itemSelect = max(0,(itemNum - ITEM_MAX-1));

	setItem(itemSelect + curItem);

	for (i = 0; i < itemNum; i++)
	{
		//获取每个项字符串长度
		uint8_t item = i + itemSelect;
		if (item < itemNum)
		{
			strcpy_P(item_buffer, *(page + item));
			//设置字体显示位置以及当前所选择菜单项
			if (curItem == i)
			{
				oled->drawBox(0, max(0, i*(item_h + 1)), item_w, item_h);
			}
			oled->setCursor(5, i*(item_h + 1) + 2);
			oled->print(item_buffer);
		}
		else
			break;
	}

}

void GMenu::itemChanged(byte dir)
{

	//oled->drawRect(0, max(0, curItem*(item_h + 1)), item_w, item_h, BLACK);
	//oled->display();

	curItem += dir;

	if (curItem < 0)
	{
		curItem = 0;
		itemSelect += dir;
		drawMenu();
	}
	else if (curItem > ITEM_MAX)
	{
		curItem = ITEM_MAX;
		itemSelect += dir;
		drawMenu();
	}
	setItem(curItem+itemSelect);

	//查看当前选项
	//Serial.println(item);

	oled->drawBox(0, max(0, curItem*(item_h + 1)), item_w, item_h);

}

uint8_t GMenu::getItem()
{
	return item;
}

void GMenu::setItem(byte dir) { item = dir; }

bool GMenu::back()
{
	//默认最后一项返回//首页无返回
	if (item == (itemNum-1) && curPage != head->pageID)
	{
		enterPage(lastPage);
	}
	return true;
}

//回首页
bool GMenu::home(void)
{
	enterPage(head->pageID);
	return true;
}

Page GMenu::getPage()
{
	return curPage;
}

void GMenu::enterPage(Page p)
{
	//进入页面
	pageListNode *pr = head;

	while (pr != NULL)
	{
		if (pr->pageID == p)
		{
			itemNum = pr->items;
			page = pr->page;
			curItem = 0;
			itemSelect = 0;
			//刷新页面
			drawMenu();
			//更新页面id
			lastPage = curPage;
			curPage = pr->pageID;

			break;
		}
		else
		{
			pr = pr->next;
		}
	}
}

void GMenu::showMBox(const char * str)
{
	oled->clear();

	uint8_t
		w = 120, h=50,
		x, y;
	x = (oled->getWidth() - w) / 2;
	y = (oled->getHeight() - h) / 2;

	oled->drawBox(x, y, w, h);
	oled->setCursor(x + 5, y + 5);
	oled->print(str);
	oled->display();

}

void GMenu::closeMBox()
{
	enterPage(curPage);
}