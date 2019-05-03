#!/usr/bin/env python
#!-*- coding:utf-8 -*-
import os, re, sys
import subprocess
import getopt
import signal

child_path = os.getcwd()
parent_path = os.path.dirname(child_path)
os.chdir(parent_path)

from_gamedb_info = []
to_gamedb_info = []

from_logdb_info = []
to_logdb_info = []

def db_merge_help():
        print "\n 合服工具使用说明"
        print " %-20s说明选项" %"-h"
        print " %-20sfrom-gamedb-info, 支持配置多个，一组数据库参数之前用英文横杠（-）区分，参数循序 数据库ipport-用户名-密码-游戏数据库名 ，数据库组之间用英文逗号区分（,）" %"--from-gamedb-info"
        print " %-20sto-gamedb-info, 支持配置一个，一组数据库参数之前用英文横杠（-）区分, 参数循序 数据库ipport-用户名-密码-游戏数据库名" %"--to-gamedb-info"
        print " %-20sfrom-logdb-info, 支持配置多个，一组数据库参数之前用英文横杠（-）区分，参数循序 数据库ipport-用户名-密码-日志数据库名 ，数据库组之间用英文逗号区分（,）" %"--from-logdb-info"
        print " %-20sto-logdb-info, 支持配置一个，一组数据库参数之前用英文横杠（-）区分, 参数循序 数据库ipport-用户名-密码-日志数据库名" %"--to-logdb-info"
        print "example: ./merge_db.py --from-gamedb-info=127.0.0.1:3306-nora-123-game1-log1,127.0.0.1:3306-nora-123-game2 --to-gamedb-info=127.0.0.1:3306-nora-123-game3"
        os._exit(1)
        
def check_merge_gamedb_info():
        for fai in from_gamedb_info:
                if (len(fai.split('-')) != 0):
                        if (len(fai.split('-')) != 4):
                                print "from-gamedb-info 参数有误"
                                return False
                else:
                        return False
        if (len(to_gamedb_info) != 0):
                if (len(to_gamedb_info) != 4):
                        print "to-gamedb-info 参数有误"
                        return False
        else:
                return False
        return True

def check_merge_logdb_info():
        for fli in from_logdb_info:
                if (len(fli.split('-')) != 0):
                        if (len(fli.split('-')) != 4):
                                print "from-logdb-info 参数有误"
                                return False
                else :
                        return False
        if (len(to_logdb_info) != 0):
                if (len(to_logdb_info) != 4):
                        print "to-logdb-info 参数有误"
                        return False
        else :
                return False
        return True

def merge_game():
        for fai in from_gamedb_info:
                merge = './merger --from_ipport ' + str(fai.split('-')[0]) + ' --from_user ' + str(fai.split('-')[1]) + ' --from_password ' + str(fai.split('-')[2]) + ' --from_game_database ' + str(fai.split('-')[3]) + ' --to_ipport ' + str(to_gamedb_info[0]) + ' --to_user ' + str(to_gamedb_info[1]) + ' --to_password ' + str(to_gamedb_info[2]) + ' --to_game_database ' + str(to_gamedb_info[3])
                os.system(merge)
        print 'game合服成功'

def merge_log():
        for i in range(len(from_logdb_info)):
                merge = '/usr/bin/mysql -h' + str(to_logdb_info[0].split(':')[0]) + ' -u' + str(to_logdb_info[1]) + ' -p' + str(to_logdb_info[2]) + ' -e "INSERT INTO ' + str(to_logdb_info[3]) + '.logs select * from ' + str(from_logdb_info[i].split('-')[3]) + '.logs"'
                os.system(merge)
        print 'log合服成功'


def start():
        if check_merge_gamedb_info():
                merge_game()
        if check_merge_logdb_info():
                merge_log()

try:
        opts, args = getopt.getopt(sys.argv[1:], "h", ["from-gamedb-info=", "to-gamedb-info=", "from-logdb-info=", "to-logdb-info="])
        for op, value in opts:
                if op == '-h':
                        db_merge_help()
                if op == "--from-gamedb-info":
                        from_gamedb_info = value.split(',')
                if op == "--to-gamedb-info":
                        to_gamedb_info = value.split('-')
                if op == "--from-logdb-info":
                        from_logdb_info = value.split(',')
                if op == "--to-logdb-info":
                        to_logdb_info = value.split('-')
except:
        print "\n参数错误"
        db_merge_help()

start()
