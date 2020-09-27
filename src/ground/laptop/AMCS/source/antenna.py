from PyQt5 import QtCore
import os
os.environ['MAVLINK_DIALECT'] = "its"
os.environ['MAVLINK20'] = "its"
from pymavlink.dialects.v20 import its as mavlink2
from pymavlink import mavutil

from source import RES_ROOT, LOG_FOLDER_PATH

import math
import time


class AntennaSystem():
    def __init__(self, ip, port):
        self.ip = ip
        self.port = port

        self.reset_state()
        self.connection = None

        self.last_msg = None

    def get_last_msg(self):
        return self.last_msg

    def reset_state(self):
        self.azimuth = None
        self.elevation = None
        self.latitude = None
        self.longitude = None
        self.altitude = None
        self.ecef = (None, None, None)
        self.top_to_ascs = (None, None, None,
                            None, None, None,
                            None, None, None)
        self.dec_to_top = (None, None, None,
                           None, None, None,
                           None, None, None)
        self.response_time = None

        self.target_azimuth = None
        self.target_elevation = None
        self.target_response_time = None

        self.mode = None
        self.aiming_period = None

        self.motors_enable = (None, None)
        self.motors_auto_disable = None
        self.motors_timeout = None

    def change_ip_and_port(self, ip, port):
        self.ip = ip
        self.port = port

    def setup_connection(self):
        self.connection = mavutil.mavlink_connection('udpout:' + self.ip + ':' + self.port)
        self.log = open(LOG_FOLDER_PATH + 'command_log_' + time.strftime("%d-%m-%Y_%H-%M-%S", time.localtime()) + '.mav', 'wb')

    def close_connection(self):
        self.log.close()
        pass

    def get_response_time(self):
        return self.response_time

    def get_antenna_pos(self):
        return (self.azimuth, self.elevation)

    def get_antenna_system_lat_lon_alt(self):
        return (self.latitude, self.longitude, self.altitude)

    def get_antenna_system_x_y_z(self):
        return self.ecef

    def get_top_to_ascs_matrix(self):
        return self.top_to_ascs

    def get_dec_to_top_matrix(self):
        return self.dec_to_top

    def get_target_pos(self):
        return (self.target_azimuth, self.target_elevation)

    def get_target_response_time(self):
        return self.target_response_time

    def get_mode(self):
        return self.mode

    def get_aiming_period(self):
        return self.aiming_period

    def get_motors_enable(self):
        return self.motors_enable

    def get_motors_auto_disable(self):
        return self.motors_auto_disable

    def get_motors_timeout(self):
        return self.motors_timeout

    def add_state_data(self, msg):
        if msg.get_type() == 'AS_STATE':
            self.azimuth = msg.azimuth
            self.elevation = msg.elevation
            self.latitude = msg.lat_lon[0]
            self.longitude = msg.lat_lon[1]
            self.altitude = msg.alt
            self.ecef = tuple(msg.ecef)
            self.top_to_ascs = tuple(msg.top_to_ascs)
            self.dec_to_top = tuple(msg.dec_to_top)
            self.response_time = self.convert_time_from_s_us_to_s(msg.time_s, msg.time_us)

            self.target_azimuth = msg.target_azimuth
            self.target_elevation = msg.target_elevation
            self.target_response_time = self.convert_time_from_s_us_to_s(msg.target_time_s, msg.target_time_us)

            self.mode = msg.mode
            self.aiming_period = msg.period

            self.motors_enable = tuple(msg.enable)
            self.motors_auto_disable = msg.motor_auto_disable
            self.motors_timeout = msg.motors_timeout
            return True
        else:
            return False

    def convert_time_from_s_to_s_us(self, current_time):
        current_time = math.modf(current_time)
        return (int(current_time[1]), int(current_time[0] * 1000000))

    def convert_time_from_s_us_to_s(self, time_s, time_us):
        return time_s + time_us/1000000

    def send_message(self, msg):
        msg.get_header().srcSystem = 0
        msg.get_header().srcComponent = 3
        if self.connection is not None:
            self.connection.mav.send(msg, False)
            self.log.write(msg.get_msgbuf())
            self.last_msg = msg

    def hard_manual_control(self, azimuth, elevation):
        current_time = self.convert_time_from_s_to_s_us(time.time())
        if self.connection is not None:
            self.send_message(mavlink2.MAVLink_as_hard_manual_control_message(current_time[0],
                                                                              current_time[1],
                                                                              azimuth,
                                                                              elevation))

    def soft_manual_control(self, azimuth, elevation):
        current_time = self.convert_time_from_s_to_s_us(time.time())
        if self.connection is not None:
            self.send_message(mavlink2.MAVLink_as_soft_manual_control_message(current_time[0],
                                                                              current_time[1],
                                                                              azimuth,
                                                                              elevation))

    def automatic_control(self, mode):
        current_time = self.convert_time_from_s_to_s_us(time.time())
        if self.connection is not None:
            self.send_message(mavlink2.MAVLink_as_automatic_control_message(current_time[0],
                                                                            current_time[1],
                                                                            int(mode)))

    def set_aiming_period(self, period):
        current_time = self.convert_time_from_s_to_s_us(time.time())
        if self.connection is not None:
            self.send_message(mavlink2.MAVLink_as_aiming_period_message(current_time[0],
                                                                        current_time[1],
                                                                        period))

    def set_motors_enable(self, mode):
        current_time = self.convert_time_from_s_to_s_us(time.time())
        if self.connection is not None:
            self.send_message(mavlink2.MAVLink_as_motors_enable_mode_message(current_time[0],
                                                                             current_time[1],
                                                                             int(mode)))
    def set_motors_auto_disable_mode(self, mode):
        current_time = self.convert_time_from_s_to_s_us(time.time())
        if self.connection is not None:
            self.send_message(mavlink2.MAVLink_as_motors_auto_disable_message(current_time[0],
                                                                             current_time[1],
                                                                             int(mode)))
    def set_motors_timeout(self, timeout):
        current_time = self.convert_time_from_s_to_s_us(time.time())
        if self.connection is not None:
            self.send_message(mavlink2.MAVLink_as_set_motors_timeout_message(current_time[0],
                                                                             current_time[1],
                                                                             timeout))

    def send_command(self, command_id):
        current_time = self.convert_time_from_s_to_s_us(time.time())
        if self.connection is not None:
            self.send_message(mavlink2.MAVLink_as_send_command_message(current_time[0],
                                                                       current_time[1],
                                                                       command_id))

    def setup_elevation_zero(self):
        self.send_command(0)

    def target_to_north(self):
        self.send_command(1)

    def setup_coord_system(self):
        self.send_command(2)

    def state_request(self):
        self.send_command(3)


