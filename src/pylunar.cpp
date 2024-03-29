//------------------------------------------------------
//
//    pylunar.cpp
//        jlunar 的 python3 扩展
//
//       作者: 逸少
//     jmchxy@gmail.com
//     创建日期: 2018.5.14
//
//-------------------------------------------------------
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>
#include <stddef.h> /* for offsetof() */
#include <string.h>
#include <time.h>
#include "jlunar.h"
//==================================================
// 对象数据结构
//==================================================
//农历日期对象
typedef struct {
	PyObject_HEAD
	/* Type-specific fields go here. */
	short  year;	//公历年
	short  month;	//公历月
	short  day;		//公历日
	short  weekday;	//星期 (0 - 6)
	short  lunar_year;	//农历年
	short  lunar_month;	//农历月
	short  lunar_day;	//农历日
	short  lunar_isleap;//农历是否闰月
} LunarObject;

//==================================================
// 辅助函数
//==================================================
// 填充数据到自身
static void fillDataToObject(LunarObject* self, const LUNARDATE& src)
{
	self->year  = src.wYear;
	self->month = src.wMonth;
	self->day   = src.wDay;
	self->weekday = src.wWeekDay;
	self->lunar_year  = src.wLunarYear;
	self->lunar_month = src.wLunarMonth;
	self->lunar_day   = src.wLunarDay;
	self->lunar_isleap = src.wIsLeapMonth;
}

// 拷贝对象数据到C结构
static void copyDataFromObject(LUNARDATE* dest, const LunarObject* src)
{
	//公历
	dest->wYear  = src->year;
	dest->wMonth = src->month;
	dest->wDay   = src->day;
	dest->wWeekDay = src->weekday;
	//农历
	dest->wLunarYear  = src->lunar_year;
	dest->wLunarMonth  = src->lunar_month;
	dest->wLunarDay  = src->lunar_day;
	dest->wIsLeapMonth  = src->lunar_isleap;
}

//==================================================
// 对象属性定义
//==================================================

//释放对象
static void Lunar_dealloc(LunarObject *self)
{
	Py_TYPE(self)->tp_free((PyObject *) self);
}

//创建对象
static PyObject* Lunar_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	LunarObject* self = (LunarObject*) type->tp_alloc(type, 0);
	if(self != NULL)
	{
		self->year    = 0;
		self->month   = 0;
		self->day     = 0;
		self->weekday = 0;
		self->lunar_year  = 0;
		self->lunar_month = 0;
		self->lunar_day   = 0;
		self->lunar_isleap = 0;
	}
	return (PyObject*)self;
}

//初始化
static int Lunar_init(LunarObject* self, PyObject* args, PyObject* kwds)
{
	static char *kwlist[] = {"year", "month", "day", NULL};
	//初始化默认参数，当天
	time_t rawtime  = time(NULL);
	struct tm * ptm = gmtime ( &rawtime );
	int y = ptm->tm_year + 1900;
	int m = ptm->tm_mon  + 1;
	int d = ptm->tm_mday;
	//解析参数
	if (! PyArg_ParseTupleAndKeywords(args, kwds, "|hhh", kwlist, &y, &m , &d))
	{
		PyErr_SetString(PyExc_TypeError, "无效的参数");
		return -1;
	}
	//参数检查
	if(y < 1900 || y > 2100)
	{
		PyErr_SetString(PyExc_ValueError, "year 必须在 1900-2100 之间");
		return -1;
	}
	if(m < 1 || m > 12)
	{
		PyErr_SetString(PyExc_ValueError, "month 必须在 1-12 之间");
		return -1;
	}
	if(d < 1 || d > cjxGetSolarMonthDays(y, m))
	{
		PyErr_SetString(PyExc_ValueError, "day 值错误");
		return -1;
	}
	//从公历计算农历
	LUNARDATE lunardate;
	memset(&lunardate, 0, sizeof(lunardate));
	lunardate.wYear  = y;
	lunardate.wMonth = m;
	lunardate.wDay   = d;
	cjxGetLunarDate(&lunardate);
	//填充数据
	fillDataToObject(self, lunardate);
	//正常返回0
	return 0;
}

