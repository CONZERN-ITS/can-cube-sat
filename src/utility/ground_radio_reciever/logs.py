import sys
import os
import glob
import re
import datetime

default_mavlink_packet_log_filename = "mavlink_packet_log_"
default_mavlink_time_packet_log_filename = "mavlink_time_packet_log_"
extantion = ".txt"


# нахождение последнего лог файла (поиск производится по дате создания)
def find_lats_file(dir_name):
    dir_name = str(dir_name)
    file_list = glob.glob(dir_name + "/*")
    file_list_logs = []

    for i in range(len(file_list)):
        if (default_mavlink_time_packet_log_filename or default_mavlink_packet_log_filename) in file_list[i]:
            file_list_logs.append(file_list[i])
    if file_list_logs == []:  # в папке нет наших логов
        return []
    return max(file_list_logs, key=os.path.getctime)


# генерация новых лог файлов
def generate_new_logs_filename(packet_log_filename, time_packet_log_filename):
    return ["{}{}.{}".format(packet_log_filename, "0", extantion), "{}{}.{}".format(time_packet_log_filename, "0", extantion)]


# генерация следующих лог файлов
def generate_next_logs_filename(last_file, logs_path):
    if last_file == []:     # в папке еще нет наших логов, генерируем новые файлы
        return generate_new_logs_filename(default_mavlink_packet_log_filename, default_mavlink_time_packet_log_filename)

    file_name = re.split(r'\D', last_file)      # выбираем все числа из строки

    file_number = 0
    for i in range(len(file_name)-1, 0, -1):    # проходим в обратном направлении т.к. в пути до файла могут быть еще цифры, кроме нужных нам
        if file_name[i] != '':
            file_number = int(file_name[i]) + 1
            break
    return ["{}{}{}{}{}".format(logs_path, "/", default_mavlink_packet_log_filename, file_number, extantion),
            "{}{}{}{}{}".format(logs_path, "/", default_mavlink_time_packet_log_filename, file_number, extantion)]


def open_log_files(list_logs_filename):
    try:
        mavlink_packet_log = open(list_logs_filename[0], 'w')
    except FileNotFoundError:
        print("No such file or directory: {}".format(list_logs_filename[0]))
        return

    try:
        mavlink_time_packet_log = open(list_logs_filename[1], 'w')
    except FileNotFoundError:
        print("No such file or directory: {}".format(list_logs_filename[1]))
        return

    return [mavlink_packet_log, mavlink_time_packet_log]


def make_logs_directory(logs_path):      # создание директории для логов
    logs_path = str(logs_path)

    try:
        os.mkdir(logs_path)
        print("Directory", logs_path, "created")
    except FileExistsError:
        print("Directory", logs_path, "already exists")


def write_logs(list_logs_files, mav_packet):
    list_logs_files[0].write(str(mav_packet))
    list_logs_files[1].write("{}\t{}\t".format(datetime.datetime.now(), mav_packet))

