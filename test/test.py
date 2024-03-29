#!/usr/bin/env python3
#-*- coding:utf-8 -*-

import lunar
from lunar import LunarDate

#打印字典
def print_dict(dictdata):
	for k in dictdata:
		print("\033[32m{}\033[0m: {}".format(k, dictdata[k]))

#-------------------------------------------------------
# 测试
#-------------------------------------------------------
print("\n----------- lunar.__doc__ --------\n")
print(lunar.__doc__)
print("\n\n")

print("\n----------- lunar.__dict__ --------\n")
print_dict(lunar.__dict__)

print("\n--------dir(LunarDate)------------------\n")
#print("\n".join(dir(LunarDate)))
for k in dir(LunarDate):
	print("\033[32m{}\033[0m: {}".format(k, getattr(LunarDate, k).__doc__ ))

#-------------------------------------------------------
# 功能模块测试
#-------------------------------------------------------
# 构造对象测试
print("\n----------- LunarDate() test --------\n")
d1 = LunarDate()
print_dict(d1.to_json())

print("\n----------- LunarDate(2012, 3, 9) test --------\n")
d2 = LunarDate(2012, 3, 9)
print_dict(d2.to_json())

print("\n----------- LunarDate(2023, 3, 23) test --------\n")
d3 = LunarDate(2023, 3, 23)  #闰二月 初二
print_dict(d3.to_json())

print("\n---- LunarDate.get_from_lunar(2023, 1, 1) 春节 test --------\n")
d = LunarDate.get_from_lunar(2023, 1, 1)
print_dict(d.to_json())

print("\n---- LunarDate.get_from_offset(366) test --------\n")
d = LunarDate.get_from_offset(366)
print_dict(d.to_json())

print("\n---- LunarDate.get_from_time(0) test --------\n")
d = LunarDate.get_from_time(0)
print_dict(d.to_json())

### Test next ###
print("\n----------- LunarDate.next() --------\n")
for i in range(1, 10):
	d1.next()
	print("{}.next() : {}   offset: {}".format(i, d1, d1.get_offsetdays()))

### Test offset
d = LunarDate()  # now
print("\n----------- LunarDate.offset() --------\n")
print("now              : {}   offset: {}".format(d, d.get_offsetdays()))
d.offset(-100)
print("now.offset(-100) : {}   offset: {}".format(d, d.get_offsetdays()))
d.offset(100)
print("now.offset back  : {}   offset: {}".format(d, d.get_offsetdays()))

### Test clone
print("\n----------- LunarDate.clone() --------\n")
d1 = d
d2 = d.clone()
print("d ({}): {}".format(repr(d), d))
print("d1 =d ({}): {}".format(repr(d1), d1))
print("d2 = d.clone() : ({}): {}".format(repr(d2), d2))
d.next()
print("\n----------- after d.next() --------- ")
print("d ({}): {}".format(repr(d), d))
print("d1 = d({}): {}".format(repr(d1), d1))
print("d2 = d.clone() : ({}): {}".format(repr(d2), d2))

## Text Compare
print("\n----------- compare --------- ")
print("d == d1 : ", d==d1)
print("d == d2 : ", d==d2)
print("d != d2 : ", d!=d2)
print("d >= d2 : ", d>=d2)
print("d <= d2 : ", d<=d2)
print("d >  d2 : ", d>d2)
print("d <  d2 : ", d<d2)