//类数据成员定义
static PyMemberDef Lunar_members[] = {
	/* name, type, offset, flags, doc */
	{"year", T_SHORT, offsetof(LunarObject, year), READONLY, "公历年(1900-2100)，只读"},
	{"month", T_SHORT, offsetof(LunarObject, month), READONLY, "公历月(1-12)，只读"},
	{"day",  T_SHORT, offsetof(LunarObject, day),  READONLY, "公历日(1-31)，只读"},
	{"weekday", T_SHORT, offsetof(LunarObject, weekday), READONLY, "星期(0-6)，只读"},
	{"lunar_year", T_SHORT, offsetof(LunarObject, lunar_year), READONLY, "农历年，只读"},
	{"lunar_month", T_SHORT, offsetof(LunarObject, lunar_month), READONLY, "农历月，只读"},
	{"lunar_day", T_SHORT, offsetof(LunarObject, lunar_day), READONLY, "农历日，只读"},
	{"lunar_isleap", T_SHORT, offsetof(LunarObject, lunar_isleap), READONLY, "当前农历月是否时闰月，只读"},
	{NULL}  //终止
};

//=====================================================
//  getter / setter
//=====================================================
// 获取年生肖
// 获取年的干支名称
static PyObject* Lunar_get_shengxiao(LunarObject* self, void* closure)
{
	return PyUnicode_FromString(cjxGetShengxiao(self->lunar_year));
}

// 获取年的干支名称
static PyObject* Lunar_get_ganzhi(LunarObject* self, void* closure)
{
	// 干支
	char buffer[40];
	strncpy(buffer, cjxGetTiangan(self->lunar_year), sizeof(buffer)-1);
	strncat(buffer, cjxGetDiZhi(self->lunar_year), sizeof(buffer)-1);
	return PyUnicode_FromString(buffer);
}

//获取月份名称
static PyObject* Lunar_get_monthname(LunarObject* self, void* closure)
{
	char buffer[40];
	// 月份
	if(self->lunar_isleap != 0)
	{
		strncpy(buffer, "闰", sizeof(buffer)-1);
		strncat(buffer, cjxGetMonthName(self->lunar_month), sizeof(buffer)-1);
	}
	else
	{
		strncpy(buffer, cjxGetMonthName(self->lunar_month), sizeof(buffer)-1);
	}
	return PyUnicode_FromString(buffer);
}

//获取日期名
static PyObject* Lunar_get_dayname(LunarObject* self, void* closure)
{
	return PyUnicode_FromString(cjxGetDayName(self->lunar_day));
}

//获取星期名
static PyObject* Lunar_get_weekname(LunarObject* self, void* closure)
{
	return PyUnicode_FromString(cjxGetWeekName(self->weekday));
}

// getter / setter 定义
static PyGetSetDef Lunar_getsetters[] = {
	{"shengxiao", (getter) Lunar_get_shengxiao, (setter) NULL, "生肖, 只读", NULL},
	{"ganzhi", (getter) Lunar_get_ganzhi, (setter) NULL, "农历年干支, 只读", NULL},
	{"month_name", (getter) Lunar_get_monthname, (setter) NULL, "农历月名称(不包含月字), 只读", NULL},
	{"day_name", (getter) Lunar_get_dayname, (setter) NULL, "农历日期名称, 只读", NULL},
	{"week_name", (getter) Lunar_get_weekname, (setter) NULL, "星期名称, 只读", NULL},
	{NULL}  /* Sentinel */
};

//=====================================================
// 成员函数定义
//=====================================================
//下一天
static PyObject* Lunar_next(LunarObject* self, PyObject* Py_UNUSED(ignored))
{
	//农历数据结构
	LUNARDATE lunardate;
	//拷贝数据到 LUNARDATE
	copyDataFromObject(&lunardate, self);
	//计算下一个日期
	cjxLunarNextDay(&lunardate);
	//填充自身
	fillDataToObject(self, lunardate);
	//Py_RETURN_NONE;  // = return None
	//返回自身
	Py_INCREF(self);
	return (PyObject*)self;
}

// 当前对象移动 n 天，向前(<0)或向后(>0)
static PyObject* Lunar_offset(LunarObject* self, PyObject* args)
{
	int offset;
	//解析参数
	if (! PyArg_ParseTuple(args, "i", &offset))
	{
		PyErr_SetString(PyExc_TypeError, "无效的参数");
		return NULL;
	}
	//计算新的绝对偏移量
	int days = offset + cjxGetOffsetSolarDays(self->year, self->month, self->day);
	//检查变量
	if( days < 1 || days > 73414)
	{
		PyErr_SetString(PyExc_ValueError, "offset 偏移值超出日期范围");
		return NULL;
	}
	//临时变量，计算用
	LUNARDATE  lunardate;
	//计算日期
	cjxGetDate(days, &lunardate);
	//填充自身
	fillDataToObject(self, lunardate);
	//返回自身
	Py_INCREF(self);
	return (PyObject*)self;
}

//到 1900-1-1(=1)的天数
static PyObject* Lunar_getOffsetDays(LunarObject* self, PyObject* Py_UNUSED(ignored))
{
	//计算天数
	int days = cjxGetOffsetSolarDays(self->year, self->month, self->day);
	//构造整数返回值
	return Py_BuildValue("i", days);
}

