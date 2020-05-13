import i2cdev
import ctypes
import time
import datetime

import numpy as NumPy
from math import sin, cos, radians, acos, asin

import wgs84

srtela_ms_rpi_c = ctypes.CDLL("./native/build/libstrela-ms-rpi.so")

ACCEL_SAMPLE_SIZE = 100
MAG_SAMPLE_SIZE = 100

PORT_I2C = 1
I2C_TIMEOUT = 1

LIS3MDL_ADRESS = 0x1e
LSM6DS3_ADRESS = 0x6B

stmdev_ctx_func_type = ctypes.CFUNCTYPE(ctypes.c_uint32,
                                        ctypes.c_void_p,
                                        ctypes.c_uint8,
                                        ctypes.POINTER(ctypes.c_uint8),
                                        ctypes.c_uint16)

class stmdev_ctx_t(ctypes.Structure):
    _fields_ = [('write_reg', stmdev_ctx_func_type),
                ('read_reg', stmdev_ctx_func_type),
                ('handle', ctypes.c_void_p)]

class location_data_t(ctypes.Structure):
    _fields_ = [('latitude', ctypes.c_double),
                ('longitude', ctypes.c_double),
                ('altitude', ctypes.c_double),
                ('day', ctypes.c_int),
                ('month', ctypes.c_int),
                ('year', ctypes.c_int)]

srtela_ms_rpi_c.lis3mdl_init.argtypes = (ctypes.POINTER(stmdev_ctx_t),)
srtela_ms_rpi_c.lis3mdl_init.restype = ctypes.c_int32

srtela_ms_rpi_c.lis3mdl_get_data_G.argtypes = (ctypes.POINTER(stmdev_ctx_t), ctypes.POINTER(ctypes.c_float),)
srtela_ms_rpi_c.lis3mdl_get_data_G.restype = ctypes.c_int32

srtela_ms_rpi_c.lsm6ds3_init.argtypes = (ctypes.POINTER(stmdev_ctx_t),)
srtela_ms_rpi_c.lsm6ds3_init.restype = ctypes.c_int32

srtela_ms_rpi_c.lsm6ds3_get_accel_data_mg.argtypes = (ctypes.POINTER(stmdev_ctx_t), ctypes.POINTER(ctypes.c_float),)
srtela_ms_rpi_c.lsm6ds3_get_accel_data_mg.restype = ctypes.c_int32

srtela_ms_rpi_c.find_true_north.argtypes = (location_data_t, ctypes.POINTER(ctypes.c_double),)
srtela_ms_rpi_c.find_true_north.restype = ctypes.c_int32

class Stmdev_i2c_context():
    def __init__(self, i2c, i2c_addr):
        self.i2c_line = i2c
        self.i2c_addr = i2c_addr
        self.last_error = None
        read = lambda handle, reg, bufp, buf_len: self.read(handle, reg, bufp, buf_len)
        write = lambda handle, reg, bufp, buf_len: self.write(handle, reg, bufp, buf_len)
        self.context = stmdev_ctx_t(stmdev_ctx_func_type(write),
                                    stmdev_ctx_func_type(read),
                                    0)
    def read(self, handle, reg, bufp, buf_len):
        self.last_error = None
        try:
            self.i2c_line.set_addr(self.i2c_addr)
            buf = self.i2c_line.rdwr(bytes([reg]), buf_len)
            for i in range(buf_len):
                bufp[i] = buf[i]
        except Exception as e:
            self.last_error = e
            return 1
        return 0

    def write(self, handle, reg, bufp, buf_len):
        self.last_error = None
        try:
            self.i2c_line.set_addr(self.i2c_addr)
            buf = [reg]
            for i in range(buf_len):
                buf.append(bufp[i])
            self.i2c_line.write(bytes(buf))
        except Exception as e:
            self.last_error = e
            return 1
        return 0


class Lis3mdl(Stmdev_i2c_context):
    def setup(self):
        error = srtela_ms_rpi_c.lis3mdl_init(ctypes.byref(self.context))
        if error:
            raise RuntimeError("lis3mdl_init returned nonzero error: %d" % error) from self.last_error

    def get_mag_data_G(self):
        mag_buf = (ctypes.c_float * 3)()
        error = srtela_ms_rpi_c.lis3mdl_get_data_G(ctypes.byref(self.context), ctypes.cast(ctypes.byref(mag_buf), ctypes.POINTER(ctypes.c_float)))
        if error:
            raise RuntimeError("lis3mdl_get_data_G returned nonzero error: %d" % error) from self.last_error
        return list(mag_buf)