class CommandSystem(QtCore.QObject):
    command_sent = QtCore.pyqtSignal(str)

    antenna_pos_changed = QtCore.pyqtSignal(tuple)
    target_pos_changed = QtCore.pyqtSignal(tuple)
    lat_lon_alt_changed = QtCore.pyqtSignal(tuple)
    ecef_changed = QtCore.pyqtSignal(tuple)
    top_to_ascs_matrix_changed = QtCore.pyqtSignal(tuple)
    dec_to_top_matrix_changed = QtCore.pyqtSignal(tuple)
    control_mode_changed = QtCore.pyqtSignal(bool)
    aiming_period_changed = QtCore.pyqtSignal(float)
    motors_enable_changed = QtCore.pyqtSignal(tuple)
    motors_auto_disable_mode_changed = QtCore.pyqtSignal(bool)
    motors_timeout_changed = QtCore.pyqtSignal(float)
    def __init__(self):
        super(CommandSystem, self).__init__()
        self.antenna_system = None
        self.mode = True

    def start_connection(self, ip, port):
        if self.antenna_system is not None:
            self.antenna_system.close_connection()
        self.antenna_system = AntennaSystem(ip, port)
        self.antenna_system.setup_connection()

    def stop_connection(self):
        self.antenna_system.close_connection()
        self.antenna_system = None

    def new_msg_reaction(self, msg):
        if self.antenna_system.add_state_data(msg):
            self.update_data()

    def update_data(self):
        data = list(self.antenna_system.get_antenna_pos())
        data.append(self.antenna_system.get_response_time())
        self.antenna_pos_changed.emit(tuple(data))

        data = list(self.antenna_system.get_target_pos())
        data.append(self.antenna_system.get_target_response_time())
        self.target_pos_changed.emit(tuple(data))

        self.lat_lon_alt_changed.emit(self.antenna_system.get_antenna_system_lat_lon_alt())

        self.ecef_changed.emit(self.antenna_system.get_antenna_system_x_y_z())

        self.top_to_ascs_matrix_changed.emit(self.antenna_system.get_top_to_ascs_matrix())
        self.dec_to_top_matrix_changed.emit(self.antenna_system.get_top_to_ascs_matrix())

        self.control_mode_changed.emit(bool(self.antenna_system.get_mode()))
        self.aiming_period_changed.emit(self.antenna_system.get_aiming_period())

        self.motors_enable_changed.emit(tuple([bool(num) for num in self.antenna_system.get_motors_enable()]))
        self.motors_auto_disable_mode_changed.emit(bool(self.antenna_system.get_motors_auto_disable()))
        self.motors_timeout_changed.emit(self.antenna_system.get_motors_timeout())

    def set_mode(self, mode):
        self.mode = mode

    def manual_control(self, azimuth, elevation):
        if self.mode:
            self.antenna_system.soft_manual_control(azimuth, elevation)
        else:
            self.antenna_system.hard_manual_control(azimuth, elevation)
        self.command_sent.emit(str(self.antenna_system.get_last_msg()))

    def count_angle(self, num):
        angle = 0
        if num == 1:
            angle = 1
        elif num == 2:
            angle = 10
        elif num == 3:
            angle = 50
        return angle

    def put_up(self, num):
        angle = self.count_angle(num)
        self.manual_control(0, angle)

    def put_down(self, num):
        angle = -self.count_angle(num)
        self.manual_control(0, angle)

    def turn_right(self, num):
        angle = -self.count_angle(num)
        self.manual_control(angle, 0)

    def turn_left(self, num):
        angle = self.count_angle(num)
        self.manual_control(angle, 0)

    def automatic_control_on(self):
        self.antenna_system.automatic_control(True)
        self.command_sent.emit(str(self.antenna_system.get_last_msg()))

    def automatic_control_off(self):
        self.antenna_system.automatic_control(False)
        self.command_sent.emit(str(self.antenna_system.get_last_msg()))

    def pull_motors_enable_pin_high(self):
        self.antenna_system.set_motors_enable(True)
        self.command_sent.emit(str(self.antenna_system.get_last_msg()))

    def pull_motors_enable_pin_low(self):
        self.antenna_system.set_motors_enable(False)
        self.command_sent.emit(str(self.antenna_system.get_last_msg()))

    def motors_auto_disable_on(self):
        self.antenna_system.set_motors_auto_disable_mode(True)
        self.command_sent.emit(str(self.antenna_system.get_last_msg()))

    def motors_auto_disable_off(self):
        self.antenna_system.set_motors_auto_disable_mode(False)
        self.command_sent.emit(str(self.antenna_system.get_last_msg()))

    def set_motors_timeout(self, timeout):
        self.antenna_system.set_motors_timeout(timeout)
        self.command_sent.emit(str(self.antenna_system.get_last_msg()))

    def set_aiming_period(self, period):
        self.antenna_system.set_aiming_period(period)
        self.command_sent.emit(str(self.antenna_system.get_last_msg()))

    def setup_elevation_zero(self):
        self.antenna_system.setup_elevation_zero()
        self.command_sent.emit(str(self.antenna_system.get_last_msg()))

    def target_to_north(self):
        self.antenna_system.target_to_north()
        self.command_sent.emit(str(self.antenna_system.get_last_msg()))

    def park(self):
        self.setup_elevation_zero()
        self.target_to_north()

    def setup_coord_system(self):
        self.antenna_system.setup_coord_system()
        self.command_sent.emit(str(self.antenna_system.get_last_msg()))

    def state_request(self):
        self.antenna_system.state_request()
        self.command_sent.emit(str(self.antenna_system.get_last_msg()))
