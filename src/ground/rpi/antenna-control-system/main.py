import os
os.environ['MAVLINK_DIALECT'] = "its"
os.environ['MAVLINK20'] = "its"
from pymavlink.dialects.v20 import its as mavlink2
from pymavlink import mavutil

import time
import math
import datetime

import numpy as NumPy
from math import sin, cos, radians, acos, asin, degrees

import i2cdev
from strela_ms_rpi import Lis3mdl, Lsm6ds3, WMM2020
from gps_data import GPS_data
from strela_ms_math import dec_to_top_matix, top_to_gcscs_matix
from DM422 import DM422_control_client
from config import *


class AutoGuidance():
    def __init__ (self, lis3mdl,
                        lsm6ds3,
                        gpsd,
                        wwm,
                        v_stepper_motor,
                        h_stepper_motor,
                        mag_sample_size=1,
                        accel_sample_size=1,
                        gps_sample_size=1,
                        act_timeout=1,
                        mag_recount_matrix=None,
                        mag_calibration_matrix=None,
                        mag_calibration_vector=None,
                        accel_recount_matrix=None,):
        self.lis3mdl = lis3mdl
        self.lsm6ds3 = lsm6ds3
        self.gpsd = gpsd
        self.wwm = wwm
        self.mag_sample_size = mag_sample_size
        self.accel_sample_size = accel_sample_size
        self.gps_sample_size = gps_sample_size
        self.act_timeout = act_timeout
        self.v_stepper_motor = v_stepper_motor
        self.h_stepper_motor = h_stepper_motor
        self.v_limit_pins_map = {}
        self.h_limit_pins_map = {}
        self.mag_recount_matrix = mag_recount_matrix
        self.mag_calibration_matrix = mag_calibration_matrix
        self.mag_calibration_vector = mag_calibration_vector
        self.accel_recount_matrix = accel_recount_matrix
        self.mag = NumPy.zeros((3, 1))
        self.accel = NumPy.zeros((3, 1))
        self.lat_lon = NumPy.array([0.0, 0.0])
        self.alt = 0.0
        self.x_y_z = NumPy.zeros((3, 1))
        self.dec_to_top = NumPy.zeros((3, 3))
        self.top_to_gcs = NumPy.zeros((3, 3))
        self.decl = 0
        self.target_alpha = 0
        self.target_phi = 0

    def setup_v_limit_pins_map(self, p_limit_pins_map, n_limit_pins_map):
        self.v_stepper_motor.setup_stop_triggers(list(p_limit_pins_map.keys()), list(n_limit_pins_map.keys()))
        self.v_limit_pins_map = p_limit_pins_map
        self.v_limit_pins_map.update(n_limit_pins_map)

    def setup_h_limit_pins_map(self, p_limit_pins_map, n_limit_pins_map):
        self.h_stepper_motor.setup_stop_triggers(list(p_limit_pins_map.keys()), list(n_limit_pins_map.keys()))
        self.h_limit_pins_map = p_limit_pins_map
        self.h_limit_pins_map.update(n_limit_pins_map)

    def setup(self):
        self.lis3mdl.setup()
        self.lsm6ds3.setup()
        self.gpsd.setup()
        for motor in (self.v_stepper_motor, self.h_stepper_motor):
            motor.setup()

    def setup_coord_system(self):
        for i in range(self.mag_sample_size):
            self.mag += NumPy.array(lis3mdl.get_mag_data_G()).reshape((3, 1))
        self.mag /= self.mag_sample_size
        if self.mag_calibration_vector is not None:
            self.mag = self.mag - self.mag_calibration_vector
        if self.mag_calibration_matrix is not None:
            self.mag = NumPy.dot(self.mag_calibration_matrix, self.mag)
        if self.mag_recount_matrix is not None:
            self.mag = NumPy.dot(self.mag_recount_matrix, self.mag)

        for i in range(self.accel_sample_size):
            self.accel += NumPy.array(lsm6ds3.get_accel_data_mg()).reshape((3, 1))
        self.accel /= self.accel_sample_size
        if self.accel_recount_matrix is not None:
            self.accel = NumPy.dot(self.accel_recount_matrix, self.accel)

        start_time = time.time()
        sample_size = 0
        while ((sample_size < self.gps_sample_size) and ((start_time - time.time()) < self.act_timeout)):
            data = gpsd.find_tpv_data()
            if gpsd.tpv_get_lat_lon(data) is None:
                continue
            if gpsd.tpv_get_alt(data) is None:
                continue
            if gpsd.tpv_get_x_y_z(data) is None:
                continue
            self.lan_lot += NumPy.array(gpsd.tpv_get_lat_lon(data))
            self.alt += gpsd.tpv_get_alt(data)
            self.x_y_z += NumPy.array(gpsd.tpv_get_x_y_z(data)).reshape((3, 1))
            sample_size += 1
        self.lat_lon /= sample_size
        self.alt /= sample_size
        self.x_y_z /= sample_size

        date = datetime.date.today()
        wwm.setup_location(self.lat_lon[0], self.lat_lon[1], self.alt, date.day, date.month, date.year)
        self.decl = wwm.get_declination()

        self.dec_to_top = dec_to_top_matix(*self.lat_lon)
        self.top_to_gcs = top_to_gcscs_matix(self.mag, self.accel, self.decl)
        self.setup_current_pos_as_def()

    def setup_current_pos_as_def(self):
        self.phi = 0
        self.alpha = 0

    def get_alpha(self):
        return self.alpha

    def get_phi(self):
        return self.phi

    def get_target_alpha(self):
        return self.target_alpha

    def get_target_phi(self):
        return self.target_phi

    def get_x_y_z(self):
        return self.x_y_z

    def get_lat_lon(self):
        return self.lat_lon

    def get_alt(self):
        return self.alt

    def get_top_to_gcs(self):
        return self.top_to_gcs

    def get_dec_to_top(self):
        return self.dec_to_top

    def get_enable_state(self):
        return (self.v_stepper_motor.get_enable_state(), self.h_stepper_motor.get_enable_state())

    def recount_vector(self, vector):
        vector = NumPy.dot(self.dec_to_top, vector)
        vector = NumPy.dot(self.top_to_gcs, vector)
        return vector

    def aiming (self, object_coords):
        vector = object_coords - self.x_y_z
        vector = self.recount_vector(vector)
        self.rotate_antenna(vector)

    def count_target_angles(self, vector):
        vector = vector / NumPy.linalg.norm(vector)
        vector_phi = degrees(asin(vector[2]))
        vector_alpha = degrees(acos(vector[0]))
        if vector[1] < 0:
            vector_alpha = 360 - vector_alpha

        self.target_alpha = vector_alpha
        self.target_phi = vector_phi

    def rotate_antenna (self, vector):
        self.count_target_angles(vector)
        if (abs(self.target_phi - self.phi) > 0.2):
            self.rotate_v_stepper_motor(self.target_phi - self.phi)
        vector = vector / NumPy.linalg.norm(vector)
        if (vector[2] < 0.995) and (abs(self.target_alpha - self.alpha) > 0.2):
            if abs(self.target_alpha - self.alpha) > 180:
                if self.alpha < self.target_alpha:
                    self.rotate_h_stepper_motor(self.target_alpha - self.alpha - 360)
                    self.alpha += 360
                else:
                    self.rotate_h_stepper_motor(self.target_alpha - self.alpha + 360)
                    self.alpha -= 360
            else:
                self.rotate_h_stepper_motor(self.target_alpha - self.alpha) 

    def rotate_v_stepper_motor(self, ang):
        trigger = self.v_stepper_motor.rotate_using_angle(ang)
        if trigger is not None:
            self.phi = self.v_limit_pins_map.get(trigger)
        else:
            self.phi += self.v_stepper_motor.steps_to_angle(self.v_stepper_motor.get_last_steps_num(),
                                                            self.v_stepper_motor.get_last_steps_direction())
        return trigger

    def rotate_h_stepper_motor(self, ang):
        trigger = self.h_stepper_motor.rotate_using_angle(ang)
        if trigger is not None:
            self.alpha = self.h_limit_pins_map.get(trigger)
        else:
            self.alpha += self.h_stepper_motor.steps_to_angle(self.h_stepper_motor.get_last_steps_num(),
                                                              self.h_stepper_motor.get_last_steps_direction())
        return trigger

    def target_to_north(self):
        vector = self.top_to_gcs[:,0]
        self.rotate_antenna(vector)

    def setup_v_limit_pos_as_beg(self):
        trigger = None
        while trigger is None:
            trigger = self.rotate_v_stepper_motor(-180)
        self.rotate_v_stepper_motor(-self.phi)

