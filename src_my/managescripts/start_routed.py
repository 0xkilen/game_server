#!/usr/bin/env python
#!-*- coding:utf-8 -*-

import os
import utils

child_path = os.getcwd()
parent_path = os.path.dirname(child_path)

def start():
        utils.check_configure()
        if os.path.exists(child_path + "/script_file/routed"):
                os.chdir(child_path + "/script_file")
                routed_ports = utils.read_file("routed_ports").split(',')
                for i in routed_ports:
                        utils.check_port_public(i, "jxwy_routed")
                os.chdir(parent_path)
                os.system("./jxwy_routed")
        if os.path.exists(child_path + "/script_file/all"):
                utils.check_single_process_port("jxwy_routed")
                os.chdir(parent_path)
                os.system("./jxwy_routed")

start()
