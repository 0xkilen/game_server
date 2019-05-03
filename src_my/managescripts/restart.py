#!/usr/bin/env python
#!-*- coding:utf-8 -*-

import os, sys, re
import getopt, subprocess
import time

program = ["./stop.py", "./start.py"]
process  = {"all" : 0, "scened" : 0, "logind" : 0, "dbcached" : 0, "chatd" : 0, "gmd" : 0, "mirrord" : 0, "log_serverd" : 0, "routed" : 0}

def restart_process():
        for p in process:
                if process[p] == 1:
                        os.system("./stop_" + p + ".py")
                        time.sleep(1)
                        os.system("./start_" + p + ".py")

def start():
        count = 0
        for p in process:
                if p != "all" and process[p] != 0:
                        count = count + 1
        if count == 0:
                if process["all"] == 1:
                        for pro in program:
                                os.system(pro)
                                time.sleep(5)
                else:
                        restart_helper()
        else:
                pg = "ps ux | grep jxwy_start_listen_process.py | grep -v 'grep' | awk '{print $2}'"
                sub = subprocess.Popen(pg, shell = True, stdout = subprocess.PIPE)
                pid_out = str(sub.communicate())
                pid = re.findall(r'(\w*[0-9]+)\w*',pid_out)
                if isinstance(pid, list) and len(pid) == 0:
                        restart_process()
                else:
                        os.system("./stop_listen_process.py")
                        restart_process()
                        os.system("nohup python -u jxwy_start_listen_process.py >/dev/null 2>&1 &")

def restart_helper():
        print "\nrestart 脚本使用说明\n"
        print "%-20s重启配置的所有进程" %"--all"
        print "%-20s重启scened进程" %"--scened"
        print "%-20s重启logind进程" %"--logind"
        print "%-20s重启dbcached进程" %"--dbcached"
        print "%-20s重启chad进程" %"--chatd"
        print "%-20s重启gmd进程" %"--gmd"
        print "%-20s重启log_serverd进程" %"--log_serverd"
        print "%-20s重启routed进程\n" %"--routedd"
        print "%-20s重启mirrord进程\n" %"--mirrord"
        os._exit(1)

try:
        opts, args = getopt.getopt(sys.argv[1:], "h", ["all", "routed", "scened", "logind", "dbcached", "chatd", "gmd", "mirrord", "log_serverd"])
        for op, value in opts:
                if op == "-h":
                        restart_helper()
                elif op == "--all":
                        process["all"] = 1
                elif op == "--scened":
                        process["scened"] = 1
                elif op == "--logind":
                        process["logind"] = 1
                elif op == "--dbcached":
                        process["dbcached"] = 1
                elif op == "--chatd":
                        process["chatd"] = 1
                elif op == "--gmd":
                        process["gmd"] = 1
                elif op == "--mirrord":
                        process["mirrord"] = 1
                elif op == "--log_serverd":
                        process["log_serverd"] = 1
                elif op == "--routed":
                        process["routed"] = 1
except:
        print "\n参数错误"
        restart_helper()

start()