//转换成JSON
static PyObject* Lunar_to_json(LunarObject* self, PyObject* Py_UNUSED(ignored))
{
	// 构造一个字典对象返回给调用者
	PyObject* pDict = PyDict_New(); // 创建一个字典对象
	assert(PyDict_Check(pDict));
	// 添加 名/值 对到字典中
	PyDict_SetItemString(pDict, "year", Py_BuildValue("i", self->year));
	PyDict_SetItemString(pDict, "month", Py_BuildValue("i", self->month));
	PyDict_SetItemString(pDict, "day", Py_BuildValue("i", self->day));
	PyDict_SetItemString(pDict, "weekday", Py_BuildValue("i", self->weekday));
	PyDict_SetItemString(pDict, "lunar_year", Py_BuildValue("i", self->lunar_year));
	PyDict_SetItemString(pDict, "lunar_month", Py_BuildValue("i", self->lunar_month));
	PyDict_SetItemString(pDict, "lunar_day", Py_BuildValue("i", self->lunar_day));
	PyDict_SetItemString(pDict, "lunar_isleap", Py_BuildValue("i", self->lunar_isleap));
	// 设置字符串变量
	PyDict_SetItemString(pDict, "week_name", Py_BuildValue("s", cjxGetWeekName(self->weekday)));
	PyDict_SetItemString(pDict, "day_name", Py_BuildValue("s", cjxGetDayName(self->lunar_day)));
	// 临时缓冲区
	char buffer[100];
	// 月份
	if(self->lunar_isleap != 0)
	{
		strncpy(buffer, "闰", sizeof(buffer)-1);
		strncat(buffer, cjxGetMonthName(self->lunar_month), sizeof(buffer)-1);
		PyDict_SetItemString(pDict, "month_name", Py_BuildValue("s", buffer));
	}
	else
	{
		PyDict_SetItemString(pDict, "month_name", Py_BuildValue("s", cjxGetMonthName(self->lunar_month)));
	}
	// 生肖
	PyDict_SetItemString(pDict, "shengxiao", Py_BuildValue("s", cjxGetShengxiao(self->lunar_year)));
	// 干支
	strncpy(buffer, cjxGetTiangan(self->lunar_year), sizeof(buffer)-1);
	strncat(buffer, cjxGetDiZhi(self->lunar_year), sizeof(buffer)-1);
	PyDict_SetItemString(pDict, "ganzhi", Py_BuildValue("s", buffer));
	//——————————————————————————————————————————————————————
	//填充节气和节日：是指公农历节日和二十四节气
	//——————————————————————————————————————————————————————
	memset(buffer, 0, sizeof(buffer));
	//获取农历二十四节气信息
	const char* jieqistr = cjxGetTermName(self->year, self->month, self->day);
	if(jieqistr)
	{
		strncpy(buffer, jieqistr, sizeof(buffer)-1);
		PyDict_SetItemString(pDict, "jieqi", Py_BuildValue("s", buffer));
	}
	//获取节日信息
	memset(buffer, 0, sizeof(buffer));
	//先获取农历节日, 设置农历节日为清明节
	if((jieqistr != NULL) && (strcmp(jieqistr, "清明")==0))
	{
		strncpy(buffer, jieqistr, sizeof(buffer)-1);
	}
	else
	{
		const char* lunar_jieri = cjxGetLunarHolidayName(self->lunar_month, self->lunar_day);
		if(lunar_jieri)
		{
			strncpy(buffer, lunar_jieri, sizeof(buffer)-1);
		}
	}
	//获取公历节日
	const char* solar_jieri = cjxGetSolarHolidayName(self->month, self->day);
	if(solar_jieri)
	{
		strncat(buffer, "|", sizeof(buffer)-1);
		strncat(buffer, solar_jieri, sizeof(buffer)-1);
	}
	//设置节日字符串
	if(strlen(buffer) > 0)
	{
		PyDict_SetItemString(pDict, "jieri", Py_BuildValue("s", buffer));
	}
	// 返回字典对象给调用者
	return pDict;
}

//获取农历二十四节气, 如果没有返回 None
static PyObject* Lunar_get_jieqi(LunarObject* self, PyObject* Py_UNUSED(ignored))
{
	//获取农历二十四节气信息
	const char* jieqistr = cjxGetTermName(self->year, self->month, self->day);
	if(jieqistr)
	{
		return PyUnicode_FromString(jieqistr);
	}
	//没有节气，返回 None
	Py_RETURN_NONE;
}

