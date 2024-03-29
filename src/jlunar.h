﻿
//-----------------------------------------------------
//				  农历计算
//
//             作者	:  逸少
//
//             创建日期: 2002.5.21
//
//-----------------------------------------------------
#ifndef __cjx_Lunar_H__
#define __cjx_Lunar_H__

#define LUNAR_API

#ifdef WORD
	#undef WORD
#endif
typedef unsigned short WORD;

#ifdef _T
	#undef _T
#endif
#define _T(x) x
//----------------------------------------------------
//            农历日期数据结构
//----------------------------------------------------
typedef struct _tagLUNARDATE
{	// 阳历数据
	WORD  wYear;	    // 输入参数 - 年，十进制，例如 2000
	WORD  wMonth;	    // 输入参数 - 月，十进制，例如 10
	WORD  wDay;			// 输入参数 - 日，十进制，例如 1
	WORD  wWeekDay;	    // 可以是输入或输出 - 星期, 0 - 星期日, 1~6 - 星期一 ~ 星期六
	// 下面是输出参数:
	//    农历数据
	WORD  wLunarYear;	// 农历年，如 2000
	WORD  wLunarMonth;  // 农历月，如 12, 1 ~ 12
	WORD  wLunarDay;	// 农历日，如 13, 1 ~ 30
	WORD  wIsLeapMonth; // 是否是闰月， 1 为农历闰月, 0 为非闰月
}LUNARDATE;

//--------------------------------------
//  几月几日 是 什么节 的 节日数据结构
//      公历农历通用
//--------------------------------------
typedef struct _tagHOLIDAY
{
	WORD  wMonth;			//月 1 ~ 12
	WORD  wDay;				//日 1 ~ 31
	char  szName[30]; //节日名, 14 个汉字
}HOLIDAY;

//----------------------------------------
//  数据结构: 几月的第几个星期几是什么节?
//           公历专用
//----------------------------------------
typedef struct _tagHOLIDAY2
{
	WORD  wMonth;			// 几月? 1 ~ 12
	WORD  wN;				// 第几个星期?
	WORD  wWeekday;		// 星期几
	char  szName[30];	//节日名, 14 个汉字
}HOLIDAY2;

//------------------------------------------------------
//	 日历函数, 下面的函数只对 1900~2100年有效
//------------------------------------------------------
//--------------------------------------------------------
// 获取农历的数据结构(完成公历到农历的转换)
//   
//     参数:
//		   LUNARDATE*  pLunarData:  输入输出
//               必须填充公历的年月日数据, 函数填充农历数据
//
//-----------------------------------------------------------
LUNAR_API bool cjxGetLunarDate(LUNARDATE*  pLunarData);

//-----------------------------------------------------------
//  完成从 农历 到 公历的转换
//     参数:
//
//		LUNARDATE*  pLunarData:  输入输出
//          必须填充 农历的年月日(和是否是闰月)数据, 函数填充公历数据
//-----------------------------------------------------------
LUNAR_API bool cjxGetSolarDate(LUNARDATE*  pLunarData);

//-----------------------------------------------------------
//     获取 距离 1900年1月1日 days 天的日期
//         1900年1月1日为1
//-----------------------------------------------------------
LUNAR_API bool cjxGetDate(int days, LUNARDATE*  pLunarData);

//--------------------------------------------
//  下面是定义的帮助函数
//--------------------------------------------
//---------------------------------------------------
//获 农历 生肖
//---------------------------------------------------
LUNAR_API const char* cjxGetShengxiao(WORD wLunarYear);

//---------------------------------------------------
//获 农历 天干
//---------------------------------------------------
LUNAR_API const char* cjxGetTiangan(WORD wLunarYear);

//---------------------------------------------------
//获 农历 地支
//---------------------------------------------------
LUNAR_API const char* cjxGetDiZhi(WORD wLunarYear);

//---------------------------------------------------
//获 农历月份的名称
//     名字串不包含月字样
//---------------------------------------------------
LUNAR_API const char* cjxGetMonthName(WORD wLunarMonth);

//---------------------------------------------------
//获 农历日的名称
//---------------------------------------------------
LUNAR_API const char* cjxGetDayName(WORD wLunarDay);

//---------------------------------------------------
// 获取星期几的名字(0="日",1="一"...)
//     名字串不包含星期字样
//---------------------------------------------------
LUNAR_API const char* cjxGetWeekName(WORD wWeekDay);

//-------------------------------------------------------
//  计算公历 y 年 m 月 d 日 是星期几 (0=星期天)
//-------------------------------------------------------
LUNAR_API int cjxGetWeekday(int y, int m, int d);

