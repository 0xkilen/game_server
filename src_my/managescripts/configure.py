#!/usr/bin/env python
#!-*- coding:utf-8 -*-

import sys, getopt, os, re
import subprocess, getpass
import linecache
import utils

child_path = os.getcwd()
parent_path = os.path.dirname(child_path)

base_port = "7017"
set_process = ""
routed_ips = ['10.1.2.27']
routed_ports = ['9023']

config_info = {
        'SCENE_PLAYER_THREAD_COUNT' : '4',
        'SERVER_ID' : '0',
        'DBCACHED_ID' : '20000',
        'LOGIND_ID' : '30000',
        'GMD_ID' : '40000',
        'CHATD_ID' : '50000',
        'MIRRORD_ID' : '60000',
        'LOGSERVERD_ID' : '70000',
        'ROUTED_ID' : '80000',
        'COMMON_RESOURCED_ID' : '90000',
        'GAME_DB_DATABASE' : 'game_jxwy',
        'GAME_DB_IP' : '127.0.0.1',
        'GAME_DB_PORT' : '3306',
        'GAME_USER_NAME_DB' : 'nora',
        'GAME_PASSWORD_DB' : '1234',
        'LOG_DB_DATABASE' : 'log_jxwy',
        'LOG_DB_IP' : '127.0.0.1',
        'LOG_DB_PORT' : '3306',
        'LOG_USER_NAME_DB' : 'nora',
        'LOG_PASSWORD_DB' : '1234',
        'LOGIN_DB_DATABASE' : 'login_jxwy',
        'LOGIN_DB_IP' : '127.0.0.1',
        'LOGIN_DB_PORT' : '3306',
        'LOGIN_USER_NAME_DB' : 'nora',
        'LOGIN_PASSWORD_DB' : '1234',
        'COMMON_RESOURCE_DB_DATABASE' : 'common_resource_jxwy',
        'COMMON_RESOURCE_DB_IP' : '127.0.0.1',
        'COMMON_RESOURCE_DB_PORT' : '3306',
        'COMMON_RESOURCE_DB_USER' : 'nora',
        'COMMON_RESOURCE_DB_PASSWORD' : '1234',
        'GM_DB_DATABASE' : 'gm_jxwy',
        'GM_DB_IP' : '127.0.0.1',
        'GM_DB_PORT' : '3306',
        'GM_DB_USER' : 'nora',
        'GM_DB_PASSWORD' : '1234',
        'SDK_CHECK_LOGIN' : '1',
        'COST_FREE' : '0',
        'GAIN_TIMES' : '1',
        'ALLOW_ROBOT' : 'false',
        'ROBOT_SLOW' : 'true',
        'TESTIN' : 'false',
        'LOG_LEVEL' : 'INFO',
        'SCENED_DEAMON' : '1',
        'DEV_GM' : '0',
        'DBCACHE_PARSE_THREAD_COUNT' : '4',
        'DBCACHED_MAX_LEAGUES' : '125',
        'DBCACHED_MAX_ROLES' : '3500',
        'DBCACHED_MAX_MANSIONS' : '4000',
        'DBCACHED_MAX_FIEFS' : '4000',
        'DBCACHED_MAX_GLADIATORS' : '3500',
        'DBCACHED_MAX_GUANS' : '3500',
        'DBCACHED_MAX_CHILDREN' : '5000',
        'SCENED_MAX_ROLES' : '3000',
        'SCENED_MAX_MANSIONS' : '3500',
        'SCENED_MAX_FIEFS' : '3500',
        'SCENED_MAX_GLADIATORS' : '3000',
        'SCENED_MAX_GUANS' : '3000',
        'SCENED_MAX_LEAGUES' : '100',
        'SCENED_MAX_CHILDREN' : '4500',
        'YUNYING_BI_ADDR' : 'http://115.159.189.146:8004',
        'YUNYING_GM_ADDR' : 'http://115.159.189.146:8020',
        'YUNYING_LOGIN_ADDR' : 'http://kslogin.ksgame.com/login/login-check',
        'SCENED_DBCACHED_IP' : '127.0.0.1',
        'SCENED_CLIENT_IP' : '0.0.0.0',
        'SCENED_LOGSERVERD_IP' : '10.1.2.27',
        'COMMON_RESOURCE_SCENED_IP' : '10.1.2.27',
        'GM_SCENED_IP' : '10.1.2.27',
        'CHAT_SCENED_IP' : '10.1.2.27',
        'LOGIN_SCENED_IP' : '10.1.2.27',
        'LOGIN_GMD_IP' : '10.1.2.27',
        'MIRROR_SCENED_IP' : '10.1.2.27',
        'ROUTE_IP' : '10.1.2.27',
        'LOGIND_CLIENT_IP' : '0.0.0.0',
        'GMD_YUNYING_IP' : '0.0.0.0',
        'GMD_ADMIN_IP' : '0.0.0.0',
        'SCENED_PUBLIC_ARENAD_IP' : '127.0.0.1',
        'APP_ID' : '100000001',
        'APP_SECRET' : '0ad15dc69cb015db790ccea3df4c923c',
        'MINICORE' : '0',
        'WRITE_DBLOG' : 'false',
        'SEND_BI' : 'false',
        'WEAK_CONNECTION' : 'false',
        'LAG' : '0',
	'SSL_CERT' : '',
        'SSL_PRIVATE_KEY' : '',
        'CLIENT_VERSION' : '0.0.458',
        'LOGIND_DEAMON' : '1',
        'HOT_UPDATE_CLIENT_VERSION' : '1.0.048',
        'AUTO_RECHARGE' : 'false',
        'SKIP_XINSHOU' : 'false',
        'SSL_VERIFY' : '',
        'SCENED_CLIENT_PORT': '0',
        'LOGIND_CLIENT_PORT': '0',
        'GM_SCENED_PORT': '0',
        'CHAT_SCENED_PORT': '0',
        'LOGIN_SCENED_PORT': '0',
        'MIRROR_SCENED_PORT': '0',
        'SCENED_DBCACHED_PORT': '0',
        'GMD_YUNYING_PORT': '0',
        'GMD_ADMIN_PORT': '0',
        'SCENED_LOGSERVERD_PORT': '0',
        'SCENED_PUBLIC_ARENAD_PORT' : '0',
        'COMMON_RESOURCE_SCENED_PORT' : '0',
        'LOGIN_GMD_PORT': '0'}

port_deltas = {
        'SCENED_CLIENT_PORT': 0,
        'LOGIND_CLIENT_PORT': 1,
        'GM_SCENED_PORT': 2,
        'CHAT_SCENED_PORT': 3,
        'LOGIN_SCENED_PORT': 4,
        'MIRROR_SCENED_PORT': 5,
        'SCENED_DBCACHED_PORT': 7,
        'GMD_YUNYING_PORT': 8,
        'GMD_ADMIN_PORT': 9,
        'SCENED_LOGSERVERD_PORT': 10,
        'SCENED_PUBLIC_ARENAD_PORT' : 11,
        'COMMON_RESOURCE_SCENED_PORT' : 12,
        'LOGIN_GMD_PORT': 13}

