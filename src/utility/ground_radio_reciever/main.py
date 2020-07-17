import os
os.environ['MAVLINK_DIALECT'] = "its"
os.environ['MAVLINK20'] = "its"
from pymavlink import mavutil
from logs import *
from parse_arguments import *

# TODO: добавить системные логи (дата и время включения, открытие, закрытие файлов и соединенй, ошибки рабооты программы


def input_connection(input_serial, serial_baudrate):
    try:
        input_conn = mavutil.mavserial(device=input_serial, baud=serial_baudrate, autoreconnect=True)
    except BaseException as ex:
        print("input connection error", ex)
        return
    return input_conn
    #     добавить в лог запись об установке входящего соединения


def output_connection(output_protocol, output_addr, output_port):
    try:
        output_conn = mavutil.mavlink_connection("{}:{}:{}".format(output_protocol, output_addr, output_port),
                                                 input=False)
    except BaseException as ex:
        print("output connection error", ex)
        return
    return output_conn


def parse(input_connection, output_connection, list_log_files, print_logs):

    input_connection.setup_logfile_raw(str(list_log_files))
    while True:
        packet = input_connection.recv()    # прием пакетов
        if packet:
            output_connection.write(packet)     # отправка пакета дальше
            if print_logs:
                print(packet)

def main():
    arg = arguments()
    make_logs_directory(arg.logs_path)
    log_list_filename = generate_new_logs_filename(arg.logs_path, arg.mavlink_packet_log_filename, arg.extantion)

    input_conn = input_connection(arg.serial_device, arg.serial_baudrate)
    output_conn = output_connection(arg.output_protocol, arg.output_address, arg.output_port)
    parse(input_conn, output_conn, log_list_filename, arg.print)


if __name__ == '__main__':
    main()