//----------------------------------------
//   计算下一个日期
//		参数中的公农历日期必须是正确的
//
//----------------------------------------
LUNAR_API bool cjxLunarNextDay(LUNARDATE*  pLunarData);

//===================================================================
//-------------------------------------------------------
// 计算农历某年的天数
//-------------------------------------------------------
LUNAR_API int cjxGetLunaYearDays(int lunarY);
//-------------------------------------------------------
//  计算农历某年某月的天数
//      月分 1~ 13, 必须有13个月时13才有意义
//-------------------------------------------------------
LUNAR_API int cjxGetLunarMonthDays(int lunarY, int lunarM);
//   返回农历 y年m月 是大月还是小月
//
//   参数:
//         y = 1900 ~ 2100(指的是农历)
//         m = 1 ~ 13(指的是农历): 
//   返回:
//         1 : 月大 (30天)
//         0 : 月小 (29天)
//-------------------------------------------------------
LUNAR_API int cjxLunarMonthBig(int y, int m);
//-------------------------------------------------------
//   返回农历 y 年 闰几月(1 ~ 12), 没有返回0
//-------------------------------------------------------
LUNAR_API int cjxLunarLeapMonth(int y);

//---------------------------------------------------------------
//  计算农历 y年m月d日到农历 1900年正月初一(公历1900-1-31)的天数
//    当天到当天为 1
//    这里不区分闰月, 农历月份为 1 ~ 13
//---------------------------------------------------------------
LUNAR_API int  cjxGetOffsetLunarDays(int year, int month, int day);

//------------------------------------------------------
//  计算 公历 year-month-day 到 1900-1-0的天数
//   
//     参数: 公历的年月日 1900-1-1 ~ 2100-12-31 之间
//
//     返回:
//          天数, 一个正数值 1900-1-1为 1
//          
//-----------------------------------------------------
LUNAR_API int  cjxGetOffsetSolarDays(int year, int month, int day);

//--------------------------------------------------------
//    返回农历年春节的公历日期(1900~2100)
//        即: 农历 y年正月初一 是 几月几日
//--------------------------------------------------------
//  返回农历春节的公历月份
LUNAR_API int cjxSpringMonth(int y);

//  返回农历春节的公历日
LUNAR_API int cjxSpringDay(int y);

//--------------------------------------------
// 返回 公历 y年某m 月的天数
//      
//     参数:  y  - 年, 1900 ~ 2100
//            m  - 月, 1    ~ 12
//--------------------------------------------
LUNAR_API int cjxGetSolarMonthDays(int y, int m) ;
//-------------------------------------------
// 计算公历某年的天数, y  - 年, 1900 ~ 2100
//-------------------------------------------
LUNAR_API int cjxGetSolarYearDays(int y);


//-----------------------------------------------------------
// 星座(constellation)
//    获取 几月几日 是 什么 星座
//    日期参数为公历
//-----------------------------------------------------------
LUNAR_API const char* cjxGetXingzuoName(int month, int day);


//----------------------------------------------------
// 获取传统农历节日的名称
//
//       参数为农历 y月m日
//
//       如果不是节日, 返回 NULL
//-----------------------------------------------------
LUNAR_API const char* cjxGetLunarHolidayName(WORD wLunarMonth, WORD wLunarDay);

//----------------------------------------------
// 获取公历节日的名称字符串
//       参数为公历日期(几月几日)
//       如果不是节日, 返回 NULL
//-----------------------------------------------
LUNAR_API const char* cjxGetSolarHolidayName(WORD wMonth, WORD wDay);

//-----------------------------------------------------
//   功能: 获取 Year 年 Month 月 Day 日 的 节气字符串
//        如果不是节气日, 返回 NULL
//
//   参数: Year:  年号(1900 - 2100) , 如 2004
//         Month: 月份, 1 ~ 12
//         Day:   日期, 1 ~ 31
//         
//         日期参数为公历
//
//   注意: 目前使用的查表法只支持 (1900 - 2100年)
// 
//----------------------------------------------------
LUNAR_API const char* cjxGetTermName(int Year, int Month, int Day);

//-----------------------------------------------------
//    获取 某年第n个节气的名称(从0小寒起算)
//       n = 0 ~ 23
//-----------------------------------------------------
LUNAR_API  const char*  cjxGetTermNameN(int n);

//-----------------------------------------------------
//    计算 y 年的第n个节气几号(从0小寒起算)
//-----------------------------------------------------
LUNAR_API  int cjxGetTermYN(int y, int n);

#endif //__cjx_Lunar_H__