game_server_default_port = {
        'LOGIND_CLIENT_PORT': 0,
        'SCENED_DBCACHED_PORT': 7,
        'GM_SCENED_PORT': 9019,
        'CHAT_SCENED_PORT': 9020,
        'LOGIN_SCENED_PORT': 9021,
        'LOGIN_GMD_PORT': 9030,
        'MIRROR_SCENED_PORT': 9022,
        'GMD_YUNYING_PORT': 0,
        'GMD_ADMIN_PORT': 0,
        'SCENED_LOGSERVERD_PORT': 9027,
        'SCENED_PUBLIC_ARENAD_PORT' : 11,
        'COMMON_RESOURCE_SCENED_PORT' : 9029}

all_default_port = {
        'SCENED_CLIENT_PORT': 0,
        'LOGIND_CLIENT_PORT': 1,
        'GM_SCENED_PORT': 2,
        'CHAT_SCENED_PORT': 3,
        'LOGIN_SCENED_PORT': 4,
        'MIRROR_SCENED_PORT': 5,
        'SCENED_DBCACHED_PORT': 7,
        'GMD_YUNYING_PORT': 8,
        'GMD_ADMIN_PORT': 9,
        'SCENED_LOGSERVERD_PORT': 10,
        'SCENED_PUBLIC_ARENAD_PORT' : 11,
        'COMMON_RESOURCE_SCENED_PORT' : 12,
        'LOGIN_GMD_PORT': 13}

all_default_ip = {
        'SCENED_DBCACHED_IP' : '127.0.0.1',
        'SCENED_CLIENT_IP' : '0.0.0.0',
        'SCENED_LOGSERVERD_IP' : '127.0.0.1',
        'COMMON_RESOURCE_SCENED_IP' : '127.0.0.1',
        'GM_SCENED_IP' : '127.0.0.1',
        'CHAT_SCENED_IP' : '127.0.0.1',
        'LOGIN_SCENED_IP' : '127.0.0.1',
        'LOGIN_GMD_IP' : '127.0.0.1',
        'MIRROR_SCENED_IP' : '127.0.0.1',
        'ROUTE_IP' : '127.0.0.1',
        'LOGIND_CLIENT_IP' : '0.0.0.0',
        'GMD_YUNYING_IP' : '0.0.0.0',
        'GMD_ADMIN_IP' : '0.0.0.0',
        'SCENED_PUBLIC_ARENAD_IP' : '127.0.0.1'}

def check_param():
        if (set_process == ""):
                configure_help()
        if (set_process == "all" or set_process == "game_server" or set_process == "scened" or set_process == "dbcached"):
                if config_info['GAME_DB_DATABASE'] == "":
                        print "\n游戏数据库没有配置\n"
                        os._exit(0)
                if config_info['LOG_DB_DATABASE'] == "":
                        print "\n日志数据库没有配置\n"
                        os._exit(0)

def clear_configure():
        os.system("rm -rf script_file")

def set_options_template():
        os.chdir(parent_path + "/config")
        template_head = linecache.getline("options_table.csv.template", 1)
        template_content = linecache.getline("options_table.csv.template", 2)
        template_content_split = template_content.split(',')

        # set routed ip port
        routed_index = template_content_split.index('ROUTED_ID')
        routed_index_ip = routed_index + 1
        routed_index_port = routed_index + 2
        for ri in routed_ips:
                template_content_split[routed_index_ip] = ri
                routed_index_ip = routed_index_ip + 2
        for rp in routed_ports:
                template_content_split[routed_index_port] = rp
                routed_index_port = routed_index_port + 2

        str_content = ','.join(template_content_split)
        content = template_head + str_content
        set_options(content)

def set_options(content):
        for ci in config_info:
                content = content.replace(ci, config_info[ci])
        write_template_file("options_table.csv", content)

def write_script_file(file_name, r_file):
        if os.path.exists(child_path + "/script_file") == False:
                os.system("mkdir script_file")
        os.chdir(child_path + "/script_file")
        with open(file_name, 'w') as w_file:
                w_file.write(r_file)

def write_template_file(file_name, r_file):
        with open(file_name, 'w') as w_file:
                w_file.write(r_file)

def set_db_name():
        db_name = {
                'GAME_DB_DATABASE' : 'create_game_db.sql',
                'LOG_DB_DATABASE' : 'create_log_db.sql',
                'LOGIN_DB_DATABASE' : 'create_login_db.sql',
                'COMMON_RESOURCE_DB_DATABASE' : 'create_common_resource_db.sql',
                'GM_DB_DATABASE' : 'create_gm_db.sql'}
        os.chdir(parent_path+"/sqls/")
        template_file = utils.read_file("create_db.sql.template")
        for dn in db_name:
                db_file = template_file.replace("{DB_NAME}", config_info[dn])
                write_template_file(db_name[dn], db_file)

def save_mysql_options():
        mysql_options = {
                'GAME_DB_DATABASE' : 'game_db',
                'LOG_DB_DATABASE' : 'log_db',
                'LOGIN_DB_DATABASE' : 'login_db',
                'COMMON_RESOURCE_DB_DATABASE' : 'common_resource_db',
                'GM_DB_DATABASE' : 'gm_db',
                'GAME_DB_IP' : 'game_mysql_ip',
                'GAME_USER_NAME_DB' : 'game_username',
                'GAME_PASSWORD_DB' : 'game_password',
                'LOG_DB_IP' : 'log_mysql_ip',
                'LOG_USER_NAME_DB' : 'log_username',
                'LOG_PASSWORD_DB' : 'log_password',
                'LOGIN_DB_IP' : 'login_mysql_ip',
                'LOGIN_USER_NAME_DB' : 'login_username',
                'LOGIN_PASSWORD_DB' : 'login_password',
                'COMMON_RESOURCE_DB_IP' : 'common_resource_mysql_ip',
                'COMMON_RESOURCE_DB_USER' : 'common_resource_username',
                'COMMON_RESOURCE_DB_PASSWORD' : 'common_resource_password',
                'GM_DB_IP' : 'gm_mysql_ip',
                'GM_DB_USER' : 'gm_username',
                'GM_DB_PASSWORD' : 'gm_password'}
        for mo in mysql_options:
                if config_info[mo] != "":
                        write_script_file(mysql_options[mo], config_info[mo])
        if set_process == "all":
                write_script_file("base_port", str(base_port))

def print_info():
        print "\n%s"%('确认参数信息')
        if set_process == 'all' or set_process == 'game_server' or set_process == "scened":
                print "%-30s%s"%('server-id', config_info['SERVER_ID'])
                print "%-30s%s"%('sdk-check-login', config_info['SDK_CHECK_LOGIN'])
                print "%-30s%s"%('cost-free', config_info['COST_FREE'])
                print "%-30s%s"%('gain-times', config_info['GAIN_TIMES'])
                print "%-30s%s"%('app-id', config_info['APP_ID'])
                print "%-30s%s"%('app-secret', config_info['APP_SECRET'])
                print "%-30s%s"%('lag', config_info['LAG'])
                print "%-30s%s"%('testin', config_info['TESTIN'])
                print "%-30s%s"%('auto-recharge', config_info['AUTO_RECHARGE'])
                print "%-30s%s"%('weak-connection', config_info['WEAK_CONNECTION'])
                print "%-30s%s"%('skip-xinshou', config_info['SKIP_XINSHOU'])
        print "%-30s%s\n"%('log-level', config_info['LOG_LEVEL'])

def check_error(pf):
        if os.path.exists(child_path + "/script_file/" + pf) == False:
                print "配置参数错误"
                os.chdir(child_path)
                os.system('rm -rf script_file')
                os._exit(1)

