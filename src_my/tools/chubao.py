#!/usr/bin/env python
#!-*- coding:utf-8 -*-
import os, re, sys
import getopt
import datetime

commit = ""

def start():
        data = datetime.datetime.now()
        year = data.year
        month = data.month
        day = data.day
        hour = data.hour
        minute = data.minute
        os.system('cd ~game_server/linux-x86_64-opt-tcmalloc/install/installers')
        os.system('mv jxwy_server_full_COMMIT_TIME_rel.tar.gz jxwy_server_full_' + commit + '_' + str(month) + str(day) + str(hour) + str(minute) + '00_rel.tar.gz')

try:
        opts, args = getopt.getopt(sys.argv[1:], ["commit=", "data="])
        for op, value in opts:
                if op == "--commit":
                        commit = value
                if op == "--data":
                        data = value
except:
        print "\n参数错误"
        helper()

start()
