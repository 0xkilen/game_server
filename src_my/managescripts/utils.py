#!/usr/bin/env python
#!-*- coding:utf-8 -*-
import os, re, sys
import subprocess

child_path = os.getcwd()
parent_path = os.path.dirname(child_path)
                
process_port = {"jxwy_scened" : 0, "jxwy_gmd" : 2, "jxwy_logind" : 4, "jxwy_chatd" : 3,  "jxwy_mirrord" : 5, "jxwy_dbcached" : 7, "jxwy_log_serverd" : 10, "jxwy_routed" : 11, "jxwy_common_resourced" : 12}

def check_configure():
        if os.path.exists(child_path + "/script_file") == False:
                print "configure脚本没有执行"
                os._exit(1)

def check_port_public(port, process_name):
        pg = "netstat -anl | grep " + str(port) + " | grep LISTEN | awk '{print $4}'"
        sub = subprocess.Popen(pg, shell = True, stdout = subprocess.PIPE)
        process_info = sub.communicate()
        if len(process_info) >= 2 and process_info[0] != '':
                n = 0
                while n < len(process_info) - 1:
                        ipport = process_info[n].split(':', 1)
                        if len(ipport) >= 2 and ipport[1].replace('\n', '') == str(port):
                                print "%s端口号冲突"%process_name
                                os._exit(0)
                        n += 1

def read_file(file_name):
        with open(file_name, 'r') as fn:
                return fn.read()

def process_pid(read_pid):
        pg = "ps --no-heading " + read_pid + " | awk '{print $1}'"
        sub = subprocess.Popen(pg, shell = True, stdout = subprocess.PIPE)
        pid_out = str(sub.communicate())
        pid = re.findall(r'(\w*[0-9]+)\w*',pid_out)
        return pid

def check_all_process_port():
        os.chdir(child_path + "/script_file")
        if os.path.exists("base_port"):
                base_port = read_file("base_port")
                for pp in process_port:
                        port = int(base_port) + int(process_port[pp])
                        check_port_public(port, pp)

def check_single_process_port(process_name):
        os.chdir(child_path + "/script_file")
        if os.path.exists("base_port"):
                base_port = read_file("base_port")
                port = int(base_port) + int(process_port[process_name])
                check_port_public(port, process_name)