def check_configure_param():
        if '-' in (config_info['GAME_DB_DATABASE'] or config_info['LOG_DB_DATABASE'] or config_info['LOGIN_DB_DATABASE'] or config_info['COMMON_RESOURCE_DB_DATABASE'] or config_info['GM_DB_DATABASE']):
                print "参数错误,数据库名称不能有中杠"
                os.chdir(child_path)
                os.system("rm -rf script_file")
                os._exit(1)
        all_file = ['base_port']
        game_server_file = ['scened_port']
        logind_file = ['logind_port']
        chatd_file = ['chatd_port']
        mirrord_file = ['mirrord_port']
        common_resourced_file = ['common_resourced_port']
        routed_file = ['routed_ports']
        if os.path.exists(child_path + "/script_file/all"):
                for af in all_file:
                        check_error(af)
        if (os.path.exists(child_path + "/script_file/game_server") or os.path.exists(child_path + "/script_file/scened")):
                for gsf in game_server_file:
                        check_error(gsf)
        if os.path.exists(child_path + "/script_file/logind"):
                for lf in logind_file:
                        check_error(lf)
        if os.path.exists(child_path + "/script_file/chatd"):
                for cf in chatd_file:
                        check_error(cf)
        if os.path.exists(child_path + "/script_file/mirrord"):
                for mf in mirrord_file:
                        check_error(mf)
        if os.path.exists(child_path + "/script_file/common_resourced"):
                for crf in common_resourced_file:
                        check_error(crf)
        if os.path.exists(child_path + "/script_file/routed"):
                for rf in routed_file:
                        check_error(rf)

def check_variables(mysql_ip, mysql_username, mysql_password):
        if set_process == "all" or set_process == "game_server" or set_process == "scened" or set_process == "dbcached" or set_process == "logind":
                sql_vriables_name = [
                        'character_set_client',
                        'character_set_connection',
                        'character_set_database',
                        'character_set_results',
                        'character_set_server',
                        'character_set_system',
                        'max_allowed_packet']
                variables = {}
                sql = str("mysql -h " + mysql_ip + " -u" + mysql_username + " -p" + mysql_password + " -e 'show variables'>sql_variables")
                os.system(sql)
                with open('sql_variables', 'r') as line:
                        for le in line:
                                str_list = le.strip().split('\t')
                                if len(str_list) >= 2:
                                        variables[str_list[0]] = str_list[1]
                for vn in sql_vriables_name:
                        if vn == 'max_allowed_packet':
                                if int(variables[vn]) < 1073741824:
                                        print "\n%s is not 1G\n"%vn
                                        os._exit(1)
                        else:
                                if variables[vn] == 'utf8':
                                        continue
                                else:
                                        print "\n%s not utf8\n"%vn
                                        os._exit(1)
                os.system("rm sql_variables")

def set_listen(listen):
        if os.path.exists(child_path + "/script_file") == False:
                os.system("mkdir script_file")
        os.chdir(child_path + "/script_file/")
        if listen == '1':
                if os.path.exists("listend"):
                        os.system("rm listend")
        else:
                if os.path.exists("listend") == False:
                        os.system("touch listend")

def check_path():
        current_path = os.getcwd()
        if "bin/managescripts" not in current_path:
                print "运行路径不正确，请在 bin/managescripts 路径下运行"
                os._exit(1)
def check_ip(value, ip_name):
        if value.count(".") != 3 and value.count(".") != 2:
                print "%s 参数格式不正确" %ip_name
                os._exit(1)

def set_routed_param(param_type, value):
        global routed_ports
        global routed_ips
        values = str(value).split(',')
        if param_type == "port":
                routed_ports = []
                write_script_file("routed_ports", ','.join(values))
                for vs in values:
                        routed_ports.append(vs)
        if param_type == "ip":
                routed_ips = []
                for vs in values:
                        check_ip(vs, "routed_ip")
                        routed_ips.append(vs)

def set_config(key, value):
        config_info[key] = value

def check_process_id(cur_id, start_id, end_id):
        if (int(cur_id) < start_id or int(cur_id) >= end_id):
                return True
        return False

def set_params():
        params = ['SDK_CHECK_LOGIN', 'COST_FREE', 'LOG_LEVEL', 'SCENED_DEAMON', 'DEV_GM', 'MINICORE', 'LOGIND_DEAMON']
        for ci in config_info:
                if ci in params:
                        if ci == 'LOG_LEVEL':
                                if config_info[ci] == "0":
                                        set_config(ci, "TRACE")
                                if config_info[ci] == "1":
                                        set_config(ci, "DEBUG")
                                if config_info[ci] == "2":
                                        set_config(ci, "INFO")
                                if config_info[ci] == "3":
                                        set_config(ci, "ERROR")
                        else:
                                if config_info[ci] == "0":
                                        set_config(ci, 'false')
                                else:
                                        set_config(ci, 'true')

