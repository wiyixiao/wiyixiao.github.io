#pragma once
#ifndef __MENU_H_
#define __MENU_H_

#include "global.h"
#include <U8g2lib.h>

class GMenu
{
public:
	U8G2 *oled;
	GMenu(U8G2 *oled);
	~GMenu();
	void
		creadMenu(const char(*p)[page_item_length], Page id, Num num),
		drawMenu(void),
		setItem(byte dir),
		itemChanged(byte dir),
		enterPage(Page p),
		showMBox(const char * str),
		closeMBox();

	bool
		back(void),
		home(void);
	uint8_t
		getItem(void);
	Page getPage(void);
private:
	struct  pageListNode
	{
		Page pageID; //页面id
		uint8_t items; //页面的选项数量
		const char(*page)[page_item_length]; //页面数据
		pageListNode *next; //指向下一个
	};
	pageListNode *head = nullptr;

	const char(*page)[page_item_length];
	uint8_t
		pageNum,
		itemNum;
	int8_t
		curItem,
		itemSelect,
		item;
	Page
		curPage,
		lastPage;
		
	byte size;
	char item_buffer[page_item_length];

protected:
	int16_t
		item_h,
		item_w;
};

#endif // !__MENU_H

