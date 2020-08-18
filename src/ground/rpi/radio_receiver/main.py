import os
os.environ['MAVLINK_DIALECT'] = "its"
os.environ['MAVLINK20'] = "its"
from pymavlink import mavutil
from pymavlink.dialects.v20 import its as its_mav

import serial
import time
import struct

from parse_config_file import parse_config, read_config
from logs import *
from parse_arguments import *

# TODO: добавить системные логи (дата и время включения, открытие, закрытие файлов и соединенй, ошибки рабооты программы

RSSI_STRUCT = struct.Struct("b")
GCS_SYSTEM_ID = 0
GCS_RADIO_COMPONENT_ID = 0

class bcolors:
    VIOLET = '\033[95m'
    OKBLUE = '\033[94m'
    OKGREEN = '\033[92m'
    YELLOW = '\033[93m'
    RED = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'



def msg_rssi(rssi):
    return its_mav.MAVLink_rssi_message(rssi=rssi)


def input_connection(input_serial, serial_baudrate):
    input_conn = serial.Serial(port=input_serial, baudrate=serial_baudrate)
    return input_conn


def output_connection(output_mavutil_def):
    output_conn = mavutil.mavlink_connection(output_mavutil_def, input=False)
    return output_conn


def hacky_send(mav_con, msg):
    """ asdasd """
    if msg.get_msgId() == its_mav.MAVLINK_MSG_ID_RSSI:
        mav_con.mav.srcSystem = GCS_SYSTEM_ID
        mav_con.mav.srcComponent = GCS_RADIO_COMPONENT_ID
        mav_con.mav.send(msg)

    mav_con.mav.srcSystem = msg.get_header().srcSystem
    mav_con.mav.srcComponent = msg.get_header().srcComponent
    mav_con.mav.send(msg)  # отправка пакета дальше


def parse(input_connection, output_connections, packet_log, raw_log, print_logs, rssi_on):
    mav = its_mav.MAVLink(file=None)
    mav.robust_parsing = True

    rssi = None

    stream_packet_log = open_log_file(packet_log, 'wb')
    stream_raw_log = open_log_file(raw_log, 'wb')

    while True:
        input_connection.timeout = None
        r1 = input_connection.read(1)

        input_connection.timeout = 1.0
        r2 = input_connection.read(400)
        data = r1 + r2

        stream_raw_log.write(data)

        if rssi_on:
            rssi, *_ = RSSI_STRUCT.unpack(bytes((data[-1],)))
            data = data[:-1]

        msgs = mav.parse_buffer(data)
        for msg in msgs or []:
            if print_logs:
                # if msg.get_type() == 'BAD_DATA':
                #     if os.name == 'posix':
                #         print(bcolors.RED + str(msg))
                print(msg)

            if msg.get_type() != 'BAD_DATA':
                for output_connection in output_connections:
                    hacky_send(output_connection, msg)  # отправка пакета дальше

                usec = int(time.time() * 1.0e6) & -3
                stream_packet_log.write(struct.pack('>Q', usec) + msg.get_msgbuf())

        if print_logs and rssi_on:
            print("rssi level = %s" % rssi)

        if rssi_on and rssi != None:
            msg = msg_rssi(rssi=rssi)
            for output_connection in output_connections:
                hacky_send(output_connection, msg)  # отправка пакета дальше

            usec = int(time.time() * 1.0e6) & -3
            stream_packet_log.write(struct.pack('>Q', usec) + msg.get_msgbuf())


def main():
    arg = arguments()
    if arg.config != None:
        config_data = read_config(str(arg.config))
        arg = parse_config(arg, config_data)

    make_logs_directory(arg.logs_path)

    packet_log_file_path = generate_new_logs_filename(arg.logs_path, arg.packet_logfile_basename, arg.logfile_extension)
    raw_log_file_path = generate_new_logs_filename(arg.logs_path, arg.raw_logfile_basename, arg.logfile_extension)

    input_conn = input_connection(arg.serial_device, arg.serial_baudrate)
    output_conns = [output_connection(x) for x in arg.output]
    parse(input_conn, output_conns, packet_log_file_path, raw_log_file_path, arg.print, arg.RSSI)


if __name__ == '__main__':
    main()