class Lsm6ds3(Stmdev_i2c_context):
    def setup(self):
        error = srtela_ms_rpi_c.lsm6ds3_init(ctypes.byref(self.context))
        if error:
            raise RuntimeError("lsm6ds3_init returned nonzero error: %d" % error) from self.last_error

    def get_accel_data_mg(self):
        accel_buf = (ctypes.c_float * 3)()
        error = srtela_ms_rpi_c.lsm6ds3_get_accel_data_mg(ctypes.byref(self.context), ctypes.cast(ctypes.byref(accel_buf), ctypes.POINTER(ctypes.c_float)))
        if error:
            raise RuntimeError("lsm6ds3_get_accel_data_mg returned nonzero error: %d" % error) from self.last_error
        return list(accel_buf)

class WMM2020():
    def __init__(self, lat=0, lon=0, height=0, day=1, month=1, year=2020):
        self.location = location_data_t(lat, lon, height, day, month, year)

    def setup_location(self, lat=0, lon=0, height=0, day=1, month=1, year=2020):
        self.location = location_data_t(lat, lon, height, day, month, year)

    def get_declination(self):
        decl = ctypes.c_double()
        error = srtela_ms_rpi_c.find_true_north(self.location, ctypes.pointer(decl))
        if error:
            raise RuntimeError("find_true_north returned nonzero error: %d" % error)
        return decl.value

def from_dec_to_topocentric(x, y, z):
    pass



if __name__ == '__main__':
    #i2c = i2cdev.I2C(PORT_I2C)
    #i2c.set_timeout(I2C_TIMEOUT)
    #lis3mdl = Lis3mdl(i2c, LIS3MDL_ADRESS)
    #lis3mdl.setup()    
    #lsm6ds3 = Lsm6ds3(i2c, LSM6DS3_ADRESS)
    #lsm6ds3.setup() 
    wwm = WMM2020() 

    #mag = NumPy.array([0, 0, 0])
    #for i in range(MAG_SAMPLE_SIZE):
    #    mag = mag + NumPy.array(lis3mdl.get_mag_data_G())
    #mag = mag / MAG_SAMPLE_SIZE

    #accel = NumPy.array([0, 0, 0])
    #for i in range(ACCEL_SAMPLE_SIZE):
    #    accel = accel + NumPy.array(lsm6ds3.get_accel_data_mg())
    #accel = accel / ACCEL_SAMPLE_SIZE

    phi = 0
    alpha = 0

    accel = NumPy.array([2, 3, 5])
    mag = NumPy.array([4, 5, 2])
    gcs_coords = NumPy.array([2830356.5, 2197847.9, 5258838.4])

    gps_latlonh = wgs84.wgs84_xyz_to_latlonh(*gcs_coords)
    date = datetime.date.today()
    wwm.setup_location(gps_latlonh[0], gps_latlonh[1], gps_latlonh[2], date.day, date.month, date.year)
    decl = wwm.get_declination()

    lat = radians(gps_latlonh[0])
    lon = radians(gps_latlonh[1])

    dec_to_top = NumPy.array([[-sin(lat)*cos(lon), -sin(lat)*sin(lon) , cos(lat)],
    						  [sin(lon), -cos(lon), 0],
    						  [cos(lat)*cos(lon), cos(lat)*sin(lon), sin(lat)]])

    accel = accel / NumPy.linalg.norm(accel)
    mag = mag / NumPy.linalg.norm(mag)

    mag = mag - accel * NumPy.dot(accel, mag)
    mag = mag / NumPy.linalg.norm(mag)
    mag = mag * cos(decl) + accel * (1 - cos(decl)) * NumPy.dot(accel, mag) + sin(decl) * NumPy.cross(accel, mag)

    b = NumPy.cross(accel, mag)
    b = b / NumPy.linalg.norm(b)

    top_to_gcs = NumPy.linalg.inv(NumPy.array([mag, b, accel]))

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


    #i2c.close()
