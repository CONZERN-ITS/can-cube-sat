import os
os.environ['MAVLINK_DIALECT'] = "its"
os.environ['MAVLINK20'] = "its"
from pymavlink.dialects.v20 import its as mavlink2
from pymavlink import mavutil
import time
import re

class TXTLogDataSource():
    def __init__(self, log_path, real_time=0, time_delay=0.01, time_from_zero=1):
        self.log_path = log_path
        self.real_time = real_time
        self.time_delay = time_delay
        self.time_from_zero = time_from_zero

    def start(self):
        self.log = open(self.log_path, 'a+')
        self.log.seek(0)
        self.time_shift = None
        self.time_start = None

    def read_data(self):
        string_data = self.log.readline()
        if string_data is '':
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

        if self.time_from_zero:
            data[0] -= self.time_shift

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
    def __init__(self, log_path, real_time=0, time_delay=0.01, time_from_zero=1):
        self.log_path = log_path
        self.real_time = real_time
        self.time_delay = time_delay
        self.time_from_zero = time_from_zero

    def start(self):
        self.connection = mavutil.mavlogfile(self.log_path, notimestamps=True)
        self.time_shift = None
        self.time_start = None

    def read_data(self):
        msg = self.connection.recv_match()
        if (msg is None):
            raise RuntimeError("No Message")
        data = MAVDataSource.get_data(MAVDataSource,msg)
        if data is None:
            raise TypeError("Message type not supported")

        if self.time_shift is None:
            self.time_shift = data[0]

        if self.time_from_zero:
            data[0] -= self.time_shift

        if self.real_time:
            if self.time_start == None:
                self.time_start = time.time()
            while (time.time() - self.time_start) < (data[0] - self.time_shift):
                time.sleep(0.001)
        else:
            time.sleep(self.time_delay)
        return [tuple(data)]

    def stop(self):
        self.connection.close()


class MAVDataSource():
    def __init__(self, connection_str, log_path="./"):
        self.connection_str = connection_str
        self.log_path = log_path

    def start(self):
        self.connection = mavutil.mavlink_connection(self.connection_str)
        self.log = open(self.log_path + time.strftime("%Y_%m_%d_%H_%M_%S", time.localtime()) + '.mav', 'wb')

    def read_data(self):
        msg = self.connection.recv_match()
        if (msg is None):
            raise RuntimeError("No Message")
        self.log.write(msg.get_msgbuf())
        data = self.get_data(msg)
        if data is None:
            raise TypeError("Message type not supported")

        return data

    def get_data(self, msg):
        if msg.get_type() == "THERMAL_STATE":
            return [('TEMPERATURE_' + str(msg.area_id),
                     msg.time_s + msg.time_us/1000000,
                     msg.temperature)]
        if msg.get_type() == "ELECTRICAL_STATE":
            return [('CURRENT_' + str(msg.area_id),
                     msg.time_s + msg.time_us/1000000,
                     msg.current),
                    ('VOLTAGE_' + str(msg.area_id),
                     msg.time_s + msg.time_us/1000000,
                     msg.voltage)]
        if msg.get_type() == "SINS_ISC":
            return [tuple(['ACCEL', msg.time_s + msg.time_us/1000000] + msg.accel),
                    tuple(['COMPASS', msg.time_s + msg.time_us/1000000] + msg.compass),
                    tuple(['MODEL', msg.time_s + msg.time_us/1000000] + msg.quaternion)]
        else:
            return None

    def stop(self):
        self.connection.close()
        self.log.close()