def send_message(connection, msg):
    msg.get_header().srcSystem = 0
    msg.get_header().srcComponent = 1
    connection.mav.send(msg, False)

def convert_time_from_s_to_s_us(current_time):
    current_time = math.modf(current_time)
    return (int(current_time[1]), int(current_time[0] * 1000000))

if __name__ == '__main__':
    i2c = i2cdev.I2C(PORT_I2C)
    i2c.set_timeout(I2C_TIMEOUT)
    lis3mdl = Lis3mdl(i2c, LIS3MDL_ADRESS)
    lsm6ds3 = Lsm6ds3(i2c, LSM6DS3_ADRESS)
    gpsd = GPS_data()
    wwm = WMM2020()
    v_stepper_motor = DM422_control_client(pul_pin=V_PUL_PIN,
                                           dir_pin=V_DIR_PIN,
                                           enable_pin=V_ENABLE_PIN,
                                           gearbox_num=V_GEARBOX_NUM,
                                           deg_per_step=V_DEG_PER_STEP,
                                           pos_dir_state=V_POS_DIR_STATE,
                                           stop_state=V_STOP_STATE)
    h_stepper_motor = DM422_control_client(pul_pin=H_PUL_PIN,
                                           dir_pin=H_DIR_PIN,
                                           enable_pin=H_ENABLE_PIN,
                                           gearbox_num=H_GEARBOX_NUM,
                                           deg_per_step=H_DEG_PER_STEP,
                                           pos_dir_state=H_POS_DIR_STATE,
                                           stop_state=H_STOP_STATE)

    data_connection = mavutil.mavlink_connection(DATA_CONNECTION_STR)
    command_connection = mavutil.mavlink_connection(COMMAND_CONNECTION_STR)
    ACS = AutoGuidance(lis3mdl=lis3mdl,
                       lsm6ds3=lsm6ds3,
                       gpsd=gpsd,
                       wwm=wwm,
                       v_stepper_motor=v_stepper_motor,
                       h_stepper_motor=h_stepper_motor,
                       mag_sample_size=MAG_SAMPLE_SIZE,
                       accel_sample_size=ACCEL_SAMPLE_SIZE,
                       gps_sample_size=GPS_SAMPLE_SIZE,
                       act_timeout=5,
                       mag_recount_matrix=MAG_RECOUNT_MATRIX,
                       mag_calibration_matrix=MAG_CALIBRATION_MATRIX,
                       mag_calibration_vector=MAG_CALIBRATION_VECTOR,
                       accel_recount_matrix=ACCEL_RECOUNT_MATRIX)
    ACS.setup()
    ACS.setup_v_limit_pins_map(V_P_LIMIT_PINS_MAP, V_N_LIMIT_PINS_MAP)
    ACS.setup_h_limit_pins_map(H_P_LIMIT_PINS_MAP, H_N_LIMIT_PINS_MAP)
    ACS.setup_coord_system()
    start_time = time.time()
    gps = None
    auto_control_mod = False
    target_last_time = (0, 0)

    while True:
        msg = command_connection.recv_match()
        if msg is not None:
            print(msg)
            if msg.get_type() == "AS_AUTOMATIC_CONTROL":
                auto_control_mod = bool(msg.mode)
            elif msg.get_type() == "AS_HARD_MANUAL_CONTROL":
                ACS.v_stepper_motor.rotate_using_angle(msg.elevation)
                ACS.h_stepper_motor.rotate_using_angle(msg.azimuth)
            elif msg.get_type() == "AS_SOFT_MANUAL_CONTROL":
                ACS.rotate_v_stepper_motor(msg.elevation)
                ACS.rotate_h_stepper_motor(msg.azimuth)
            elif msg.get_type() == "AS_MOTORS_ENABLE_MODE":
                ACS.v_stepper_motor.set_enable(bool(msg.mode))
                ACS.h_stepper_motor.set_enable(bool(msg.mode))
            elif msg.get_type() == "AS_SEND_COMMAND":
                enum = mavutil.mavlink.enums['AS_COMMANDS']
                if enum[msg.command_id].name == 'AS_SETUP_ELEVATION_ZERO':
                    ACS.setup_v_limit_pos_as_beg()
                elif enum[msg.command_id].name == 'AS_TARGET_TO_NORTH':
                    ACS.target_to_north()
                elif enum[msg.command_id].name == 'SETUP_COORD_SYSTEM':
                    ACS.setup_coord_system()
            command_connection.last_address = (command_connection.last_address[0], 13404)

            if msg.get_type() != 'BAD_DATA':
                current_time = convert_time_from_s_to_s_us(time.time())
                send_message(command_connection, mavlink2.MAVLink_as_state_message(time_s=current_time[0],
                                                                                   time_us=current_time[1],
                                                                                   azimuth=ACS.get_alpha(),
                                                                                   elevation=ACS.get_phi(),
                                                                                   ecef=list(ACS.get_x_y_z()),
                                                                                   lat_lon=list(ACS.get_lat_lon()),
                                                                                   alt=ACS.get_alt(),
                                                                                   top_to_ascs=list(ACS.get_top_to_gcs().reshape(9)),
                                                                                   dec_to_top=list(ACS.get_dec_to_top().reshape(9)),
                                                                                   target_time_s=target_last_time[0],
                                                                                   target_time_us=target_last_time[1],
                                                                                   target_azimuth=ACS.get_target_alpha(),
                                                                                   target_elevation=ACS.get_target_phi(),
                                                                                   mode=int(auto_control_mod),
                                                                                   enable=[int(mode) for mode in ACS.get_enable_state()]))

        msg = data_connection.recv_match()
        if msg is not None:
            if msg.get_type() == "GPS_UBX_NAV_SOL":
                print(msg)
                if msg.gpsFix > 0:
                    target_last_time = (msg.time_s, msg.time_us)
                    gps = NumPy.array([msg.ecefX / 100, msg.ecefY / 100, msg.ecefZ / 100]).reshape((3, 1))
                    vector = gps - ACS.x_y_z
                    vector = ACS.recount_vector(vector)
                    ACS.count_target_angles(vector)

        if auto_control_mod:
            if (time.time() - start_time) > ANTENNA_AIMING_PERIOD:
                if gps is not None:
                   ACS.aiming(gps)
                   gps = None
                start_time = time.time()



    i2c.close()