def configure_help():
        print "congigure 使用说明\n"
        print "%-40s启动进程名称" %"--set-process"
        print "%-40sssl证书路径" %"--ssl-cert"
        print "%-40sssl秘钥路径" %"--ssl-private-key"
        print "%-40sssl验证路径" %"--ssl-verify"
        print "%-40s启动所有进程,默认起始端口为7017,其他端口在该基础上递增,如果不设置--port则使用端口7017作为默认起始端口,其他端口在此基础上递增,否则使用设置的端口作为默认参数,其他端口在此基础上递增" %"--set-process=all"
        print "%-40s启动游戏服,scened进程和dbcached进程" %"--set-process=game_server"
        print "%-40s启动游戏服,scened进程" %"--set-process=scened"
        print "%-40s启动缓存服,dbcached进程" %"--set-process=dbcached"
        print "%-40s启动聊天服,chatd进程" %"--set-process=chatd"
        print "%-40s启动登陆服,logind进程" %"--set-process=logind"
        print "%-40s启动GM服,gmd进程" %"--set-process=gmd"
        print "%-40s启动日志服,log_serverd进程" %"--set-process=log_serverd"
        print "%-40s日志等级(0:TRACE    1:DEBUG    2:INFO    3:ERROR), 默认参数为2" %"--log-level"
        print "%-40s是否开启日志数据库(0:不开启    1:开启), 默认参数为0" %"--write-dblog"
        print "%-40s是否上传BI数据(0:不上传    1:上传), 默认参数为0" %"--send-bi"
        print "%-40s是否跳过新手引导(0:不跳过    1:跳过), 默认参数为0" %"--skip-xinshou"
        print "%-40s是否启动监听进程,默认状态不启动监听(1:启动监听, 0:不启动监听),默认参数为0" %"--start-listen"
        print "%-40s回复客户端延迟，单位毫秒，默认参数为0ms" %"--lag"
        print "%-40s是否开启arena(0:不开启    1:开启), 默认参数为1" %"--scene-info-arena"
        print "%-40s是否开启fief(0:不开启    1:开启), 默认参数为1" %"--scene-info-fief"
        print "%-40s是否开启mansion(0:不开启    1:开启), 默认参数为1" %"--scene-info-mansion"
        print "%-40s是否开启guanpin(0:不开启    1:开启), 默认参数为1" %"--scene-info-guanpin"
        print "%-40s是否开启huanzhuang-pvp(0:不开启    1:开启), 默认参数为1" %"--scene-info-huanzhuang-pvp"
        print "%-40sall-base-port起始端口,在--set-process=all情况下使用，默认为7017\n" %"--all-base-port"

        print "%-40sgameserver ID默认1, 范围0~2047" %"--server-id"
        print "%-40sdbcached ID默认20000, 范围20000~29999" %"--dbcached-id"
        print "%-40slogserverd ID默认70000, 范围70000~79999" %"--logserverd-id"
        print "%-40sscened是否为守护进程,默认状态是守护进程(1:守护进程, 0:不是守护进程),默认参数为1" %"--scened-deamon"
        print "%-40s是否使用迷你宕机转储文件,默认参数为0（0:不使用, 1:使用）" %"--minicore"
        print "%-40splayer线程数量，默认为4" %"--scene-player-thread-count"
        print "%-40s平台SDK登陆验证开关，默认为1（0:LOGIN_CHECK关闭, 1:LOGIN_CHECK打开）" %"--login-sdk-check"
        print "%-40slogind是否为守护进程,默认状态是守护进程(1:守护进程, 0:不是守护进程),默认参数为1" %"--logind-deamon"
        print "%-40s热更新最低版本号和小于最低版本号, 默认值1.0.048" %"--hot-update-client-version"
        print "%-40s客户端版本号" %"--client-version"
        print "%-40s是否消耗资源，默认为0（0: 关闭, 1: 打开）" %"--cost-free"
        print "%-40s收益增益倍数，默认为1" %"--gain-times"
        print "%-40s允许机器人登录，默认为0" %"--allow-robot"
        print "%-40s机器人慢速执行，默认为1（0：否，1：是）" %"--robot-slow"
        print "%-40stestin测试专用，默认为0" %"--testin"
        print "%-40s服务器是否开弱网，默认为0(0: 不开，1：开)" %"--weak-connect"
        print "%-40sGM聊天开关，默认为0（0:GM关闭, 1:GM打开）" %"--dev-gm"
        print "%-40sscened最大角色数量,默认参数为5000" %"--scened-max-roles"
        print "%-40sscened最大府邸数量,默认参数为6000" %"--scened-max-mansions"
        print "%-40sscened最大封地数量,默认参数为6000" %"--scened-max-fiefs"
        print "%-40sscened最大竞技者数量,默认参数为7000" %"--scened-max-gladiators"
        print "%-40sscened最大官的数量,默认参数为5000" %"--scened-max-guans"
        print "%-40s最大家族数, 默认参数为300" %"--scened-max-leagues"
        print "%-40s游戏数据库名称, 默认为game_jxwy" %"--game-db"
        print "%-40s游戏数据库用户名, 默认为nora" %"--game-mysql-username"
        print "%-40s游戏数据库密码,与游戏数据库用户名匹配,默认为1234" %"--game-mysql-password"
        print "%-40s游戏数据库IP, 默认为127.0.0.1" %"--game-mysql-ip"
        print "%-40s游戏数据库PORT, 默认为3306" %"--game-mysql-port"
        print "%-40s日志数据库名称, 默认为log_jxwy" %"--log-db"
        print "%-40s日志数据库用户名,默认为nora" %"--log-mysql-username"
        print "%-40s日志数据库密码,与日志数据库用户名匹配,默认为1234" %"--log-mysql-password"
        print "%-40s日志数据库IP,默认为127.0.0.1" %"--log-mysql-ip"
        print "%-40s日志数据库PORT, 默认为3306" %"--log-mysql-port"
        print "%-40sapp_id, 默认为100000001" %"--app-id"
        print "%-40sapp_secret,默认为0ad15dc69cb015db790ccea3df4c923c" %"--app-secret"
        print "%-40s运营login地址,默认地址为http://kslogin.ksgame.com/login/login-check" %"--yunying-login-addr"
        print "%-40s运营BI地址,默认地址为http://115.159.189.146:8004" %"--yunying-bi-addr"
        print "%-40s运营gm地址,默认地址为http://115.159.189.146:8020\n" %"--yunying-gm-addr"

        print "%-40sscened-logserverd连接端口" %"--scened-logserverd-port"
        print "%-40sscened-client连接端口" %"--scened-client-port"
        print "%-40sscened-dbcached连接端口" %"--scened-dbcached-port"
        print "%-40sscened-public-arenad连接端口," %"--scened-public-arenad-port"
        print "%-40sscened-logserverd连接IP, 默认为0.0.0.0" %"--scened-logserverd-ip"
        print "%-40sscened-client连接IP, 默认为0.0.0.0" %"--scened-client-ip"
        print "%-40sscened-dbcached连接IP, 默认为127.0.0.1" %"--scened-dbcached-ip"
        print "%-40sscened-public-arenad连接IP, 默认为127.0.0.1\n" %"--scened-public-arenad-ip"

        print "%-40schatd ID默认50000, 范围50000~59999" %"--chatd-id"
        print "%-40schatd-scened-port连接端口,可设置多个端口号" %"--chatd-scened-port"
        print "%-40schatd-scened-ip连接IP\n" %"--chatd-scened-ip"

        print "%-40smirrord ID默认60000, 范围60000~69999" %"--mirrord-id"
        print "%-40smirrord-scened-ip连接端口" %"--mirrord-scened-port"
        print "%-40smirrord-scened-port连接IP\n" %"--mirrord-scened-ip"

        print "%-40slogind ID默认30000, 范围30000~39999" %"--logind-id"
        print "%-40slogind-client连接端口" %"--logind-client-port"
        print "%-40slogind-scened连接端口" %"--logind-scened-port"
        print "%-40slogind-gmd连接端口" %"--logind-gmd-port"
        print "%-40slogind-client连接IP, 默认为0.0.0.0" %"--logind-client-ip"
        print "%-40slogind-scened连接IP\n" %"--logind-scened-ip"
        print "%-40slogind-gmd连接IP\n" %"--logind-gmd-ip"

        print "%-40sgmd ID默认40000, 范围40000~49999" %"--gmd-id"
        print "%-40sgmd-yunying连接端口" %"--gmd-yunying-port"
        print "%-40sgmd-scened连接端口" %"--gmd-scened-port"
        print "%-40sgmd-scened连接IP" %"--gmd-scened-ip"
        print "%-40sgmd-yunying连接IP, 默认为0.0.0.0\n" %"--gmd-yunying-ip"

        print "%-40srouted ID默认80000, 范围80000~89999" %"--routed-id"
        print "%-40s路由服端口,除routed进程之外的其他进程可以配置多个，逗号区分" %"--routed-ports"
        print "%-40s路有服连接IP,除routed进程之外的其他进程可以配置多个，逗号区分\n" %"--routed-ips"

        print "%-40s登录数据库名称, 默认为login_jxwy" %"--login-db"
        print "%-40s登录数据库IP,默认为127.0.0.1" %"--login-mysql-ip"
        print "%-40s登录数据库PORT, 默认为3306" %"--login-mysql-port"
        print "%-40s登录数据库用户名,默认为nora" %"--login-mysql-username"
        print "%-40s登录数据库密码,与日志数据库用户名匹配,默认为1234\n" %"--login-mysql-password"

        print "%-40scommon-resourced ID默认90000, 范围90000~99999" %"--common-resourced-id"
        print "%-40s登录数据库名称, 默认为common_resource_jxwy" %"--common-resource-db"
        print "%-40s登录数据库IP,默认为127.0.0.1" %"--common-resource-mysql-ip"
        print "%-40s登录数据库PORT, 默认为3306" %"--common-resource-mysql-port"
        print "%-40s登录数据库用户名,默认为nora" %"--common-resource-mysql-username"
        print "%-40s登录数据库密码,与日志数据库用户名匹配,默认为1234\n" %"--common-resource-mysql-password"

        print "%-40sdbcache解包线程数量，默认为4" %"--dbcache-parse-thread-count"
        print "%-40sdbcached缓存最大角色数量,默认参数为6000" %"--dbcached-max-roles"
        print "%-40sdbcached缓存最大府邸数量,默认参数为7000" %"--dbcached-max-mansions"
        print "%-40sdbcached缓存最大封地数量,默认参数为7000" %"--dbcached-max-fiefs"
        print "%-40sdbcached缓存最大竞技者数量,默认参数为8000\n" %"--dbcached-max-gladiators"
        print "%-40sdbcached缓存最官的数量,默认参数为6000\n" %"--dbcached-max-guans"
        print "%-40sdbcached缓存最大家族的数量,默认参数为300\n" %"--dbcached-max-leagues"
        print "\ngame_server:./configure.py --game-db=game_jxwy --game-mysql-ip=127.0.0.1 --game-mysql-port=3306 --game-mysql-username=nora --game-mysql-password=1234 --log-db=log_jxwy --log-mysql-ip=127.0.0.1 --log-mysql-port=3306 --log-mysql-username=nora --log-mysql-password=1234 --log-level=0 --start-listen=0 --scened-deamon=1 --server-id=1 --scene-player-thread-count=4 --dbcache-parse-thread-count=4 --scened-client-port=7017 --scened-client-ip=0.0.0.0 --login-sdk-check=1 --cost-free=0 --gain-times=1 --allow-robot=0 --testin=0 --dev-gm=0 --minicore=0 --write-dblog=1 --send-bi=1 --weak-connection=0 --skip-xinshou=0 --lag=100 --app-id=100000017 --app-secret=44c776cc85bd577de78f81dfa4e4d8b4 --set-process=game_server\n"
        print "scened:./configure.py --game-db=game_jxwy --game-mysql-ip=127.0.0.1 --game-mysql-port=3306 --game-mysql-username=nora --game-mysql-password=1234 --log-db=log_jxwy --log-mysql-ip=127.0.0.1 --log-mysql-port=3306 --log-mysql-username=nora --log-mysql-password=1234 --log-level=0 --start-listen=0 --scened-deamon=1 --server-id=1 --scene-player-thread-count=4 --scened-client-port=7017 --scened-logserverd-port=7027 --scened-dbcached-port=7024 --scened-public-arenad-port=7026 --scened-client-ip=0.0.0.0 --scened-logserverd-ip=0.0.0.0 --scened-dbcached-ip=127.0.0.1 --scened-public-arenad-ip=127.0.0.1 --login-sdk-check=1 --cost-free=0 --gain-times=1 --allow-robot=0 --testin=0 --dev-gm=0 --minicore=0 --write-dblog=1 --send-bi=1 --weak-connection=0 --skip-xinshou=0 --lag=100 --app-id=100000017 --app-secret=44c776cc85bd577de78f81dfa4e4d8b4 --set-process=scened\n"
        print "dbcached:./configure.py --dbcached-id=20000 --game-db=game_jxwy --game-mysql-ip=127.0.0.1 --game-mysql-port=3306 --game-mysql-username=nora --game-mysql-password=1234 --log-db=log_jxwy --log-mysql-ip=127.0.0.1 --log-mysql-port=3306 --log-mysql-username=nora --log-mysql-password=1234 --log-level=0 --start-listen=0 --dbcache-parse-thread-count=4 --scened-dbcached-port=7024 --scened-dbcached-ip=127.0.0.1 --set-process=dbcached\n"
        print "chatd:./configure.py --chatd-id=50000 --chatd-scened-port=7020 --routed-ports=7023 --routed-ips=127.0.0.1 --chatd-scened-ip=0.0.0.0 --log-level=0 --start-listen=0 --set-process=chatd\n"
        print "logind:./configure.py --logind-id=30000 --login-db=login_jxwy --login-mysql-ip=127.0.0.1 --login-mysql-port=3306 --login-mysql-username=nora --login-mysql-password=1234 --logind-client-port=7018 --logind-scened-port=7021 --logind-gmd-port=7030 --routed-ports=7023 --routed-ips=127.0.0.1 --logind-client-ip=0.0.0.0 --logind-scened-ip=127.0.0.1  --logind-gmd-ip=127.0.0.1 --log-level=0 --start-listen=0 --set-process=logind\n"
        print "gmd:./configure.py --gmd-id=40000 --gmd-yunying-port=7025 --gmd-scened-port=7019 --routed-ports=7023 --routed-ips=127.0.0.1 --gmd-scened-ip=0.0.0.0 --gmd-yunying-ip=0.0.0.0 --gm-db=gm_jxwy --gm-mysql-ip=127.0.0.1 --gm-mysql-port=3306 --gm-mysql-username=nora --gm-mysql-password=1234 --log-level=0 --start-listen=0 --set-process=gmd\n"
        print "mirrord:./configure.py --mirrord-id=60000 --mirrord-scened-port=7022 --mirrord-scened-ip=0.0.0.0 --routed-ips=127.0.0.1 --routed-ports=7023 --log-level=0 --start-listen=0 --set-process=mirrord\n"
        print "log_serverd:./configure.py --logserverd-id=70000 --scened-logserverd-port=7027 --scened-logserverd-ip=0.0.0.0 --app-id=100000017 --app-secret=44c776cc85bd577de78f81dfa4e4d8b4 --log-level=0 --start-listen=0 --set-process=log_serverd\n"
        print "routed:./configure.py --routed-id=80000 --routed-ports=7023 --routed-ips=0.0.0.0 --log-level=0 --set-process=routed\n"
        print "common_resourced:./configure.py --common-resourced-id=90000 --common-resource-scened-port=7029 --routed-ports=7023 --routed-ips=127.0.0.1 --common-resource-scened-ip=0.0.0.0 --common-resource-db=common_resource_jxwy --common-resource-mysql-ip=127.0.0.1 --common-resource-mysql-port=3306 --common-resource-mysql-username=nora --common-resource-mysql-password=1234 --set-process=common_resourced\n"
        print "all:./configure.py --all-base-port=7017 --game-db=game_jxwy --game-mysql-ip=127.0.0.1 --game-mysql-port=3306 --game-mysql-username=nora --game-mysql-password=1234 --log-db=log_jxwy --log-mysql-ip=127.0.0.1 --log-mysql-port=3306 --log-mysql-username=nora --log-mysql-password=1234 --login-db=login_jxwy --login-mysql-ip=127.0.0.1 --login-mysql-port=3306 --login-mysql-username=nora --login-mysql-password=1234 --log-level=0 --start-listen=0 --scened-deamon=1 --server-id=10000 --scene-player-thread-count=4 --dbcache-parse-thread-count=4 --login-sdk-check=1 --cost-free=0 --gain-times=1 --allow-robot=0 --testin=0 --dev-gm=1 --minicore=0 --write-dblog=1 --send-bi=1 --weak-connection=0 --skip-xinshou=0 --lag=100 --app-id=100000017 --app-secret=44c776cc85bd577de78f81dfa4e4d8b4 --common-resource-db=common_resource_jxwy --common-resource-mysql-ip=127.0.0.1 --common-resource-mysql-port=3306 --common-resource-mysql-username=nora --common-resource-mysql-password=1234 --gm-db=gm_jxwy --gm-mysql-ip=127.0.0.1 --gm-mysql-port=3306 --gm-mysql-username=nora --gm-mysql-password=1234 --gmd-id=40001 --chatd-id=50001 --logind-id=30001 --mirrord-id=60001 --common-resourced-id=90001 --set-process=all\n"
        print "注意：在选项--set-process后面的参数不为all的情况下, 不可以使用--all-base-port选项, 启动某一个进程时只需要配置与该进程相关的参数即可\n"
        os._exit(1)

