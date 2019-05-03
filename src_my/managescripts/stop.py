#!/usr/bin/env python
#!-*- coding:utf-8 -*-
import os, sys
import getopt
import time
import utils

child_path = os.getcwd()
parent_path = os.path.dirname(child_path)
sign = 0
package = False

program = ["./stop_listen.py", "./stop_routed.py", "./stop_chatd.py", "./stop_logind.py", "./stop_gmd.py", "./stop_common_resourced.py", "./stop_robot.py", "./stop_scened.py", "./stop_mirrord.py", "./stop_log_serverd.py", "./stop_dbcached.py"]

def log_package():
        if os.path.exists(parent_path + "/logs/"):
                os.chdir(parent_path)
                old_log_name = time.strftime('%Y-%m-%d_%H.%M%S', time.localtime())
                if len(os.listdir(parent_path + "/logs/")) > 0:
                        if os.path.exists(parent_path + "/old_logs/") == False:
                                os.system("mkdir old_logs")
                        os.chdir(parent_path + "/logs/")
                        os.system("tar zcf " + old_log_name + ".tar.gz ./ --warning=no-file-changed")
                        os.system("mv *tar.gz ../old_logs/")
                        os.system("rm *")

def start():
        utils.check_configure()
        for pro in program:
                if sign == 1 and pro != "./stop_listen.py":
                        os.system(pro + " --force")
                else:
                        os.system(pro)
        if package:
                log_package()

def stop_help():
        print "\nstop 使用说明"
        print "%-20s说明选项" %"-h"
        print "%-20s选项后面不需要加任何参数，如果添加了该选项则所有停止进程脚本使用 kill -9 选项，否则使用 kill 选项" %"--force"
        print "%-20s是否对已经存在的日志打包(1 : 打包  0 : 不打包)，默认不打包\n" %"--log-package"
        os._exit(1)
try:
        opts, args = getopt.getopt(sys.argv[1:], "h", ["force", "log-package="])
        for op, value in opts:
                if op == "-h":
                        stop_help()
                        os._exit(1)
                elif op == "--force":
                        sign = 1
                elif op == "--log-package":
                        if value == "1":
                                package = True
except:
        print "\n参数错误"
        stop_help()

start()
