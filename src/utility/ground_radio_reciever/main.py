import os
os.environ['MAVLINK_DIALECT'] = "its"
os.environ['MAVLINK20'] = "its"
from pymavlink import mavutil

from logs import *
from parse_arguments import *

# TODO: добавить системные логи (дата и время включения, открытие, закрытие файлов и соединенй, ошибки рабооты программы


def input_connection(input_serial, serial_baudrate):
    input_conn = mavutil.mavserial(device=input_serial, baud=serial_baudrate, autoreconnect=True)
    return input_conn
    #     добавить в лог запись об установке входящего соединения


def output_connection(output_mavutil_def):
    output_conn = mavutil.mavlink_connection(output_mavutil_def, input=False)
    return output_conn


def parse(input_connection, output_connection, packet_log, raw_log, print_logs):

    input_connection.setup_logfile_raw(raw_log)
    input_connection.setup_logfile(packet_log)

    while True:
        packet = input_connection.recv_match(blocking=True)    # прием пакетов
        if not packet:
            continue

        output_connection.mav.send(packet)     # отправка пакета дальше
        if print_logs:
            print(packet)

def main():
    arg = arguments()
    make_logs_directory(arg.logs_path)

    packet_log_file_path = generate_new_logs_filename(arg.logs_path, arg.packet_logfile_basename, arg.logfile_extension)
    raw_log_file_path = generate_new_logs_filename(arg.logs_path, arg.raw_logfile_basename, arg.logfile_extension)

    input_conn = input_connection(arg.serial_device, arg.serial_baudrate)
    output_conn = output_connection(arg.output)
    parse(input_conn, output_conn, packet_log_file_path, raw_log_file_path, arg.print)


if __name__ == '__main__':
    main()
