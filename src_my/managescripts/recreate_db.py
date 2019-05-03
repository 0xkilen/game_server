#!/usr/bin/env python
#!-*- coding:utf-8 -*-

import os, re, sys
import subprocess
import getopt
import utils

game_sql = ['create_game_db.sql',
            'role_table.sql',
            'notice_table.sql',
            'league_table.sql',
            'league_separation_table.sql',
            'league_war_table.sql',
            'league_mgr_table.sql',
            'city_table.sql',
            'arena_rank_table.sql',
            'gladiator_table.sql',
            'fief_table.sql',
            'mansion_table.sql',
            'mansion_mgr_table.sql',
            'huanzhuang_pvp_table.sql',
            'marriage_table.sql',
            'server_table.sql',
            'system_announcement_table.sql',
            'images_announcement_table.sql',
            'rank_table.sql',
            'guanpin_table.sql',
            'guan_table.sql',
            'tower_records_table.sql',
            'conflict_battle_record_table.sql',
            'mail_table.sql',
            'used_rolename_table.sql',
            'child_table.sql',
            'child_mgr_table.sql',
            'activity_mgr_table.sql']
login_sql = ['create_login_db.sql', 'login_gonggao_table.sql', 'white_list_table.sql']
log_sql = ['create_log_db.sql', 'log_table.sql']
gm_sql = ['create_gm_db.sql', 'gmd_administrator_table.sql']
common_resource_sql = ['create_common_resource_db.sql', 'shandang_fanli_table.sql', 'come_back_table.sql']
pid_list = ['scened_pid', 'dbcached_pid', 'chatd_pid', 'logind_pid', 'gmd_pid', 'common_resourced_pid']

child_path = os.getcwd()
parent_path = os.path.dirname(child_path)

def read(name_file):
        with open(name_file, 'r') as port_file:
                return port_file.read()

def check_running_process():
        for pl in pid_list:
                if os.path.exists(parent_path + "/pids/" + pl):
                        os.chdir(parent_path + "/pids/")
                        read_pid = read(pl)
                        pg = "ps --no-heading " + read_pid + " | awk '{print $1}'"
                        sub = subprocess.Popen(pg, shell = True, stdout = subprocess.PIPE)
                        pid_out = str(sub.communicate())
                        pid = re.findall(r'(\w*[0-9]+)\w*',pid_out)
                        for i in pid:
                                if i == read_pid:
                                        print "进程正在运行，不可以执行清空数据库操作，请先执行 ./stop.py 关闭进程，然后再执行该操作"
                                        os._exit(1)
                                else:
                                        os.system("rm " + pl)
def recreate_game_db():
        os.chdir(parent_path)
        for gs in game_sql:
                if gs == 'create_game_db.sql':
                        os.system("/usr/bin/mysql -h " + game_mysql_ip + " -u" + game_mysql_username + " -p" + game_mysql_password + " < sqls/" + gs)
                else:
                        os.system("/usr/bin/mysql -h " + game_mysql_ip + " -u" + game_mysql_username +" -p"+ game_mysql_password + " " + game_db + " < sqls/" + gs)

def recreate_log_db():
        os.chdir(parent_path)
        for ls in log_sql:
                if ls == 'create_log_db.sql':
                        os.system("/usr/bin/mysql -h " + log_mysql_ip + " -u" + log_mysql_username + " -p" + log_mysql_password + " < sqls/" + ls)
                else:
                        os.system("/usr/bin/mysql -h " + log_mysql_ip + " -u" + log_mysql_username +" -p"+ log_mysql_password + " " + log_db + " < sqls/" + ls)

def recreate_login_db():
        os.chdir(parent_path)
        for ls in login_sql:
                if ls == 'create_login_db.sql':
                        os.system("/usr/bin/mysql -h " + login_mysql_ip + " -u" + login_mysql_username + " -p" + login_mysql_password + " < sqls/" + ls)
                else:
                        os.system("/usr/bin/mysql -h " + login_mysql_ip + " -u" + login_mysql_username +" -p"+ login_mysql_password + " " + login_db + " < sqls/" + ls)
def recreate_common_resource_db():
        os.chdir(parent_path)
        for crs in common_resource_sql:
                if crs == 'create_common_resource_db.sql':
                        os.system("/usr/bin/mysql -h " + common_resource_mysql_ip + " -u" + common_resource_mysql_username + " -p" + common_resource_mysql_password + " < sqls/" + crs)
                else:
                        os.system("/usr/bin/mysql -h " + common_resource_mysql_ip + " -u" + common_resource_mysql_username +" -p"+ common_resource_mysql_password + " " + common_resource_db + " < sqls/" + crs)
def recreate_gm_db():
        os.chdir(parent_path)
        for gs in gm_sql:
                if gs == 'create_gm_db.sql':
                        os.system("/usr/bin/mysql -h " + gm_mysql_ip + " -u" + gm_mysql_username + " -p" + gm_mysql_password + " < sqls/" + gs)
                else:
                        os.system("/usr/bin/mysql -h " + gm_mysql_ip + " -u" + gm_mysql_username +" -p"+ gm_mysql_password + " " + gm_db + " < sqls/" + gs)

try:
        os.chdir(child_path + "/script_file/")
        game_db = read("game_db")
        game_mysql_ip = read("game_mysql_ip")
        game_mysql_username = read("game_username")
        game_mysql_password = read("game_password")

        log_db = read("log_db")
        log_mysql_ip = read("log_mysql_ip")
        log_mysql_username = read("log_username")
        log_mysql_password = read("log_password")

        login_db = read("login_db")
        login_mysql_ip = read("login_mysql_ip")
        login_mysql_username = read("login_username")
        login_mysql_password = read("login_password")

        common_resource_db = read("common_resource_db")
        common_resource_mysql_ip = read("common_resource_mysql_ip")
        common_resource_mysql_username = read("common_resource_username")
        common_resource_mysql_password = read("common_resource_password")

        gm_db = read("gm_db")
        gm_mysql_ip = read("gm_mysql_ip")
        gm_mysql_username = read("gm_username")
        gm_mysql_password = read("gm_password")
except:
        print "请先执行configure 脚本,配置数据库所需参数"
        os._exit(1)

def start():
        utils.check_configure()
        check_running_process()
        os.chdir(child_path + "/script_file/")
        if os.path.exists("game_server") or os.path.exists("dbcached") or os.path.exists("scened"):
                recreate_game_db()
                recreate_log_db()
        elif os.path.exists("logind"):
                recreate_login_db()
        elif os.path.exists("common_resourced"):
                recreate_common_resource_db()
        elif os.path.exists("gmd"):
                recreate_gm_db()
        elif os.path.exists("all"):
                recreate_game_db()
                recreate_log_db()
                recreate_login_db()
                recreate_common_resource_db()
                recreate_gm_db()
        else:
                print "\n不需要重建数据库\n"
                os._exit(0)
start()