//获取农历节日字符串, 如果没有返回 None
static PyObject* Lunar_get_lunar_holiday(LunarObject* self, PyObject* Py_UNUSED(ignored))
{
	const char* jieqistr = cjxGetLunarHolidayName(self->lunar_month, self->lunar_day);
	if(jieqistr)
	{
		return PyUnicode_FromString(jieqistr);
	}
	//没有节气，返回 None
	Py_RETURN_NONE;
}

//获取公历节日字符串, 如果没有返回 None
static PyObject* Lunar_get_solar_holiday(LunarObject* self, PyObject* Py_UNUSED(ignored))
{
	const char* jieqistr = cjxGetSolarHolidayName(self->month, self->day);
	if(jieqistr)
	{
		return PyUnicode_FromString(jieqistr);
	}
	//没有节气，返回 None
	Py_RETURN_NONE;
}

//=====================================================
// 普通成员函数前置函数声明
//=====================================================
// 返回当前对象的一个副本
static PyObject* Lunar_clone(LunarObject* self, PyObject* ignored);

//=====================================================
// 类函数前置函数声明
//=====================================================
// 从公历日期生成一个日期对象
//  参数：公历年月日
static PyObject* Lunar_getFromSolar(PyObject* self, PyObject* args);
// 从农历日期生成一个日期对象
//  参数：农历年月日，是否闰月
static PyObject* Lunar_getFromLunar(PyObject* self, PyObject* args, PyObject* kwds);
// 从距离 1900-01-01 的天数生成一个日期对象
//  参数： 距离 1900-01-01 的天数, 1900-01-01 为 1
static PyObject* Lunar_getFromOffset(PyObject* self, PyObject* args);
// 从指定的UNIX时间戳获取日期对象
//  参数：UNIX 时间戳，从 1970-01-01 00：00：00 开始经过的秒数
static PyObject* Lunar_getFromTime(PyObject* self, PyObject* args);
// 获取 公历 year年的天数 
static PyObject* Lunar_getSolarYearDays(PyObject* self, PyObject* args);
// 获取 公历 year年month月的天数 
static PyObject* Lunar_getSolarMonthDays(PyObject* self, PyObject* args);
// 获取 农历 year年的天数 
static PyObject* Lunar_getLunarYearDays(PyObject* self, PyObject* args);
// 获取 农历 year 年 month 月的天数 
static PyObject* Lunar_getLunarMonthDays(PyObject* self, PyObject* args);
// 获取 农历 year 年闰几月， 没有闰月返回 0
static PyObject* Lunar_getLunarLeap(PyObject* self, PyObject* args);
// 获取指定年份的春节
static PyObject* Lunar_getSpringDate(PyObject* self, PyObject* args);

//成员函数列表
static PyMethodDef Lunar_methods[] = {
	//类成员函数
	{"get_from_solar", (PyCFunction) Lunar_getFromSolar, METH_VARARGS | METH_CLASS, "get_from_solar(year, month, day): 从公历日期获取 LunarDate 对象"},
	{"get_from_lunar", (PyCFunction) Lunar_getFromLunar, METH_VARARGS | METH_CLASS, "get_from_solar(year, month, day, isleap=0): 从农历日期获取 LunarDate 对象"},
	{"get_from_offset", (PyCFunction) Lunar_getFromOffset, METH_VARARGS | METH_CLASS, "get_from_offset(days): 获取距离 1900-01-01(=1)为days天的日期"},
	{"get_from_time", (PyCFunction) Lunar_getFromTime, METH_VARARGS | METH_CLASS, "get_from_time(time_t): 从指定的UNIX时间戳获取日期对象"},
	//辅助函数
	{"get_days_for_solaryear", (PyCFunction) Lunar_getSolarYearDays, METH_VARARGS | METH_CLASS, "get_days_for_solaryear(year): 获取 公历 year年的天数 "},
	{"get_days_for_solarmonth", (PyCFunction) Lunar_getSolarMonthDays, METH_VARARGS | METH_CLASS, "get_days_for_solarmonth(year, month): 获取 公历 year年month月的天数"},
	{"get_days_for_lunaryear", (PyCFunction) Lunar_getLunarYearDays, METH_VARARGS | METH_CLASS, "get_days_for_lunaryear(year): 获取 农历 year年的天数 "},
	{"get_days_for_lunarmonth", (PyCFunction) Lunar_getLunarMonthDays, METH_VARARGS | METH_CLASS, "get_days_for_lunarmonth(year, month): 获取 农历 year 年 month 月的天数"},
	{"get_lunar_leapmonth", (PyCFunction) Lunar_getLunarLeap, METH_VARARGS | METH_CLASS, "get_lunar_leapmonth(year): 获取 农历 year 年闰几月， 没有闰月返回 0"},
	{"get_spring_date", (PyCFunction) Lunar_getSpringDate, METH_VARARGS | METH_CLASS, "get_spring_date(year): 获取 农历 year 年的春节是 几月几日，返回 (month, day)"},
	//普通成员函数
	{"clone", (PyCFunction) Lunar_clone, METH_NOARGS, "clone(): 返回当前对象的一个复制副本"},
	{"offset", (PyCFunction) Lunar_offset, METH_VARARGS, "offset(days): 当前对象向前或后移动 abs(days) 天, 返回自身"},
	{"get_offsetdays", (PyCFunction) Lunar_getOffsetDays, METH_NOARGS, "get_offsetdays(): 返回当前对象距离 1900-01-01 的天数 (1900-01-01 时为1)"},
	{"next", (PyCFunction) Lunar_next, METH_NOARGS, "next(): 到下一天，同 offset(1)，返回自身"},
	{"get_jieqi", (PyCFunction) Lunar_get_jieqi, METH_NOARGS, "get_jieqi(): 返回当日的二十四节气字符串，没有则返回 None"},
	{"get_lunar_holiday", (PyCFunction) Lunar_get_lunar_holiday, METH_NOARGS, "get_lunar_holiday(): 获取农历节日字符串, 如果没有返回 None"},
	{"get_solar_holiday", (PyCFunction) Lunar_get_solar_holiday, METH_NOARGS, "get_solar_holiday(): 获取公历节日字符串, 如果没有返回 None"},
	{"to_json", (PyCFunction) Lunar_to_json, METH_NOARGS, "to_json() : 转换成 JSON 字符串"},
	{NULL}  /* Sentinel */
};

