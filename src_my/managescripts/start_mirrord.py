#!/usr/bin/env python
#!-*- coding:utf-8 -*-

import os
import utils

child_path = os.getcwd()
parent_path = os.path.dirname(child_path)

def start():
        utils.check_configure()
        if os.path.exists(child_path + "/script_file/mirrord"):
                os.chdir(child_path + "/script_file")
                utils.check_port_public(utils.read_file("mirrord_port"), "jxwy_mirrord")
                os.chdir(parent_path)
                os.system("./jxwy_mirrord")
        if os.path.exists(child_path + "/script_file/all"):
                utils.check_single_process_port("jxwy_mirrord")
                os.chdir(parent_path)
                os.system("./jxwy_mirrord")

start()
