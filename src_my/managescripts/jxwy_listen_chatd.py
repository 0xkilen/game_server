#!/usr/bin/env python
#!-*- coding:utf-8 -*-
import sys, os, re
import time
import subprocess
import utils

child_path = os.getcwd()
parent_path = os.path.dirname(child_path)

def write_file(file_name, value):
        os.chdir(child_path + "/script_file/")
        with open(file_name, 'w') as fn:
                fn.write(value)

def again_process(pro):
        os.chdir(parent_path + "/pids")
        read_pid = utils.read_file("chatd_pid")
        pg = "ps --no-heading " + read_pid + " | awk '{print $1}'"
        sub = subprocess.Popen(pg, shell = True, stdout = subprocess.PIPE)
        pid_out = str(sub.communicate())
        pid = re.findall(r'(\w*[0-9]+)\w*',pid_out)
        if isinstance(pid, list) and len(pid) == 0:
		print " chatd 进程不存在，重启进程中..."
                os.chdir(child_path)
                os.system(pro)
        else:
	        print " chatd 进程存在"
                        
	time.sleep(1)

def start():
        if os.path.exists(parent_path + "/pids/chatd_pid"):
                pid = os.getpid()
                write_file("listen_chatd_pid", str(pid))
                while True:
		        again_process("./start_chatd.py")

start()
