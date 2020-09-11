import os
os.environ['MAVLINK_DIALECT'] = "its"
os.environ['MAVLINK20'] = "its"
from pymavlink.dialects.v20 import its as mavlink2
from pymavlink import mavutil
import time
import re
import math
import numpy as NumPy

from source.functions.wgs84 import wgs84_xyz_to_latlonh as wgs84_conv


class TXTLogDataSource():
    def __init__(self, log_path, real_time=0, time_delay=0.01):
        self.log_path = log_path
        self.real_time = real_time
        self.time_delay = time_delay

    def start(self):
        self.log = open(self.log_path, 'a+')
        self.log.seek(0)
        self.time_shift = None
        self.time_start = None

    def read_data(self):
        string_data = self.log.readline()
        if string_data == '':
            raise EOFError("Log file end")
        data = re.findall(r"[\w.]+", string_data)
        if data is []:
            raise TypeError("String configuration not supported")

        for i in range(len(data)):
            if re.match(r"\d+[.]\d+", data[i]):
                data[i] = float(data[i])
            elif re.match(r"\A\d+\Z",data[i]):
                data[i] = int(data[i])

        if self.time_shift is None:
            self.time_shift = data[0]


        if self.real_time:
            if self.time_start == None:
                self.time_start = time.time()
            while (time.time() - self.time_start) < (data[0] - self.time_shift):
                time.sleep(0.001)
        else:
            time.sleep(self.time_delay)
        return [tuple(data)]

    def write_data(self, data):
        string_data = ''
        for i in data[:-1]:
            string_data += str(i) + ','
        string_data += str(data[-1]) + '\n'
        self.log.write(string_data)

    def stop(self):
        self.log.close()


class MAVLogDataSource():
    def __init__(self, log_path, real_time=0, time_delay=0.01):
        self.log_path = log_path
        self.real_time = real_time
        self.time_delay = time_delay

    def start(self):
        self.connection = mavutil.mavlogfile(self.log_path, notimestamps=True)
        self.time_shift = None
        self.time_start = None

    def read_data(self):
        msg = self.connection.recv_match()
        if (msg is None):
            raise RuntimeError("No Message")
        data = MAVDataSource.get_data(MAVDataSource, msg)
        if data is None:
            raise TypeError("Message type not supported")

        print(data)

        if self.real_time:
            if self.time_shift is None:
                self.time_shift = data[0][1][0][0]
            if self.time_start == None:
                self.time_start = time.time()
            while (time.time() - self.time_start) < (data[0][1][0][0] - self.time_shift):
                time.sleep(0.001)

        else:
            time.sleep(self.time_delay)
        return data

    def stop(self):
        self.connection.close()


