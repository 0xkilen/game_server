# !/usr/bin/python
# -*- coding: UTF-8 -*-

import csv,codecs
import os
import sys
import re

reload(sys)
sys.setdefaultencoding('utf-8')

# common function
def format_ip(ip):
        ip_list = []
        for i in ip.split('.'):
                if int(i) < 10:
                        ip_list.append('00' + i)
                elif int(i) < 100:
                        ip_list.append('0' + i)
                else:
                        ip_list.append(i)

        return str('.'.join(ip_list))

def next_ip(ip):
        new_list = []
        ip_list = [int(i) for i in ip.split('.')]
        number = ip_list.pop() + 1
        if number > 255:
                new_list.append(0)
                if len(ip_list) > 0:
                        ip_list[len(ip_list) - 1] +=  1;

                while len(ip_list) > 0:
                        number = ip_list.pop()
                        if number > 255:
                                new_list.append(0)
                                if len(ip_list) > 0:
                                        ip_list[len(ip_list) - 1] +=  1;
                        else:
                                new_list.append(number);

                new_list = reversed(new_list)
        else:
                new_list = ip_list.extend([number])
                new_list = ip_list

        return format_ip('.'.join([str(x) for x in new_list]))

def read_file(file_path):
        if file_path == 'area.txt':
                # sed 's/要被取代的字串/新的字串/g'
                sed_replace = "sed -i 's/ //g;s/、/,/g;/^$/d' " + file_path
                os.system(sed_replace)

        file_data = []
        file = open(file_path)
        read_lines = 10000
        while True:
                lines = file.readlines(read_lines)
                if not lines:
                        break
                for line in lines:
                        if '\xef\xbb\xbf' in line:
                                file_data.append(line.strip().replace('\xef\xbb\xbf','')) # strip()去掉\n  用replace替换掉'\xef\xbb\xbf'
                        else:
                                file_data.append(line)

        return [x.encode('utf-8') for x in file_data]


# handle function
def parse_area_txt(file_data):
        area_dict = dict()
        zone_dict = dict()
        area_id = 1001
        for da in file_data:
                area = da.split(':')
                if len(area) < 2:
                        continue
                area_dict[area_id] = re.sub(re.compile('\s+'), '', area[0])
                zone_dict[area_id] = [re.sub(re.compile('\s+'), '', x) for x in area[1].split(',')]
                area_id += 1

        return area_dict, zone_dict

def parse_ip_txt(file_data):
        ips_list = []
        for da in file_data:
                ip_info = []
                for x in da.split(" "):
                        if x != "":
                                ip_info.append(re.sub(re.compile('\s+'), '', x))
                        if len(ip_info) >= 3:
                                ips_list.append(ip_info)
                                break

        return ips_list;

# IP 分组
def group_ips(area_dict, zone_dict, ips_list):
        progress = 0
        sum_total = len(ips_list)

        area_ips = {}
        for data in ips_list:
                progress += 1
                success = False
                ip_name = data[2].decode('utf-8')
                for area_id, zones in zone_dict.items():
                        for name in zones:
                                name = name.decode('utf-8')
                                if len(ip_name) >= len(name):
                                        name = ip_name[0:len(name)]
                                        if name in zones:
                                                data = data[0:2]
                                                data.append(area_id)
                                                data.append(area_dict[area_id])

                                                if area_ips.has_key(area_id) == True:
                                                        area_ips[area_id].append(data)
                                                else:
                                                        area_ips[area_id] = []
                                                        area_ips[area_id].append(data)
                                                success = True
                                                break;

                        if success:
                            break

                # if not success:
                #     print "[REMOVE] line:", progress, " ip_name:", ip_name.encode('utf-8'), len(ip_name), "\n"

                tip = str(progress) + ' / ' + str(sum_total)
                sys.stdout.write(tip + '\r')

        return area_ips


# merge same_segment ip
def handle_group_ips(ips_dic):
        csv_data = [];
        for area_id in ips_dic.keys():
                area_list = ips_dic[area_id]
                area_list.sort(key=lambda x:format_ip(x[0]), reverse=False)
                if len(area_list) == 0:
                        continue

                first_element = area_list.pop(0)
                has_merge = False
                if len(area_list) == 0:
                        has_merge = True

                while len(area_list) > 0:
                        next_element = area_list.pop(0)
                        if next_ip(first_element[1]) == format_ip(next_element[0]):
                                first_element[1] = next_element[1]
                                has_merge = True
                        else:
                                csv_data.append(first_element)
                                first_element = next_element
                                has_merge = False

                if has_merge:
                        csv_data.append(first_element)

        # 升序
        csv_data.sort(key=lambda x:format_ip(x[0]), reverse=False)
        # 火星地区
        csv_data.append(['0.0.0.0', '255.255.255.255', 1000, '神秘地区']);

        # 编号
        for i in range(len(csv_data)):
                csv_data[i].insert(0, i + 1)

        return csv_data

def write_to_csv(file_name, row_header, data):
        # csvFile = open(file_name, 'w')
        csvFile = codecs.open(file_name, 'w', 'utf_8_sig')
        writer = csv.writer(csvFile)
        writer.writerow(row_header)
        writer.writerows(data)
        csvFile.close()

def generate_area_table():
        area_dict, zone_dict = parse_area_txt(read_file('area.txt'))
        ips_list             = parse_ip_txt(read_file('ip.txt'))

        area = group_ips(area_dict, zone_dict, ips_list)
        data = handle_group_ips(area)
        header = ['id', 'ips.origin', 'ips.end_point', 'ips.area_id', 'ips.area_name']
        write_to_csv('area_ip_table.csv', header, data)

generate_area_table()