try:
        params_list = [
                "all-base-port=",
                "game-db=",
                "game-mysql-ip=",
                "game-mysql-port=",
                "game-mysql-username=",
                "game-mysql-password=",
                "log-db=",
                "log-mysql-ip=",
                "log-mysql-port=",
                "log-mysql-username=",
                "log-mysql-password=",
                "login-db=",
                "login-mysql-ip=",
                "login-mysql-port=",
                "login-mysql-username=",
                "login-mysql-password=",
                "gm-db=",
                "gm-mysql-ip=",
                "gm-mysql-port=",
                "gm-mysql-username=",
                "gm-mysql-password=",
                "common-resource-db=",
                "common-resource-mysql-ip=",
                "common-resource-mysql-port=",
                "common-resource-mysql-username=",
                "common-resource-mysql-password=",
                "dbcache-max-role=",
                "log-level=",
                "start-listen=",
                "scened-deamon=",
                "logind-deamon=",
                "hot-update-client-version=",
                "server-id=",
                "scene-player-thread-count=",
                "dbcache-parse-thread-count=",
                "login-sdk-check=",
                "cost-free=",
                "gain-times=",
                "allow-robot=",
                "robot-slow=",
                "testin=",
                "dev-gm=",
                "minicore=",
                "dbcached-max-roles=",
                "dbcached-max-mansions=",
                "dbcached-max-fiefs=",
                "dbcached-max-gladiators=",
                "dbcached-max-guans=",
                "dbcached-max-leagues=",
                "scened-max-roles=",
                "scened-max-mansions=",
                "scened-max-fiefs=",
                "scened-max-gladiators=",
                "scened-max-guans=",
                "scened-max-leagues=",
                "yunying-bi-addr=",
                "yunying-gm-addr=",
                "yunying-login-addr=",
                "logind-client-port=",
                "chatd-scened-port=",
                "gmd-yunying-port=",
                "gmd-admin-port=",
                "gmd-scened-port=",
                "common-resource-scened-port=",
                "routed-ports=",
                "scened-client-port=",
                "scened-logserverd-port=",
                "scened-dbcached-port=",
                "mirrord-scened-port=",
                "scened-public-arenad-port=",
                "chatd-scened-ip=",
                "scened-client-ip=",
                "scened-logserverd-ip=",
                "scened-dbcached-ip=",
                "logind-client-ip=",
                "gmd-scened-ip=",
                "common-resourced-id=",
                "common-resource-scened-ip=",
                "routed-ips=",
                "gmd-yunying-ip=",
                "gmd-admin-ip=",
                "mirrord-scened-ip=",
                "logind-scened-ip=",
                "logind-scened-port=",
                "logind-gmd-ip=",
                "logind-gmd-port=",
                "scened-public-arenad-ip=",
                "app-id=",
                "app-secret=",
                "set-process=",
                "write-dblog=",
                "auto-recharge=",
                "skip-xinshou=",
                "send-bi=",
                "weak-connection=",
                "lag=",
		"ssl-cert=",
                "ssl-private-key=",
                "logind-id=",
                "dbcached-id=",
                "gmd-id=",
                "routed-id=",
                "mirrord-id=",
                "chatd-id=",
                "logserverd-id=",
                "client-version=",
                "ssl-verify="]
        opts, args = getopt.getopt(sys.argv[1:], "h", params_list)
        for op, value in opts:
                if os.path.exists(child_path + "/script_file"):
                        clear_configure()
                if op == "--all-base-port":
                        base_port = value
                elif op == "--game-db":
                        set_config("GAME_DB_DATABASE", value)
                elif op == "--game-mysql-ip":
                        set_config("GAME_DB_IP", value)
                elif op == "--game-mysql-port":
                        set_config("GAME_DB_PORT", value)
                elif op == "--game-mysql-username":
                        set_config("GAME_USER_NAME_DB", value)
                elif op == "--game-mysql-password":
                        set_config("GAME_PASSWORD_DB", value)
                elif op == "--log-db":
                        set_config("LOG_DB_DATABASE", value)
                elif op == "--log-mysql-ip":
                        set_config("LOG_DB_IP", value)
                elif op == "--log-mysql-port":
                        set_config("LOG_DB_PORT", value)
                elif op == "--log-mysql-username":
                        set_config("LOG_USER_NAME_DB", value)
                elif op == "--log-mysql-password":
                        set_config("LOG_PASSWORD_DB", value)
                elif op == "--login-db":
                        set_config("LOGIN_DB_DATABASE", value)
                elif op == "--login-mysql-ip":
                        set_config("LOGIN_DB_IP", value)
                elif op == "--login-mysql-port":
                        set_config("LOGIN_DB_PORT", value)
                elif op == "--login-mysql-username":
                        set_config("LOGIN_USER_NAME_DB", value)
                elif op == "--login-mysql-password":
                        set_config("LOGIN_PASSWORD_DB", value)
                elif op == "--gm-db":
                        set_config("GM_DB_DATABASE", value)
                elif op == "--gm-mysql-ip":
                        set_config("GM_DB_IP", value)
                elif op == "--gm-mysql-port":
                        set_config("GM_DB_PORT", value)
                elif op == "--gm-mysql-username":
                        set_config("GM_DB_USER", value)
                elif op == "--gm-mysql-password":
                        set_config("GM_DB_PASSWORD", value)
                elif op == "--common-resource-db":
                        set_config("COMMON_RESOURCE_DB_DATABASE", value)
                elif op == "--common-resource-mysql-ip":
                        set_config("COMMON_RESOURCE_DB_IP", value)
                elif op == "--common-resource-mysql-port":
                        set_config("COMMON_RESOURCE_DB_PORT", value)
                elif op == "--common-resource-mysql-username":
                        set_config("COMMON_RESOURCE_DB_USER", value)
                elif op == "--common-resource-mysql-password":
                        set_config("COMMON_RESOURCE_DB_PASSWORD", value)
                elif op == "--login-sdk-check":
                        if (value == "0" or value == "1") == False:
                                print "\n--login-sdk-check=%s 参数配置错误"%value
                                os._exit(1)
                        set_config("SDK_CHECK_LOGIN", value)
                elif op == "--cost-free":
                        if (value == "0" or value == "1") == False:
                                print "\n--cost-free=%s 参数配置错误"%value
                                os._exit(1)
                        set_config("COST_FREE", value)
                elif op == "--allow-robot":
                        if value == "1":
                                set_config("ALLOW_ROBOT", "true")
                        if value == "0":
                                set_config("ALLOW_ROBOT", "false")
                elif op == "--robot-slow":
                        if value == "1":
                                set_config("ROBOT_SLOW", "true")
                        if value == "0":
                                set_config("ROBOT_SLOW", "false")
                elif op == "--testin":
                        if value == "1":
                                set_config("TESTIN", "true")
                        if value == "0":
                                set_config("TESTIN", "false")
                elif op == "--gain-times":
                        set_config("GAIN_TIMES", value)
                elif op == "--log-level":
                        if (value == "0" or value == "1" or value == "2" or value == "3") == False:
                                print "\n--log-level=%s 参数配置错误"%value
                                os._exit(1)
                        set_config("LOG_LEVEL", value)
                elif op == "--start-listen":
                        set_listen(value)
                elif op == "--scened-deamon":
                        if (value == "0" or value == "1") == False:
                                print "\n--scened-deamon=%s 参数配置错误"%value
                                os._exit(1)
                        set_config("SCENED_DEAMON", value)
                elif op == "--logind-deamon":
                        if (value == "0" or value == "1") == False:
                                print "\n--logind-deamon=%s 参数配置错误"%value
                                os._exit(1)
                        set_config("LOGIND_DEAMON", value)
                elif op == "--hot-update-client-version":
                        set_config("HOT_UPDATE_CLIENT_VERSION", value)
                        print config_info['HOT_UPDATE_CLIENT_VERSION']
                elif op == "--server-id":
                        if (check_process_id(value, 0, 2047)):
                                print "\nserver-id参数错误\n"
                                os._exit(1)
                        set_config("SERVER_ID", value)
                        write_script_file("server_id", str(value))
                elif op == "--scene-player-thread-count":
                        set_config("SCENE_PLAYER_THREAD_COUNT", value)
                        write_script_file("scene_player_thread_count", str(value))
                elif op == "--dbcache-parse-thread-count":
                        set_config("DBCACHE_PARSE_THREAD_COUNT", value)
                        write_script_file("dbcache_parse_thread_count", str(value))
                elif op == "--dbcached-max-leagues":
                        set_config("DBCACHED_MAX_LEAGUES", value)
                        write_script_file("dbcache_parse_thread_count", str(value))
                elif op == "--dev-gm":
                        if (value == "0" or value == "1") == False:
                                print "\n--dev-gm=%s 参数配置错误"%value
                                os._exit(1)
                        set_config("DEV_GM", value)
                elif op == "--minicore":
                        if (value == "0" or value == "1") == False:
                                print "\n--minicore=%s 参数配置错误"%value
                                os._exit(1)
                        set_config("MINICORE", value)
                elif op == "--dbcached-max-roles":
                        set_config("DBCACHED_MAX_ROLES", value)
                elif op == "--dbcached-max-mansions":
                        set_config("DBCACHED_MAX_MANSIONS", value)
                elif op == "--dbcached-max-fiefs":
                        set_config("DBCACHED_MAX_FIEFS", value)
                elif op == "--dbcached-max-gladiators":
                        set_config("DBCACHED_MAX_GLADIATORS", value)
                elif op == "--dbcached-max-guans":
                        set_config("DBCACHED_MAX_GUANS", value)
                elif op == "--dbcached-max-children":
                        set_config("DBCACHED_MAX_CHILDREN", value)
                elif op == "--scened-max-roles":
                        set_config("SCENED_MAX_ROLES", value)
                elif op == "--scened-max-mansions":
                        set_config("SCENED_MAX_MANSIONS", value)
                elif op == "--scened-max-fiefs":
                        set_config("SCENED_MAX_FIEFS", value)
                elif op == "--scened-max-gladiators":
                        set_config("SCENED_MAX_GLADIATORS", value)
                elif op == "--scened-max-guans":
                        set_config("SCENED_MAX_GUANS", value)
                elif op == "--scened-max-children":
                        set_config("SCENED_MAX_CHILDREN", value)
                elif op == "--yunying-bi-addr":
                        set_config("YUNYING_BI_ADDR", value)
                elif op == "--yunying-gm-addr":
                        set_config("YUNYING_GM_ADDR", value)
                elif op == "--yunying-login-addr":
                        set_config("YUNYING_LOGIN_ADDR", value)
                elif op == "--logind-client-port":
                        set_config("LOGIND_CLIENT_PORT", value)
                        write_script_file("logind_port", str(value))
                elif op == "--chatd-scened-port":
                        set_config("CHAT_SCENED_PORT", value)
                        write_script_file("chatd_port", str(value))
                elif op == "--mirrord-scened-port":
                        set_config("MIRROR_SCENED_PORT", value)
                        write_script_file("mirrord_port", str(value))
                elif op == "--gmd-admin-port":
                        set_config("GMD_ADMIN_PORT", value)
                elif op == "--gmd-yunying-port":
                        set_config("GMD_YUNYING_PORT", value)
                elif op == "--gmd-scened-port":
                        set_config("GM_SCENED_PORT", value)
                        write_script_file("gmd_port", str(value))
                elif op == "--common-resource-scened-port":
                        set_config("COMMON_RESOURCE_SCENED_PORT", value)
                        write_script_file("common_resourced_port", str(value))
                elif op == "--routed-ports":
                        set_routed_param("port", value);
                elif op == "--scened-client-port":
                        set_config('SCENED_CLIENT_PORT', value)
                        write_script_file("scened_port", str(value))
                elif op == "--scened-logserverd-port":
                        set_config('SCENED_LOGSERVERD_PORT', value)
                        write_script_file("log_serverd_port", str(value))
                elif op == "--scened-dbcached-port":
                        set_config('SCENED_DBCACHED_PORT', value)
                        write_script_file("dbcached_port", str(value))
                elif op == "--logind-scened-port":
                        set_config('LOGIN_SCENED_PORT', value)
                elif op == "--logind-gmd-port":
                        set_config('LOGIN_GMD_PORT', value)
                elif op == "--scened-public-arenad-port":
                        set_config("SCENED_PUBLIC_ARENAD_PORT", value)
                elif op == "--scened-client-ip":
                        set_config("SCENED_CLIENT_IP", value)
                        check_ip(value, "scened_client_ip")
                elif op == "--scened-logserverd-ip":
                        set_config("SCENED_LOGSERVERD_IP", value)
                        check_ip(value, "scened_log_serverd_ip")
                elif op == "--scened-dbcached-ip":
                        set_config("SCENED_DBCACHED_IP", value)
                        check_ip(value, "scened_dbcached_ip")
                elif op == "--chatd-scened-ip":
                        set_config("CHAT_SCENED_IP", value)
                        check_ip(value, "chat_ip")
                elif op == "--mirrord-scened-ip":
                        set_config("MIRROR_SCENED_IP", value)
                        check_ip(value, "mirror_ip")
                elif op == "--logind-client-ip":
                        set_config("LOGIND_CLIENT_IP", value)
                        check_ip(value, "logind_client_ip")
                elif op == "--gmd-scened-ip":
                        set_config("GM_SCENED_IP", value)
                        check_ip(value, "gm_ip")
                elif op == "--common-resourced-id":
                        set_config("COMMON_RESOURCED_ID", value)
                elif op == "--common-resource-scened-ip":
                        set_config("COMMON_RESOURCE_SCENED_IP", value)
                        check_ip(value, "common_resource_ip")
                elif op == "--routed-ips":
                        set_routed_param("ip", value);
                elif op == "--gmd-admin-ip":
                        set_config("GMD_ADMIN_IP", value)
                        check_ip(value, "gmd_admin_ip")
                elif op == "--gmd-yunying-ip":
                        set_config("GMD_YUNYING_IP", value)
                        check_ip(value, "gmd_yunying_ip")
                elif op == "--logind-scened-ip":
                        set_config("LOGIN_SCENED_IP", value)
                        check_ip(value, "login_ip")
                elif op == "--logind-gmd-ip":
                        set_config("LOGIN_GMD_IP", value)
                        check_ip(value, "login_ip")
                elif op == "--scened-public-arenad-ip":
                        set_config("SCENED_PUBLIC_ARENAD_IP", value)
                        check_ip(value, "scened_public_arenad_ip")
                elif op == "--app-id":
                        set_config("APP_ID", value)
                elif op == "--write-dblog":
                        if value == "1":
                                set_config("WRITE_DBLOG", "true")
                elif op == "--auto-recharge":
                        if value == "1":
                                set_config("AUTO_RECHARGE", "true")
                elif op == "--skip-xinshou":
                        if value == "1":
                                set_config("SKIP_XINSHOU", "true")
                        else:
                                set_config("SKIP_XINSHOU", "false")
                elif op == "--send-bi":
                        if value == "0":
                                set_config("SEND_BI", "false")
                        if value == "1":
                                set_config("SEND_BI", "true")
                elif op == "--weak-connection":
                        if value == "1":
                                set_config("WEAK_CONNECTION", "true")
                elif op == "--lag":
                        set_config("LAG", value)
                elif op == "--scened-max-leagues":
                        set_config("SCENED_MAX_LEAGUES", value)
                elif op == "--ssl-cert":
                        set_config("SSL_CERT", value)
                elif op == "--ssl-private-key":
                        set_config("SSL_PRIVATE_KEY", value)
                elif op == "--client-version":
                        set_config("CLIENT_VERSION", value)
                elif op == "--ssl-verify":
                        set_config("SSL_VERIFY", value)
                elif op == "--app-secret":
                        set_config("APP_SECRET", value)
                elif op == "--app-secret":
                        set_config("APP_SECRET", value)
                elif op == "--logind-id":
                        if (check_process_id(value, 30000, 40000)):
                                print "\nlogind-id参数错误\n"
                                os._exit(1)
                        set_config("LOGIND_ID", value)
                elif op == "--dbcached-id":
                        if (check_process_id(value, 20000, 30000)):
                                print "\ndbcached-id参数错误\n"
                                os._exit(1)
                        set_config("DBCACHED_ID", value)
                elif op == "--mirrord-id":
                        if (check_process_id(value, 60000, 70000)):
                                print "\nmirrord-id参数错误\n"
                                os._exit(1)
                        set_config("MIRRORD_ID", value)
                elif op == "--gmd-id":
                        if (check_process_id(value, 40000, 50000)):
                                print "\ngmd-id参数错误\n"
                                os._exit(1)
                        set_config("GMD_ID", value)
                elif op == "--routed-id":
                        if (check_process_id(value, 80000, 90000)):
                                print "\nroutedd-id参数错误\n"
                                os._exit(1)
                        set_config("ROUTED_ID", value)
                elif op == "--chatd-id":
                        if (check_process_id(value, 50000, 60000)):
                                print "\nchatd-id参数错误\n"
                                os._exit(1)
                        set_config("CHATD_ID", value)
                elif op == "--logserverd-id":
                        if (check_process_id(value, 70000, 80000)):
                                print "\nlohserverd-id参数错误\n"
                                os._exit(1)
                        set_config("LOGSERVERD_ID", value)
                elif op == "--set-process":
                        process_list = ['all', 'game_server', 'scened', 'dbcached', 'logind', 'chatd', 'gmd', 'mirrord', 'log_serverd', 'routed', 'common_resourced']
                        if value in process_list:
                                set_process = value
                                if os.path.exists(child_path + "/script_file") == False:
                                        os.system("mkdir script_file")
                                os.chdir(child_path + "/script_file/")
                                os.system("touch " + value)
                                if value == "all":
                                        for adp in all_default_port:
                                                set_config(adp, str(int(base_port) + int(all_default_port[adp])))
                                        for adi in all_default_ip:
                                                set_config(adi, all_default_ip[adi])
                                        set_routed_param("port", int(base_port) + 6)
                                        set_routed_param("ip", '127.0.0.1')
                                elif value == "game_server":
                                        for gsdp in game_server_default_port:
                                                if gsdp == 'SCENED_PUBLIC_ARENAD_PORT':
                                                        set_config(gsdp, str(int(config_info['SCENED_CLIENT_PORT']) + int(game_server_default_port[gsdp])))
                                                elif gsdp == 'SCENED_LOGSERVERD_PORT':
                                                        if config_info[gsdp] == '0':
                                                                set_config(gsdp, str(int(game_server_default_port[gsdp])))
                                                elif gsdp == 'SCENED_DBCACHED_PORT':
                                                        if config_info[gsdp] == '0':
                                                                set_config(gsdp, str(int(config_info['SCENED_CLIENT_PORT']) + int(game_server_default_port[gsdp])))
                                                else:
                                                        set_config(gsdp, str(int(game_server_default_port[gsdp])))

                        else:
                                print "\n --set-process参数错误，请执行 ./configure.py -h 查看参数说明\n"
                                os.system("rm -rf ../script_file")
                                os._exit(1)