//=====================================================
// 标准函数
//=====================================================
// __str__   被 str() 调用
static PyObject* Lunar_str(LunarObject* self)
{
	// 构造一个字符串返回给调用者
	char buffer[120];
	if(self->lunar_isleap == 0)
	{
		snprintf(buffer, sizeof(buffer)-1, "%d年%d月%d日 农历%s%s(%s)年%s月%s 星期%s",
			self->year, self->month, self->day,
			cjxGetTiangan(self->lunar_year),
			cjxGetDiZhi(self->lunar_year),
			cjxGetShengxiao(self->lunar_year),
			cjxGetMonthName(self->lunar_month),
			cjxGetDayName(self->lunar_day),
			cjxGetWeekName(self->weekday)
		);
	}
	else
	{
		snprintf(buffer, sizeof(buffer)-1, "%d年%d月%d日 农历%s%s(%s)年闰%s月%s 星期%s",
			self->year, self->month, self->day,
			cjxGetTiangan(self->lunar_year),
			cjxGetDiZhi(self->lunar_year),
			cjxGetShengxiao(self->lunar_year),
			cjxGetMonthName(self->lunar_month),
			cjxGetDayName(self->lunar_day),
			cjxGetWeekName(self->weekday)
		);
	}
	return PyUnicode_FromString(buffer);
}

// tp_richcompare ，被比较运算符调用
static PyObject* Lunar_richcmp(LunarObject* obj1, LunarObject* obj2, int op)
{
	bool r = false;
	//计算两个日期的便宜
	int offset1 = cjxGetOffsetSolarDays(obj1->year, obj1->month, obj1->day);
	int offset2 = cjxGetOffsetSolarDays(obj2->year, obj2->month, obj2->day);
	//比较
	switch (op)
	{
	case Py_LT:
		r = offset1 < offset2;
		break;
	
	case Py_LE:
		r = offset1 <= offset2;
		break;
	
	case Py_EQ:
		r = offset1 == offset2;
		break;
	
	case Py_NE:
		r = offset1 != offset2;
		break;
	
	case Py_GT:
		r = offset1 > offset2;
		break;
	
	case Py_GE:
		r = offset1 >= offset2;
		break;

	default:
		break;
	}
	PyObject* result = r ? Py_True : Py_False;
	Py_INCREF(result);
	return result;
}

