#!/usr/bin/env python
#!-*- coding:utf-8 -*-

import os
import utils

child_path = os.getcwd()
parent_path = os.path.dirname(child_path)

def start():
        utils.check_configure()
        if os.path.exists(child_path + "/script_file/gmd"):
                os.chdir(child_path + "/script_file")
                utils.check_port_public(utils.read_file("gmd_port"), "jxwy_gmd")
                os.chdir(parent_path)
                os.system("./jxwy_gmd")
        if os.path.exists(child_path + "/script_file/all"):
                utils.check_single_process_port("jxwy_gmd")
                os.chdir(parent_path)
                os.system("./jxwy_gmd")

start()
