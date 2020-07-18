import os
os.environ['MAVLINK_DIALECT'] = "its"
os.environ['MAVLINK20'] = "its"
from pymavlink.dialects.v20 import its as mavlink2
from pymavlink import mavutil

import time
import datetime

import numpy as NumPy
from math import sin, cos, radians, acos, asin

from strela_ms_rpi import Lis3mdl, Lsm6ds3, WMM2020
from gps_data import GPSD_data
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
                        act_timeout=1):
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

    def setup_v_limit_pins_map(self, limit_pins_map):
        self.v_limit_pins_map = limit_pins_map
        self.v_stepper_motor.setup_stop_triggers(limit_pins_map.keys())

    def setup_h_limit_pins_map(self, limit_pins_map):
        self.h_limit_pins_map = limit_pins_map
        self.h_stepper_motor.setup_stop_triggers(limit_pins_map.keys())

    def setup(self):
        self.lis3mdl.setup()
        self.lsm6ds3.setup()
        self.gpsd.setup()
        for motor in (self.v_stepper_motor, self.h_stepper_motor):
            motor.setup()

    def setup_coord_system(self):
        self.mag = NumPy.array([0, 0, 0])
        for i in range(self.mag_sample_size):
            self.mag += NumPy.array(lis3mdl.get_mag_data_G())
        self.mag /= self.mag_sample_size

        self.accel = NumPy.array([0, 0, 0])
        for i in range(self.accel_sample_size):
            self.accel += NumPy.array(lsm6ds3.get_accel_data_mg())
        self.accel /= self.accel_sample_size

        self.lan_lot = NumPy.array([0, 0])
        self.alt = NumPy.array([0, 0, 0])
        self.x_y_z = NumPy.array([0, 0, 0])
        start_time = timt.time()
        sample_size = 0
        while ((sample_size < self.gps_sample_size) and ((start_time - timt.time()) < self.act_timeout)):
            data = gpsd.find_tpv_data()
            if gpsd.tpv_get_lat_lon(data) is None:
                continue
            if gpsd.tpv_get_alt(data) is None:
                continue
            if gpsd.tpv_get_x_y_z(data) is None:
                continue      
            self.lan_lot += NumPy.array(gpsd.tpv_get_lat_lon(data))
            self.alt += NumPy.array(gpsd.tpv_get_alt(data))
            self.x_y_z += NumPy.array(gpsd.tpv_get_x_y_z(data))
        self.lan_lot /= GPS_SAMPLE_SIZE
        self.alt /= GPS_SAMPLE_SIZE
        self.x_y_z /= GPS_SAMPLE_SIZE

        date = datetime.date.today()
        wwm.setup_location(lan_lot_alt[0], lan_lot_alt[1], lan_lot_alt[2], date.day, date.month, date.year)
        self.decl = wwm.get_declination()

        self.dec_to_top = dec_to_top_matix(*lan_lot)
        self.top_to_gcs = top_to_gcscs_matix(mag, accel, decl)
        self.setup_current_pos_as_def()

    def setup_v_stepper_motor(self, dm422):
        self.v_stepper_motor = dm422

    def setup_h_stepper_motor(self, dm422):
        self.h_stepper_motor = dm422

    def setup_current_pos_as_def(self):
        self.phi = 0
        self.alpha = 0

    def aiming (self, object_coords):
        vector = object_coords - self.x_y_z
        vector = NumPy.dot(self.dec_to_top, vector)
        vector = NumPy.dot(self.top_to_gcs, vector)
        self.rotate_antenna(vector)

    def rotate_antenna (self, vector):
        vector = vector / NumPy.linalg.norm(vector)
        vector_phi = asin(vector[2])
        vector_alpha = acos(vector[0])
        if vector[1] < 0:
            vector_alpha = 360 - vector_alpha

        self.rotate_v_stepper_motor(vector_phi - self.phi)
        self.rotate_h_stepper_motor(vector_alpha - self.alpha)

    def rotate_v_stepper_motor(self, ang):
        trigger = self.v_stepper_motor.rotate_using_angle(ang)
        if trigger is not None:
            self.phi = self.limit_pins_map.get(trigger)
        else:
            self.phi += self.v_stepper_motor.steps_to_angle(self.v_stepper_motor.get_last_steps_num(),
                                                            self.v_stepper_motor.get_last_steps_direction())
        return trigger

    def rotate_h_stepper_motor(self, ang):
        trigger = self.h_stepper_motor.rotate_using_angle(ang)
        if trigger is not None:
            self.alpha = self.limit_pins_map.get(trigger)
        else:
            self.alpha += self.h_stepper_motor.steps_to_angle(self.h_stepper_motor.get_last_steps_num(),
                                                              self.h_stepper_motor.get_last_steps_direction())
        return trigger

    def setup_nort_as_zero(self):
        vector = self.mag
        vector[2] = 0
        self.rotate_antenna(vector)
        time.sleep(1)
        self.setup_coord_system()

    def setup_v_limit_pos_as_beg(self):
        trigger = None
        while trigger is None:
            self.rotate_v_stepper_motor(90)

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
                                           deg_per_step=V_DEG_PER_STEP)
    h_stepper_motor = DM422_control_client(pul_pin=H_PUL_PIN,
                                           dir_pin=H_DIR_PIN,
                                           enable_pin=H_ENABLE_PIN,
                                           gearbox_num=H_GEARBOX_NUM,
                                           deg_per_step=H_DEG_PER_STEP)

    data_connection = mavutil.mavlink_connection(DATA_CONNECTION_STR)
    gcs_pc_connections = []
    for string in GCS_PC_CONNECTION_STRS:
        gcs_pc_connections.append(mavutil.mavlink_connection(string))

    automode = AUTOMODE

    if automode:
        ACS = AutoGuidance(lis3mdl=lis3mdl,
                           lsm6ds3=lsm6ds3,
                           gpsd=gpsd,
                           wwm=wwm,
                           v_stepper_motor=v_stepper_motor,
                           h_stepper_motor=h_stepper_motor,
                           mag_sample_size=MAG_SAMPLE_SIZE,
                           accel_sample_size=ACCEL_SAMPLE_SIZE,
                           gps_sample_size=GPS_SAMPLE_SIZE,
                           act_timeout=5)
        ACS.setup()
        ACS.setup_v_limit_pins_map(V_LIMIT_PINS_MAP)
        ACS.setup_h_limit_pins_map(H_LIMIT_PINS_MAP)
        ACS.setup_coord_system()
        ACS.setup_v_limit_pos_as_beg()
        ACS.setup_nort_as_zero()
        start_time = time.time()
        gps = None

    while True:
        msg = data_connection.recv_match(blocking=True)
        for connection in gcs_pc_connections:
            connection.mav.send(msg)

        if automode:
            if msg.get_type() == "GPS_UBX_NAV_SOL":
                gps = NumPy.array(msg.ecefX / 100, msg.ecefY / 100, msg.ecefZ / 100)
            if gps is not None:
                if (time.time() - start_time) < ANTENNA_AIMING_PERIOD:
                   ACS.aiming(gps) 

    i2c.close()
