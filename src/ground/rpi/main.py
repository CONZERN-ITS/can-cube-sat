import time
import datetime

import numpy as NumPy
from math import sin, cos, radians, acos, asin

from strela_ms_rpi import Lis3mdl, Lsm6ds3, WMM2020
from gps_data import GPSD_data
from strela_ms_math import 

PORT_I2C = 1
I2C_TIMEOUT = 1

LIS3MDL_ADRESS = 0x1e
LSM6DS3_ADRESS = 0x6B

ACCEL_SAMPLE_SIZE = 100
MAG_SAMPLE_SIZE = 100
GPS_SAMPLE_SIZE = 20


if __name__ == '__main__':
    i2c = i2cdev.I2C(PORT_I2C)
    i2c.set_timeout(I2C_TIMEOUT)

    lis3mdl = Lis3mdl(i2c, LIS3MDL_ADRESS)
    lis3mdl.setup()    
    lsm6ds3 = Lsm6ds3(i2c, LSM6DS3_ADRESS)
    lsm6ds3.setup()
    gpsd = GPS_data()
    gpsd.setup()
    wwm = WMM2020()

    mag = NumPy.array([0, 0, 0])
    for i in range(MAG_SAMPLE_SIZE):
        mag = mag + NumPy.array(lis3mdl.get_mag_data_G())
    mag = mag / MAG_SAMPLE_SIZE

    accel = NumPy.array([0, 0, 0])
    for i in range(ACCEL_SAMPLE_SIZE):
        accel = accel + NumPy.array(lsm6ds3.get_accel_data_mg())
    accel = accel / ACCEL_SAMPLE_SIZE

    lan_lot_alt = NumPy.array([0, 0, 0])
    x_y_z = NumPy.array([0, 0, 0])
    for i in range(GPS_SAMPLE_SIZE):
    	data = gpsd.find_tpv_data()
    	if gpsd.tpv_get_lat_lon_alt(data) is None:
    		raise ValueError("lat lon alt data is None")
    	if gpsd.tpv_get_x_y_z(data) is None:
    		raise ValueError("x y z data is None")    	
    	lan_lot_alt = lan_lot_alt + NumPy.array(gpsd.tpv_get_lat_lon_alt(data))
    	x_y_z = x_y_z + NumPy.array(gpsd.tpv_get_x_y_z(data))
    lan_lot_alt = lan_lot_alt / GPS_SAMPLE_SIZE
    x_y_z = x_y_z / GPS_SAMPLE_SIZE

    date = datetime.date.today()
    wwm.setup_location(lan_lot_alt[0], lan_lot_alt[1], lan_lot_alt[2], date.day, date.month, date.year)
    decl = wwm.get_declination()

    dec_to_top = dec_to_top_matix(*lan_lot_alt[:2])

    top_to_gcs = top_to_gcscs_matix(mag, accel, decl)

    phi = 0
    alpha = 0

    while True:
        object_coords = NumPy.array([2830356.5 + 7, 2197847.9 - 2, 5258838.4 + 30])
        vector = object_coords - gcs_coords

        vector = NumPy.dot(dec_to_top, vector)
        vector = NumPy.dot(top_to_gcs, vector)

        vector = vector / NumPy.linalg.norm(vector)
        d_phi = acos(vector[2]) - phi
        phi = acos(vector[2])

        vector[2] = 0
        vector = vector / NumPy.linalg.norm(vector)
        d_alpha = acos(vector[2]) - alpha
        alpha = acos(vector[2])

        time.sleep(1)


    i2c.close()
