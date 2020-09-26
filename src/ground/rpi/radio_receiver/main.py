import os
os.environ['MAVLINK_DIALECT'] = "its"
os.environ['MAVLINK20'] = "its"
from pymavlink import mavutil
from pymavlink.dialects.v20 import its as its_mav

import serial
import time
import struct
import signal
import traceback

from parse_config_file import parse_config, read_config
from logs import *
from parse_arguments import *

# TODO: добавить системные логи (дата и время включения, открытие, закрытие файлов и соединенй, ошибки рабооты программы

RSSI_STRUCT = struct.Struct("b")
GCS_SYSTEM_ID = 1
GCS_RADIO_COMPONENT_ID = 0

run_w = True

my_signals = [signal.SIGINT, signal.SIGHUP, signal.SIGTERM]

class bcolors:
    VIOLET = '\033[95m'
    OKBLUE = '\033[94m'
    OKGREEN = '\033[92m'
    YELLOW = '\033[93m'
    RED = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'


def signal_handler(signum, frame):
    global run_w
    run_w = False


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


def open_packet_log(packet_log):
    return open_log_file(packet_log, 'wb')


def open_raw_log(raw_log):
    return open_log_file(raw_log, 'wb')


def parse(input_connection, output_connections, packet_log, raw_log, print_logs, rssi_on):
    mav = its_mav.MAVLink(file=None)
    mav.robust_parsing = True

    rssi = None

    stream_packet_log = packet_log
    stream_raw_log = raw_log

    while run_w:
        input_connection.timeout = 5.0
        r1 = input_connection.read(1)

        if r1 == b'':
            continue

        # Один радио пакет в 220 байт передается примерно
        # 220 байт / 9600 бауд / (8 бит + 1 старт бит + 1 стоп бит) ~= 0.0023 с.
        # Поэтому если мы подождем тут 0.3мс - мы его точно поймаем
        # Сильно дольше ждать опасно, так как мы можем ухватить кусок следующего пакета
        input_connection.timeout = 0.3
        r2 = input_connection.read(400)

        data = r1 + r2

        stream_raw_log.write(data)
        stream_raw_log.flush()

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
                stream_packet_log.flush()

        if print_logs and rssi_on:
            print("rssi level = %s" % rssi)

        if rssi_on and rssi != None:
            msg = msg_rssi(rssi=rssi)
            for output_connection in output_connections:
                hacky_send(output_connection, msg)  # отправка пакета дальше

            usec = int(time.time() * 1.0e6) & -3
            stream_packet_log.write(struct.pack('>Q', usec) + msg.get_msgbuf())

    stream_packet_log.flush()
    stream_raw_log.flush()

    stream_packet_log.close()
    stream_raw_log.close()

    input_connection.close()
    for output_connection in output_connections:
        output_connection.close()

    print("I`m tired. I`m leaving.")
    exit(0)


def main():
    arg = arguments()
    if arg.config != None:
        config_data = read_config(str(arg.config))
        arg = parse_config(arg, config_data)

    for sig in my_signals:
        signal.signal(sig, signal_handler)

    pack_log = None
    raw_log = None

    try:
        make_logs_directory(arg.logs_path)

        packet_log_file_path = generate_new_logs_filename(arg.logs_path, arg.packet_logfile_basename, arg.logfile_extension)
        raw_log_file_path = generate_new_logs_filename(arg.logs_path, arg.raw_logfile_basename, arg.logfile_extension)

        pack_log = open_packet_log(packet_log_file_path)
        raw_log = open_raw_log(raw_log_file_path)

        input_conn = input_connection(arg.serial_device, arg.serial_baudrate)
        output_conns = [output_connection(x) for x in arg.output]
        parse(input_conn, output_conns, pack_log, raw_log, arg.print, arg.RSSI)
    except Exception:
        print("I`m tired. I`m leaving.")
        print("Your exception. Good luck!")
        traceback.print_exc()

        if pack_log:
            pack_log.flush()
            pack_log.close()

        if raw_log:
            raw_log.flush()
            raw_log.close()

        exit(1)


if __name__ == '__main__':
    main()
