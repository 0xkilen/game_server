#!/usr/bin/env python
#!-*- coding:utf-8 -*-

import os
import utils

child_path = os.getcwd()
parent_path = os.path.dirname(child_path)

def start():
        utils.check_configure()
        if os.path.exists(child_path + "/script_file/game_server") or os.path.exists(child_path + "/script_file/scened"):
                os.chdir(child_path + "/script_file")
                utils.check_port_public(utils.read_file("scened_port"), "jxwy_scened")
                os.chdir(parent_path)
                os.system("./jxwy_scened")
        if os.path.exists(child_path + "/script_file/all"):
                utils.check_single_process_port("jxwy_scened")
                os.chdir(parent_path)
                os.system("./jxwy_scened")

start()