//=====================================================
//对象类型定义
//=====================================================
static PyTypeObject LunarType = {
	.ob_base = PyVarObject_HEAD_INIT(NULL, 0)
	.tp_name = "lunar.LunarDate",
	.tp_basicsize = sizeof(LunarObject),
	.tp_itemsize  = 0,
	/* Methods to implement standard operations */
	.tp_dealloc           = (destructor) Lunar_dealloc,
	.tp_vectorcall_offset = 0,
	.tp_getattr           = NULL,
	.tp_setattr           = NULL,
	.tp_as_async          = NULL,
	.tp_repr              = NULL,
	/* Method suites for standard classes */
	.tp_as_number         = NULL,
	.tp_as_sequence       = NULL,
	.tp_as_mapping        = NULL,
	/* More standard operations (here for binary compatibility) */
	.tp_hash              = NULL,
	.tp_call              = NULL,
	.tp_str               = (reprfunc)Lunar_str,
	.tp_getattro          = NULL,
	.tp_setattro          = NULL,
	/* Functions to access object as input/output buffer */
	.tp_as_buffer         = NULL,
	/* Flags to define presence of optional/expanded features */
	.tp_flags     = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
	.tp_doc       = PyDoc_STR("LunarDate objects"),
	/* call function for all accessible objects */
	.tp_traverse  = NULL,
	/* delete references to contained objects */
	.tp_clear     = NULL,
	/* rich comparisons */
	.tp_richcompare = (richcmpfunc)Lunar_richcmp,
	/* weak reference enabler */
	.tp_weaklistoffset = NULL,
	/* Iterators */
	.tp_iter           = NULL,
	.tp_iternext       = NULL,
	/* Attribute descriptor and subclassing stuff */
	.tp_methods = Lunar_methods,
	.tp_members = Lunar_members,
	.tp_getset  = Lunar_getsetters,
	.tp_base    = NULL,
	.tp_dict    = NULL,
	.tp_descr_get = NULL,
	.tp_descr_set = NULL,
	.tp_dictoffset = NULL,
	.tp_init       = (initproc) Lunar_init,
	.tp_alloc      = NULL,
	.tp_new        = Lunar_new,
};

//=====================================================
// 普通成员函数定义
//=====================================================
// 返回当前对象的一个副本
static PyObject* Lunar_clone(LunarObject* self, PyObject* Py_UNUSED(ignored))
{
	// 构造一个 LunarObject 对象返回给调用者
	LunarObject* pLunarDate = PyObject_New(LunarObject, &LunarType);
	if(! pLunarDate)
	{
		return NULL;
	}
	//填充数据
	// 公历
	pLunarDate->year  = self->year;
	pLunarDate->month = self->month;
	pLunarDate->day   = self->day;
	pLunarDate->weekday = self->weekday;
	// 农历
	pLunarDate->lunar_year   = self->lunar_year;
	pLunarDate->lunar_month  = self->lunar_month;
	pLunarDate->lunar_day    = self->lunar_day;
	pLunarDate->lunar_isleap = self->lunar_isleap;
	//返回
	return (PyObject*)pLunarDate;
}

//=====================================================
// 类成员函数定义
//=====================================================
// 从公历日期生成一个日期对象
//  参数：公历年月日
static PyObject* Lunar_getFromSolar(PyObject* self, PyObject* args)
{
	int y, m, d;
	if (! PyArg_ParseTuple(args, "iii", &y, &m , &d))
	{
		PyErr_SetString(PyExc_TypeError, "无效的参数");
		return NULL;
	}
	//检查参数范围
	if(y < 1900 || y > 2100)
	{
		PyErr_SetString(PyExc_ValueError, "year 必须在 1900-2100 之间");
		return NULL;
	}
	if(m < 1 || m > 12)
	{
		PyErr_SetString(PyExc_ValueError, "month 必须在 1-12 之间");
		return NULL;
	}
	if(d < 1 || d > cjxGetSolarMonthDays(y, m))
	{
		PyErr_SetString(PyExc_ValueError, "day 值错误");
		return NULL;
	}
	//计算
	LUNARDATE lunardate;
	memset(&lunardate, 0, sizeof(lunardate));
	lunardate.wYear  = y;
	lunardate.wMonth = m;
	lunardate.wDay   = d;
	cjxGetLunarDate(&lunardate);
	// 构造一个 LunarObject 对象返回给调用者
	LunarObject* pLunarDate = PyObject_New(LunarObject, &LunarType);
	if(! pLunarDate)
	{
		return NULL;
	}
	fillDataToObject(pLunarDate, lunardate);
	return (PyObject*)pLunarDate;
}

