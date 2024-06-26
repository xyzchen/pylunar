﻿//------------------------------------------------------
//
//    cjxLunarJQ.cpp
//        农历转换的实现部分
//
//				  计算 24 节气
//
//             作者	:  逸少
//
//             创建日期: 2002.5.21
//
//-------------------------------------------------------
#include <math.h>
#include <time.h>
#include <string.h>

#include "jlunar.h"

//====================================================================
//  关于二十四节气:
//          每年的二十四节气对应的阳历日期几乎固定，
//        平均分布于十二个月中   
//    1月      2月        3月       4月       5月      6月   
//小寒 大寒 立春 雨水 惊蛰 春分 清明 谷雨 立夏 小满 芒种 夏至   
//    7月      8月        9月      10月      11月     12月   
//小暑 大暑 立秋 处暑 白露 秋分 寒露 霜降 立冬 小雪 大雪 冬至  
//===================================================================


/* 农历二十四节气名称数据 */
const char* const cg_szLunarJieqi[] = { // 0 ~ 23
									//  以 0 小寒 起算
									_T("小寒"), _T("大寒"), // 一月的节气
									_T("立春"), _T("雨水"), // 二月的节气
									_T("惊蛰"), _T("春分"),
									_T("清明"), _T("谷雨"),
									_T("立夏"), _T("小满"),
									_T("芒种"), _T("夏至"),
									_T("小暑"), _T("大暑"),
									_T("立秋"), _T("处暑"),
									_T("白露"), _T("秋分"),
									_T("寒露"), _T("霜降"),
									_T("立冬"), _T("小雪"),
									_T("大雪"), _T("冬至")
									
								};

// PI值
const double PI = 3.1415926535897932;	
//---------------------
// 15度对应的弧度
const double QUOTIETY = PI * 15.0 / 180.0 ; 
//
// 因为每个节气之间相差15度(360/24)
//-----------------------------------------------------
//    计算 y 年的第n个节气几号(从0小寒起算)
//       该节气的月份   (int)(n/2+1)
//-----------------------------------------------------
LUNAR_API  int cjxGetTermYN(int y, int n)
{
	// 一个回归年 365.242 天
	// y年第n个节气距离(1900-1-0)的天数
	int offday = (int)(365.242 * (y - 1900) + 15.22 * n  - 1.9 * sin(QUOTIETY * n) + 6.2);

	LUNARDATE lunar;
	memset(&lunar, 0, sizeof(LUNARDATE));
	cjxGetDate(offday, &lunar);

	return lunar.wDay;
}

//-----------------------------------------------------
//    获取 某年第n个节气的名称(从0小寒起算)
//       n = 0 ~ 23
//-----------------------------------------------------
LUNAR_API  const char*  cjxGetTermNameN(int n)
{
	return cg_szLunarJieqi[n];
}

//-----------------------------------------------------
//   功能: 获取 Year 年 Month 月 Day 日 的 节气字符串
//        如果不是节气日, 返回 NULL
//
//   参数: Year:  年号, 如 2004
//         Month: 月份, 1 ~ 12
//         Day:   日期, 1 ~ 31
//----------------------------------------------------
LUNAR_API  const char* cjxGetTermName(int Year, int Month, int Day)
{
	int n = 0;
	if(Day < 15)
	{
		n = (Month - 1) * 2;
	}
	else
	{
		n = Month * 2 - 1;
	}
	if(cjxGetTermYN(Year, n) == Day)
	{// 是一个节气
		return cjxGetTermNameN(n);
	}
	return NULL;
}
