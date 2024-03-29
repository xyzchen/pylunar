#!/usr/bin/env python3
#-*- coding:utf-8 -*-
#-----------------------------------------------
# 内存测试
#-----------------------------------------------
import linecache
import os
import tracemalloc
from lunar import LunarDate

#打印字典
def print_dict(dictdata):
	for k in dictdata:
		print("\033[32m{}\033[0m: {}".format(k, dictdata[k]))

#打印内存信息
def display_top(snapshot, key_type='lineno', limit=10):
	snapshot = snapshot.filter_traces((
		tracemalloc.Filter(False, "<frozen importlib._bootstrap>"),
		tracemalloc.Filter(False, "<unknown>"),
	))
	top_stats = snapshot.statistics(key_type)

	print("Top %s lines" % limit)
	for index, stat in enumerate(top_stats[:limit], 1):
		frame = stat.traceback[0]
		print("#%s: %s:%s: %d Bytes"
				% (index, frame.filename, frame.lineno, stat.size))
		line = linecache.getline(frame.filename, frame.lineno).strip()
		if line:
			print('    %s' % line)

	other = top_stats[limit:]
	if other:
		size = sum(stat.size for stat in other)
		print("%s other: %d Bytes" % (len(other), size))
	total = sum(stat.size for stat in top_stats)
	print("Total allocated size: %d Bytes" % (total))

#================================================
# main
#================================================
tracemalloc.start()
d = LunarDate()	

#分配1000, 不保存
for i in range(0, 1000):
	d = LunarDate.get_from_offset(1000+i)
	#print(d)

#分配10个对象并保存
a = [ ]
for i in range(0, 10):
	t = LunarDate.get_from_offset(1000+i)
	a.append(t)
for i in range(0, 10):
	print("{}. ({})  {}".format(i, id(a[i]), a[i]))

#查看内存使用
snapshot = tracemalloc.take_snapshot()
display_top(snapshot)

first_size, first_peak = tracemalloc.get_traced_memory()
print(f"{first_size=}, {first_peak=}")