class MAVDataSource():
    def __init__(self, connection_str, log_path="./"):
        self.connection_str = connection_str
        self.log_path = log_path

    def start(self):
        self.connection = mavutil.mavlink_connection(self.connection_str)
        self.log = open(self.log_path + time.strftime("%d-%m-%Y_%H-%M-%S", time.localtime()) + '.mav', 'wb')

    def read_data(self):
        msg = self.connection.recv_match()
        if (msg is None):
            raise RuntimeError("No Message")
        self.log.write(msg.get_msgbuf())
        data = self.get_data(msg)
        
        if data is None:
            raise TypeError("Message type not supported")

        print(data)
        return data

    def get_data(self, msg):
        header = msg.get_header()
        name_prefix = str(header.srcSystem) + '_' + str(header.srcComponent) + '_'
        if msg.get_type() == "THERMAL_STATE":
            return [(name_prefix + 'TEMPERATURE',
                     NumPy.array([[msg.time_s + msg.time_us/1000000,
                                   msg.temperature]]))]
        if msg.get_type() == "ELECTRICAL_STATE":
            return [(name_prefix + 'CURRENT',
                     NumPy.array([[msg.time_s + msg.time_us/1000000,
                                   msg.current]])),
                    (name_prefix + 'AREA_' + str(msg.area_id) + '_VOLTAGE',
                     NumPy.array([[msg.time_s + msg.time_us/1000000,
                                   msg.voltage]]))]
        if msg.get_type() == "SINS_ISC":
            return [(name_prefix + 'SINS_ACCEL',
                     NumPy.array([[msg.time_s + msg.time_us/1000000] +
                                   msg.accel])),
                    (name_prefix + 'SINS_COMPASS',
                     NumPy.array([[msg.time_s + msg.time_us/1000000] +
                                   msg.compass])),
                    (name_prefix + 'SINS_MODEL',
                     NumPy.array([[msg.time_s + msg.time_us/1000000] +
                                   msg.quaternion]))]
        if msg.get_type() == "GPS_UBX_NAV_SOL":
            gps = wgs84_conv(msg.ecefX / 100, msg.ecefY / 100, msg.ecefZ / 100)
            return [(name_prefix + 'GPS_LAT_LON',
                     NumPy.array([[msg.time_s + msg.time_us/1000000] +
                                   gps[:2]])),
                    (name_prefix + 'GPS_ALTITUDE',
                     NumPy.array([[msg.time_s + msg.time_us/1000000,
                                   gps[2]]])),
                    (name_prefix + 'GPS_RAW',
                     NumPy.array([[msg.time_s + msg.time_us/1000000] +
                                   [msg.ecefX / 100, msg.ecefY / 100, msg.ecefZ / 100, msg.gpsFix]])),]
        if msg.get_type() == "OWN_TEMP":
            return [(name_prefix + 'OWN_TEMPERATURE',
                     NumPy.array([[msg.time_s + msg.time_us/1000000,
                                   msg.deg]]))]
        if msg.get_type() == "PLD_BME280_DATA":
            return [(name_prefix + 'BME280_TEMPERATURE',
                     NumPy.array([[msg.time_s + msg.time_us/1000000,
                                   msg.temperature]])),
                    (name_prefix + 'BME280_PRESSURE',
                     NumPy.array([[msg.time_s + msg.time_us/1000000,
                                   msg.pressure]])),
                    (name_prefix + 'BME280_HUMIDITY',
                     NumPy.array([[msg.time_s + msg.time_us/1000000,
                                   msg.humidity]])),
                    (name_prefix + 'BME280_ALTITUDE',
                     NumPy.array([[msg.time_s + msg.time_us/1000000,
                                   msg.altitude]]))]
        if msg.get_type() == "PLD_MICS_6814_DATA":
            return [(name_prefix + 'MICS_6814_CO',
                     NumPy.array([[msg.time_s + msg.time_us/1000000,
                                   msg.co_conc]])),
                    (name_prefix + 'MICS_6814_NO2',
                     NumPy.array([[msg.time_s + msg.time_us/1000000,
                                   msg.no2_conc]])),
                    (name_prefix + 'MICS_6814_NH3',
                     NumPy.array([[msg.time_s + msg.time_us/1000000,
                                   msg.nh3_conc]]))]
        if msg.get_type() == "PLD_ME2O2_DATA":
            return [(name_prefix + 'ME2O2',
                     NumPy.array([[msg.time_s + msg.time_us/1000000, msg.o2_conc]]))]
        if msg.get_type() == "PLD_STATS":
            return [(name_prefix + 'PLD_STATS',
                     NumPy.array([[msg.time_s + msg.time_us/1000000,
                                   msg.bme_init_error,
                                   msg.bme_last_error,
                                   msg.bme_error_counter,
                                   msg.adc_init_error,
                                   msg.adc_last_error,
                                   msg.adc_error_counter,
                                   msg.me2o2_init_error,
                                   msg.me2o2_last_error,
                                   msg.me2o2_error_counter,
                                   msg.mics6814_init_error,
                                   msg.mics6814_last_error,
                                   msg.mics6814_error_counter,
                                   msg.integrated_init_error,
                                   msg.integrated_last_error,
                                   msg.integrated_error_counter]]))]
        if msg.get_type() == "I2C_LINK_STATS":
            return [(name_prefix + 'I2C_LINK_STATS',
                     NumPy.array([[msg.time_s + msg.time_us/1000000,
                                   msg.rx_done_cnt,
                                   msg.rx_dropped_cnt,
                                   msg.rx_error_cnt,
                                   msg.tx_done_cnt,
                                   msg.tx_zeroes_cnt,
                                   msg.tx_overrun_cnt,
                                   msg.tx_error_cnt,
                                   msg.restarts_cnt,
                                   msg.listen_done_cnt,
                                   msg.last_error]]))]
        return None

    def stop(self):
        self.connection.close()
        self.log.close()
