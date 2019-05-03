#!/usr/bin/env python
#!-*- coding:utf-8 -*-
import os, re, sys
import utils
import time

child_path = os.getcwd()
parent_path = os.path.dirname(child_path)

process_string = ["routed", "logind", "mirrord", "dbcached", "chatd", "gmd", "log_serverd", "scened", "common_resourced", "listen"]

def check_process_exit():
        for i in process_string:
                if os.path.exists(parent_path + "/pids/" + i + "_pid"):
                        os.chdir(parent_path + "/pids/")
                        read_pid = utils.read_file(i + "_pid")
                        pid = utils.process_pid(read_pid)
                        for j in pid:
                                if j == read_pid:
                                        print "进程存在"
                                        os._exit(1)

def start():
        utils.check_configure()
        if os.path.exists("all"):
                utils.check_all_process_port()
        check_process_exit()
        for ps in process_string:
                os.chdir(child_path)
                pro = str("./start_" + ps + ".py")
                if pro == "./start_listen.py":
	                time.sleep(3)
                        if os.path.exists("script_file/listend") == False:
                                os.system(pro)
                else:
                        os.system(pro)

start()
