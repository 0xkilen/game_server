#!/usr/bin/env python
#!-*- coding:utf-8 -*-

import os
import utils

child_path = os.getcwd()
parent_path = os.path.dirname(child_path)

def start():
        utils.check_configure()
        if os.path.exists(child_path + "/script_file/dbcached"):
                os.chdir(child_path + "/script_file")
                utils.check_port_public(utils.read_file("dbcached_port"), "jxwy_dbcached")
                os.chdir(parent_path)
                os.system("./jxwy_dbcached")
        if os.path.exists(child_path + "/script_file/all") or os.path.exists(child_path + "/script_file/game_server"):
                utils.check_single_process_port("jxwy_dbcached")
                os.chdir(parent_path)
                os.system("./jxwy_dbcached")

start()
