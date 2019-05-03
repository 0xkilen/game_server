#!/usr/bin/env python
#!-*- coding:utf-8 -*-

import os, re, sys
import subprocess
import time
import getopt, getpass

sign = 0

child_path = os.getcwd()
parent_path = os.path.dirname(child_path)
os.chdir(parent_path)

def read_file(file_name):
        with open(file_name, 'r') as fn:
                return fn.read()

def judge_process(read_pid):
        pg = "ps --no-heading " + read_pid + " | awk '{print $1}'"
        sub = subprocess.Popen(pg, shell = True, stdout = subprocess.PIPE)
        pid_out = str(sub.communicate())
        pid = re.findall(r'(\w*[0-9]+)\w*',pid_out)
        return pid

def try_stop_process(pid, read_pid):
        for p in pid:
                if p == read_pid:
                        if sign == 1:
                                os.system("kill -9 " + read_pid)
                        else:
                                os.system("kill " + read_pid)

        time.sleep(1)
        pid = judge_process(read_pid)
        if isinstance(pid, list) and len(pid) == 0:
                os.system("rm robot_pid")
                return False
        for p in pid:
                if p == read_pid:
                        return True
                else:
                        os.system("rm robot_pid")
                        return False

def stop_process():
        if os.path.exists(parent_path + "/pids"):
                os.chdir(parent_path + "/pids")
                if os.path.exists("robot_pid"):
                        read_pid = read_file("robot_pid")
                        pid = judge_process(read_pid)
                        count = 1
                        while True:
                                if (try_stop_process(pid, read_pid)):
                                        if count > 30 :
                                                print "kill jxwy_robot 进程失败"
                                                os._exit(1)
                                        count = count + 1
                                else:
                                        break;

opts, args = getopt.getopt(sys.argv[1:], "h", ["force"])
for op, value in opts:
        if op == "--force":
                sign = 1

stop_process()