// 从农历日期生成一个日期对象
//  参数：农历年月日，是否闰月
static PyObject* Lunar_getFromLunar(PyObject* self, PyObject* args, PyObject* kwds)
{
	//参数列表
	static char *kwlist[] = {"year", "month", "day", "isleap", NULL};
	//获取参数
	int y, m, d, leap=0;
	if (! PyArg_ParseTupleAndKeywords(args, kwds, "iii|i", kwlist, &y, &m, &d, &leap))
	{
		PyErr_SetString(PyExc_TypeError, "无效的参数，参数为(year, month, day, leap)");
		return NULL;
	}
	//检查参数范围
	if(y < 1900 || y > 2100)
	{
		PyErr_SetString(PyExc_ValueError, "year 必须在 1900-2100 之间");
		return NULL;
	}
	if(m < 1 || m > 12)
	{
		PyErr_SetString(PyExc_ValueError, "month 必须在 1-12 之间");
		return NULL;
	}
	if(d < 1 || d > cjxGetLunarMonthDays(y, m))
	{
		PyErr_SetString(PyExc_ValueError, "day 值错误");
		return NULL;
	}
	LUNARDATE lunardate;
	memset(&lunardate, 0, sizeof(lunardate));
	//计算到公历的时间
	lunardate.wLunarYear  = (WORD)y;
	lunardate.wLunarMonth = (WORD)m;
	lunardate.wLunarDay   = (WORD)d;
	lunardate.wIsLeapMonth= (WORD)leap;
	cjxGetSolarDate(&lunardate);
	// 构造一个 LunarObject 对象返回给调用者
	LunarObject* pLunarDate = PyObject_New(LunarObject, &LunarType);
	if(! pLunarDate)
	{
		return NULL;
	}
	fillDataToObject(pLunarDate, lunardate);
	return (PyObject*)pLunarDate;
}

// 从距离 1900-01-01 的天数生成一个日期对象
//  参数： 距离 1900-01-01 的天数, 1900-01-01 为 1
static PyObject* Lunar_getFromOffset(PyObject* self, PyObject* args)
{
	int days;
	//解析参数
	if (! PyArg_ParseTuple(args, "i", &days))
	{
		PyErr_SetString(PyExc_TypeError, "无效的参数");
		return NULL;
	}
	//检查变量
	if( days < 1 || days > 73414)
	{
		PyErr_SetString(PyExc_ValueError, "days 必须在 1 - 73414 之间");
		return NULL;
	}
	//临时变量，计算用
	LUNARDATE  lunardate;
	//计算日期
	cjxGetDate(days, &lunardate);
	//构造一个 LunarObject 对象返回给调用者
	LunarObject* pLunarDate = PyObject_New(LunarObject, &LunarType);
	if(! pLunarDate)
	{
		return NULL;
	}
	fillDataToObject(pLunarDate, lunardate);
	return (PyObject*)pLunarDate;
}

// 从指定的UNIX时间戳获取日期对象
//  参数：UNIX 时间戳，从 1970-01-01 00：00：00 开始经过的秒数
static PyObject* Lunar_getFromTime(PyObject* self, PyObject* args)
{
	//默认参数
	time_t rawtime = time(NULL);
	//解析参数
	if (! PyArg_ParseTuple(args, "|L", &rawtime))
	{
		PyErr_SetString(PyExc_TypeError, "无效的参数, 接收一个参数为unix时间戳(int)");
		return NULL;
	}
	struct tm * ptm = gmtime ( &rawtime );
	//获取时间
	LUNARDATE lunardate;
	memset(&lunardate, 0, sizeof(lunardate));
	lunardate.wYear  = ptm->tm_year + 1900;
	lunardate.wMonth = ptm->tm_mon  + 1;
	lunardate.wDay   = ptm->tm_mday;

	if(cjxGetLunarDate(&lunardate) == false)
	{
		return NULL;
	}
	//构造一个 LunarObject 对象返回给调用者
	LunarObject* pLunarDate = PyObject_New(LunarObject, &LunarType);
	if(! pLunarDate)
	{
		return NULL;
	}
	fillDataToObject(pLunarDate, lunardate);
	return (PyObject*)pLunarDate;
}

//------------------------------------------------------
//  获取 公历 year年的天数 
//     参数: year，month
//     返回:
//          天数
//-----------------------------------------------------
static PyObject* Lunar_getSolarYearDays(PyObject* self, PyObject* args) 
{
	int y;
	//解析参数
	if (! PyArg_ParseTuple(args, "i", &y))
	{
		PyErr_SetString(PyExc_TypeError, "无效的参数");
		return NULL;
	}
	//计算天数
	int days = cjxGetSolarYearDays(y);
	//构造整数返回值
	return Py_BuildValue("i", days);	
}

//------------------------------------------------------
//  获取 公历 year年month月的天数 
//     参数: year，month
//     返回:
//          天数, 一个正数值
//-----------------------------------------------------
static PyObject* Lunar_getSolarMonthDays(PyObject* self, PyObject* args) 
{
	int y, m;
	//解析参数
	if (! PyArg_ParseTuple(args, "ii", &y, &m))
	{
		PyErr_SetString(PyExc_TypeError, "无效的参数");
		return NULL;
	}
	//计算天数
	int days = cjxGetSolarMonthDays(y, m);
	//构造整数返回值
	return Py_BuildValue("i", days);	
}

