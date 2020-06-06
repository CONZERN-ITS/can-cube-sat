from pymavlink import mavutil

from logs import *

# TODO: добавить системные логи (дата и время включения, открытие, закрытие файлов и соединенй, ошибки рабооты программы
# TODO: сделать запись глобальных переменных через консоль
dir_logs_path = "logs"

serial = "COM3"
baudrate = 9600
protocol = "udp"
address = "127.0.0.22"


def input_connection(input_serial, serial_baudrate):
    try:
        input_conn = mavutil.mavserial(device=input_serial, baud=serial_baudrate)
    except BaseException as ex:
        print("input connection error", ex)
        return
    return input_conn
    #     добавить в лог запись об установке входящего соединения


def output_connection(output_protocol, output_addr):
    try:
        output_conn = mavutil.mavlink_connection("{}:{}".format(output_protocol, output_addr))
    except BaseException as ex:
        print("output connection error", ex)
        return
    return output_conn


def parse(input_connection, output_connection, list_log_files):
    while True:
        packet = input_connection.recv_match(blocking=False)    # прием пакетов
        write_logs(list_log_files, packet)      # запись логов
        output_connection.write(packet)     # отправка пакета дальше


def main():
    make_logs_directory(dir_logs_path)
    last = find_lats_file(dir_logs_path)
    log_list_filename = generate_next_logs_filename(last, dir_logs_path)
    log_list = open_log_files(log_list_filename)

    input_conn = input_connection(serial, baudrate)
    output_conn = output_connection(protocol, address)
    parse(input_conn, output_conn, log_list)


if __name__ == '__main__':
    main()
