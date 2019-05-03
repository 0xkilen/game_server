#!/usr/bin/env python
#!-*- coding:utf-8 -*-
import os

process_string = ["logind", "chatd", "scened", "dbcached", "gmd", "mirrord", "log_serverd", "common_resourced", "routed"]
def start():
        for ps in process_string:
                os.system("nohup python -u jxwy_listen_" + ps + ".py >/dev/null 2>&1 &")

start()
