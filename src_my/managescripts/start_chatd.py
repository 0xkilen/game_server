#!/usr/bin/env python
#!-*- coding:utf-8 -*-

import os
import utils

child_path = os.getcwd()
parent_path = os.path.dirname(child_path)

def start():
        utils.check_configure()
        if os.path.exists(child_path + "/script_file/chatd"):
                os.chdir(child_path + "/script_file")
                utils.check_port_public(utils.read_file("chatd_port"), "jxwy_chatd")
                os.chdir(parent_path)
                os.system("./jxwy_chatd")
        if os.path.exists(child_path + "/script_file/all"):
                utils.check_single_process_port("jxwy_chatd")
                os.chdir(parent_path)
                os.system("./jxwy_chatd")

start()
