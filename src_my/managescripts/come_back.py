#!/usr/bin/env python
#!-*- coding:utf-8 -*-
import os, re, sys
import subprocess
import getopt
import signal

child_path = os.getcwd()
parent_path = os.path.dirname(child_path)
os.chdir(parent_path)

from_backdb_info = []
to_backdb_info = []

def come_back_help():
        print "\n 合服工具使用说明"
        print " %-20s说明选项" %"-h"
        print " %-20sfrom-backdb-info，数据库参数之前用英文横杠（-）区分，参数循序 数据库ipport-用户名-密码-回归数据库名-回归数据库表名" %"--from-backdb-info"
        print " %-20sto-backdb-info, 数据库参数之前用英文横杠（-）区分, 参数循序 数据库ipport-用户名-密码-回归数据库名" %"--backdb-info"
        print "example: ./come_back.py --from-backdb-info=127.0.0.1:3306-nora-1234-backdb-backtable --to-backdb-info=127.0.0.1:3306-nora-1234-backdb"
        os._exit(1)
        
def check_merge_come_backdb_info():
        if (len(from_backdb_info) != 5):
                print "from-backdb-info 参数有误"
                return False
        if (len(to_backdb_info) != 4):
                print "to-backdb-info 参数有误"
                return False
        return True

def come_back():
        back = '/usr/bin/mysql -h' + str(to_backdb_info[0].split(':')[0]) + ' -u' + str(to_backdb_info[1]) + ' -p' + str(to_backdb_info[2]) + ' -e "INSERT INTO ' + str(to_backdb_info[3]) + '.come_back(username) select uid from ' + str(from_backdb_info[3]) + '.' + str(from_backdb_info[4]) + '"'
        os.system(back)
        print '回归数据导入成功'


def start():
        if check_merge_come_backdb_info():
                come_back()

try:
        opts, args = getopt.getopt(sys.argv[1:], "h", ["from-backdb-info=", "to-backdb-info="])
        for op, value in opts:
                if op == '-h':
                        come_back_help()
                if op == "--from-backdb-info":
                        from_backdb_info = value.split('-')
                if op == "--to-backdb-info":
                        to_backdb_info = value.split('-')
except:
        print "\n参数错误"
        come_back_help()

start()
