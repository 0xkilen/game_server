#!/usr/bin/env python
#!-*- coding:utf-8 -*-
import os, re, sys
import subprocess
import getopt
import signal

child_path = os.getcwd()
parent_path = os.path.dirname(child_path)
os.chdir(parent_path)

from_fanlidb_info = []
to_fanlidb_info = []

def shandang_fanli_help():
        print "\n 合服工具使用说明"
        print " %-20s说明选项" %"-h"
        print " %-20sfrom-fanlidb-info，数据库参数之前用英文横杠（-）区分，参数循序 数据库ipport-用户名-密码-返利数据库名" %"--from-fanlidb-info"
        print " %-20sto-fanlidb-info, 数据库参数之前用英文横杠（-）区分, 参数循序 数据库ipport-用户名-密码-返利数据库名" %"--fanlidb-info"
        print "example: ./shandang_fanli.py --from-fanlidb-info=127.0.0.1:3306-nora-1234-fanlidb --to-fanlidb-info=127.0.0.1:3306-nora-1234"
        os._exit(1)
        
def check_merge_shandang_fanlidb_info():
        if (len(from_fanlidb_info) != 4):
                print "from-fanlidb-info 参数有误"
                return False
        if (len(to_fanlidb_info) != 4):
                print "to-fanlidb-info 参数有误"
                return False
        return True

def shandang_fanli():
        fanli = '/usr/bin/mysql -h' + str(to_fanlidb_info[0].split(':')[0]) + ' -u' + str(to_fanlidb_info[1]) + ' -p' + str(to_fanlidb_info[2]) + ' -e "INSERT INTO ' + str(to_fanlidb_info[3]) + '.shandang_fanli(username, recharge_id) select user_id,product_id from ' + str(from_fanlidb_info[3]) + '.ks_order"'
        print fanli
        os.system(fanli)
        print '返利数据导入成功'


def start():
        if check_merge_shandang_fanlidb_info():
                shandang_fanli()

try:
        opts, args = getopt.getopt(sys.argv[1:], "h", ["from-fanlidb-info=", "to-fanlidb-info="])
        for op, value in opts:
                if op == '-h':
                        shandang_fanli_help()
                if op == "--from-fanlidb-info":
                        from_fanlidb_info = value.split('-')
                if op == "--to-fanlidb-info":
                        to_fanlidb_info = value.split('-')
except:
        print "\n参数错误"
        shandang_fanli_help()

start()
