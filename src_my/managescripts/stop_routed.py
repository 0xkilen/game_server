#!/usr/bin/env python
#!-*- coding:utf-8 -*-

import os, re, sys
import subprocess
import time
import getopt, getpass
import utils

sign = 0

child_path = os.getcwd()
parent_path = os.path.dirname(child_path)
os.chdir(parent_path)

def try_stop_process(pid, read_pid):
        for p in pid:
                if p == read_pid:
                        if sign == 1:
                                os.system("kill -9 " + read_pid)
                        else:
                                os.system("kill " + read_pid)

        time.sleep(1)
        pid = utils.process_pid(read_pid)
        if isinstance(pid, list) and len(pid) == 0:
                os.system("rm routed_pid")
                return False
        for p in pid:
                if p == read_pid:
                        return True
                else:
                        os.system("rm routed_pid")
                        return False

def stop_process():
        if os.path.exists(parent_path + "/pids"):
                os.chdir(parent_path + "/pids")
                if os.path.exists("routed_pid"):
                        read_pid = utils.read_file("routed_pid")
                        pid = utils.process_pid(read_pid)
                        count = 1
                        while True:
                                if (try_stop_process(pid, read_pid)):
                                        if count > 30 :
                                                os.system("kill -9 " + read_pid)
                                                print "强制停止jxwy_routed进程"
                                                os._exit(1)
                                        count = count + 1
                                else:
                                        break;

opts, args = getopt.getopt(sys.argv[1:], "h", ["force"])
for op, value in opts:
        if op == "--force":
                sign = 1

stop_process()