//------------------------------------------------------
//  计算 农历 year年的天数 
//     参数: year
//     返回:
//          天数, 一个正数值
//-----------------------------------------------------
static PyObject* Lunar_getLunarYearDays(PyObject* self, PyObject* args) 
{
	int y;
	//解析参数
	if (! PyArg_ParseTuple(args, "i", &y))
	{
		PyErr_SetString(PyExc_TypeError, "无效的参数");
		return NULL;
	}
	if(y < 1900 || y > 2100)
	{
		PyErr_SetString(PyExc_ValueError, "year 必须在 1900 - 2100 之间");
		return NULL;
	}
	//计算天数
	int days = cjxGetLunaYearDays(y);
	//构造整数返回值
	return Py_BuildValue("i", days);	
}

//------------------------------------------------------
//  计算 农历 year年month月的天数 
//     参数: year，month(1-13)
//     返回:
//          天数, 一个正数值
//-----------------------------------------------------
static PyObject* Lunar_getLunarMonthDays(PyObject* self, PyObject* args) 
{
	int y, m;
	//解析参数
	if (! PyArg_ParseTuple(args, "ii", &y, &m))
	{
		PyErr_SetString(PyExc_TypeError, "无效的参数");
		return NULL;
	}
	if(y < 1900 || y > 2100)
	{
		PyErr_SetString(PyExc_ValueError, "year 必须在 1900 - 2100 之间");
		return NULL;
	}
	//计算天数
	int days = cjxGetLunarMonthDays(y, m);

	//构造整数返回值
	return Py_BuildValue("i", days);	
}

// 获取 农历 year 年闰几月， 没有闰月返回 0
static PyObject* Lunar_getLunarLeap(PyObject* self, PyObject* args)
{
	int y;
	//解析参数
	if (! PyArg_ParseTuple(args, "i", &y))
	{
		PyErr_SetString(PyExc_TypeError, "无效的参数");
		return NULL;
	}
	if(y < 1900 || y > 2100)
	{
		PyErr_SetString(PyExc_ValueError, "year 必须在 1900 - 2100 之间");
		return NULL;
	}
	// 获取 y 年 闰几月， 没有闰月返回 0
	int leapmonth = cjxLunarLeapMonth(y);
	return Py_BuildValue("i", leapmonth);
}

// 获取指定年份的春节
static PyObject* Lunar_getSpringDate(PyObject* self, PyObject* args)
{
	int y;
	//解析参数
	if (! PyArg_ParseTuple(args, "i", &y))
	{
		PyErr_SetString(PyExc_TypeError, "无效的参数");
		return NULL;
	}
	if(y < 1900 || y >= 2100)
	{
		PyErr_SetString(PyExc_ValueError, "year 必须在 1900 - 2099 之间");
		return NULL;
	}
	int m = cjxSpringMonth(y);
	int d = cjxSpringDay(y);
	//构造一个 Tuple (m, d) 返回给调用者
	PyObject* pTuple = PyTuple_New(2);
	assert(PyTuple_Check(pDict));
	// 添加 名/值 对到字典中
	PyTuple_SetItem(pTuple, 0, Py_BuildValue("i", m));
	PyTuple_SetItem(pTuple, 1, Py_BuildValue("i", d));
	// 返回
	return pTuple;
}

//=====================================================
//模块定义
//=====================================================
static PyModuleDef lunarmodule = {
	.m_base = PyModuleDef_HEAD_INIT,
	.m_name = "lunar",
	.m_doc  = "Python bindings for lunar（中国农历）",
	.m_size = -1,
	.m_methods = NULL,
	.m_slots   = NULL,
	.m_traverse = NULL,
	.m_clear   = NULL,
	.m_free    = NULL,
};

//=====================================================
// 初始化模块
//=====================================================
PyMODINIT_FUNC  PyInit_lunar(void)
{
	PyObject *m;
	if (PyType_Ready(&LunarType) < 0)
	{
		return NULL;
	}

	m = PyModule_Create(&lunarmodule);
	if (m == NULL)
	{
		 return NULL;
	}

	Py_INCREF(&LunarType);
	if (PyModule_AddObject(m, "LunarDate", (PyObject *) &LunarType) < 0)
	{
		Py_DECREF(&LunarType);
		Py_DECREF(m);
		return NULL;
	}

	//获取模版字典，然后给模版字典增加参数
	PyObject* d = PyModule_GetDict(m);
	if(d == NULL)
	{
		return NULL;
	}
	//添加变量
	PyDict_SetItemString(d, "__author__", Py_BuildValue("s", "陈逸少(jmchxy@gmail.com)"));
	//添加版本变量
	PyDict_SetItemString(d, "version", Py_BuildValue("(ii)", 0, 2));
	PyDict_SetItemString(d, "__version__", Py_BuildValue("s", "0.2"));

	return m;
}