except:
        print "\n参数错误"
        configure_help()

check_path()
check_param()
save_mysql_options()
if set_process == "all":
        check_variables(config_info['GAME_DB_IP'], config_info['GAME_USER_NAME_DB'], config_info['GAME_PASSWORD_DB'])
        check_variables(config_info['LOG_DB_IP'], config_info['LOG_USER_NAME_DB'], config_info['LOG_PASSWORD_DB'])
        check_variables(config_info['LOGIN_DB_IP'], config_info['LOGIN_USER_NAME_DB'], config_info['LOGIN_PASSWORD_DB'])
        check_variables(config_info['GM_DB_IP'], config_info['GM_DB_USER'], config_info['GM_DB_PASSWORD'])
        check_variables(config_info['COMMON_RESOURCE_DB_IP'], config_info['COMMON_RESOURCE_DB_USER'], config_info['COMMON_RESOURCE_DB_PASSWORD'])
if set_process == "game_server" or set_process == "scened" or set_process == "dbcached":
        check_variables(config_info['GAME_DB_IP'], config_info['GAME_USER_NAME_DB'], config_info['GAME_PASSWORD_DB'])
        check_variables(config_info['LOG_DB_IP'], config_info['LOG_USER_NAME_DB'], config_info['LOG_PASSWORD_DB'])
if set_process == "common_resourced":
        check_variables(config_info['COMMON_RESOURCE_DB_IP'], config_info['COMMON_RESOURCE_DB_USER'], config_info['COMMON_RESOURCE_DB_PASSWORD'])
if set_process == "gmd":
        check_variables(config_info['GM_DB_IP'], config_info['GM_DB_USER'], config_info['GM_DB_PASSWORD'])
if set_process == "logind":
        check_variables(config_info['LOGIN_DB_IP'], config_info['LOGIN_USER_NAME_DB'], config_info['LOGIN_PASSWORD_DB'])
check_configure_param()
print_info()
set_db_name()
set_params()
set_options_template()
if set_process == "all" or set_process == "game_server" or set_process == "scened" or set_process == "dbcached" or set_process == "logind" or set_process == "log_serverd" or set_process == "common_resourced" or set_process == "routed":
        print "configure 已经重新配置，如果是第一次部署请执行./recreate_db.py 脚本，否则不需要执行"
