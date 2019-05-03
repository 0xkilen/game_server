#!/usr/bin/env python
#!-*- coding:utf-8 -*-
import sys, os, re
import time
import subprocess
import utils

process_string = ["logind", "chatd", "scened", "dbcached", "gmd", "mirrord", "log_serverd"]

child_path = os.getcwd()
parent_path = os.path.dirname(child_path)

def write_file(file_name, value):
        os.chdir(child_path + "/script_file/")
        with open(file_name, 'w') as fn:
                fn.write(value)

def again_process(ps, pro):
        os.chdir(parent_path)
        if os.path.exists(parent_path + "/pids"):
                os.chdir(parent_path + "/pids")
                if os.path.exists(parent_path + "/pids/" + ps + "_pid"):
                        read_pid = utils.read_file(ps + "_pid")
                        pg = "ps --no-heading " + read_pid + " | awk '{print $1}'"
                        sub = subprocess.Popen(pg, shell = True, stdout = subprocess.PIPE)
                        pid_out = str(sub.communicate())
                        pid = re.findall(r'(\w*[0-9]+)\w*',pid_out)
                        if isinstance(pid, list) and len(pid) == 0:
	                	print " %s 进程不存在，重启进程中..."%(pro)
                                os.chdir(child_path)
                                os.system(pro)
                        else:
	                        print " %s 进程存在"%(pro)
                        
	time.sleep(1)

def start_listen():
	for ps in process_string:
                pro = "./start_" + ps + ".py"
		again_process(ps, pro)
def start():
        pid = os.getpid()
        write_file("listen_pid", str(pid))
        while True:
                start_listen()

start()
