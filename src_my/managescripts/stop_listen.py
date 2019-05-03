#!/usr/bin/env python
#!-*- coding:utf-8 -*-

import os, re, sys
import subprocess
import getopt, time
import utils

child_path = os.getcwd()
parent_path = os.path.dirname(child_path)
process_string = ["logind", "chatd", "scened", "dbcached", "gmd", "mirrord", "log_serverd", "common_resourced", "routed"]

def try_stop_process(pid, read_pid, listen_pid):
        for p in pid:
                if p == read_pid:
                        os.system("kill -9 " + read_pid)

        time.sleep(1)
        pid = utils.process_pid(read_pid)
        if isinstance(pid, list) and len(pid) == 0:
                os.system("rm " + listen_pid)
                return False
        for p in pid:
                if p == read_pid:
                        return True
                else:
                        os.system("rm " + listen_pid)
                        return False

def start():
        for ps in process_string:
                os.chdir(child_path + "/script_file")
                listen_pid = "listen_" + ps+ "_pid"
                if os.path.exists(listen_pid):
                        read_pid = utils.read_file(listen_pid)
                        pid = utils.process_pid(read_pid)
                        count = 1
                        while True:
                                if (try_stop_process(pid, read_pid, listen_pid)):
                                        if count > 30 :
                                                print "kill 监听进程失败"
                                                os._exit(1)
                                        count = count + 1
                                else:
                                        break;

start()
