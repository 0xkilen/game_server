#!/usr/bin/env python
#!-*- coding:utf-8 -*-
import os, re, sys
import subprocess
import getopt
import signal

logind = 0
scened = 0
gmd = 0

child_path = os.getcwd()
parent_path = os.path.dirname(child_path)
os.chdir(parent_path)

def relogconfig_help():
        print "\n relogconfig使用说明"
        print " %-20s说明选项" %"-h"
        print " %-20s表示logind进程重新加载配置, 选项后面不需要加任何参数" %"--logind"
        print " %-20s表示scened进程重新加载配置, 选项后面不需要加任何参数" %"--scened"
        print " %-20s表示gmd进程重新加载配置, 选项后面不需要加任何参数" %"--gmd"
        os._exit(1)
        
def read_file(file_name):
        with open(file_name, 'r') as fn:
                return fn.read()

def start():
        if logind == 0 and scened == 0 and gmd == 0:
                relogconfig_help()
        if os.path.exists(parent_path + "/pids"):
                os.chdir(parent_path + "/pids")
                if logind == 0 and scened == 0 and gmd == 0:
                        relogconfig_help()
                        os._exit(1)
                if logind == 1:
                        if os.path.exists("logind_pid"):
                                read_pid = read_file("logind_pid")
                                os.kill(int(read_pid), signal.SIGUSR1)
                if scened == 1:
                        if os.path.exists("scened_pid"):
                                read_pid = read_file("scened_pid")        
                                os.kill(int(read_pid), signal.SIGUSR1)
                if gmd == 1:
                        if os.path.exists("gmd_pid"):
                                read_pid = read_file("gmd_pid")        
                                os.kill(int(read_pid), signal.SIGUSR1)

try:
        opts, args = getopt.getopt(sys.argv[1:], "h", ["logind", "scened", "gmd"])
        for op, value in opts:
                if op == "--logind":
                        logind = 1
                elif op == "--scened":
                        scened = 1
                elif op == "--gmd":
                        gmd = 1
except:
        print "\n参数错误"
        relogconfig_help()

start()
