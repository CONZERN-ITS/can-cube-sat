import os
import datetime


# нахождение текущего времени и текущей даты
def now_datetime_str():
    now = datetime.datetime.now().strftime("%Y%m%dT%H%M%S.%fZ")
    return now


def generate_new_logs_filename(logs_path, basename, extension):
    """ генерация новых лог файлов """
    datetime_str = now_datetime_str()
    file_name = "{}_{}.{}".format(basename, datetime_str, extension)
    return os.path.join(logs_path, file_name)


def open_log_file(log_filename, mode):
    try:
        mavlink_packet_log = open(log_filename, str(mode))
    except FileNotFoundError:
        print("No such file or directory: {}".format(log_filename))
        return

    return mavlink_packet_log


def make_logs_directory(logs_path):      # создание директории для логов
    logs_path = str(logs_path)

    try:
        os.mkdir(logs_path)
        print("Directory", logs_path, "created")
    except FileExistsError:
        print("Directory", logs_path, "already exists")
