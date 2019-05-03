#!/usr/bin/env python
#!-*- coding:utf-8 -*-
import os, re, sys
import getopt

child_path = os.getcwd()
parent_path = os.path.dirname(child_path)

def write_file(file_name, r_file):
        os.chdir(child_path)
        with open(file_name, 'a') as w_file:
                w_file.write(r_file)

def start():
        if os.path.exists("bi_explain.txt"):
                os.system("rm bi_explain.txt")
        os.chdir(parent_path + "/proto")
        f = open("data_base.proto")
        while True:
                line = f.readline()  
                if not line:
                        f.close()
                        os._exit(1)
                if 'CO_' in line:
                        key =  filter(str.isdigit, line)
                        split_value = line.split(';')
                        if len(split_value) >= 2:
                                value = split_value[1].split(' ')
                                if len(value) >= 3:
                                        write_file("bi_explain.txt", str(key) + " " + str(value[2]))

def helper():
        print "tools下执行该脚本, 不带参数, 会在该目录下生成bi_explain.txt文件, txt文件在windows下需要用node++打开才能显示正常格式\n"
        os._exit(0)
try:
        opts, args = getopt.getopt(sys.argv[1:], "h", "help")
        for op, value in opts:
                if op == "--help" or op == "-h":
                        helper()
except:
        print "\n参数错误"
        helper()

start()
