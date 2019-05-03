#!/usr/bin/env python
#!-*- coding:utf-8 -*-

import os

def start():
        sed_dunhao = str("sed -i 's/、/,/g' zangzi.txt")
        os.system(sed_dunhao)

        txt_file = open("zangzi.txt", 'r')
        csv_file = open("zangzi_table.csv", 'a')
        txt_ = txt_file.read()
        txt_.replace(', ,', ',')
        count = 0
        i = 1
        csv_ = csv_file.write(str(i)+',')
        for t in txt_:
                if count != 100:
                        if t == ',':
                                count +=1
                        csv_ = csv_file.write(t)
                if count == 100:
                        i +=1
                        csv_ = csv_file.write(',')
                        csv_ = csv_file.write('\n')
                        csv_ = csv_file.write(str(i)+',')
                        count = 0
        os.system("sed -i 's/, ,/,/g' zangzi_table.csv")
start()
