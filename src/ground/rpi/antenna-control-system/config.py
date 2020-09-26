import numpy as NumPy

DATA_CONNECTION_STR = 'udpin:0.0.0.0:41313'
COMMAND_CONNECTION_STR = 'udpin:0.0.0.0:13404'

PORT_I2C = 1
I2C_TIMEOUT = 1

LIS3MDL_ADRESS = 0x1e
LSM6DS3_ADRESS = 0x6B

ACCEL_SAMPLE_SIZE = 100
MAG_SAMPLE_SIZE = 50
GPS_SAMPLE_SIZE = 20

GPS_DATA_TIMEOUT = 30

MAG_RECOUNT_MATRIX = NumPy.array([[-1, 0, 0],
                                  [ 0, 0, 1],
                                  [ 0, 1, 0]])

MAG_CALIBRATION_MATRIX = NumPy.array([[ 0.948874, -0.010167, 0.266488],
                                      [-0.010167,  1.230079, 0.015828],
                                      [ 0.266488,  0.015828, 1.178367]])

MAG_CALIBRATION_VECTOR = NumPy.array([[-0.784921], [-0.038389], [-0.837087]])

ACCEL_RECOUNT_MATRIX = NumPy.array([[ 1,  0, 0],
                                    [ 0, -1, 0],
                                    [ 0,  0, -1]])

V_PUL_PIN = 18
V_DIR_PIN = 16
V_ENABLE_PIN = 32
V_GEARBOX_NUM = 102
V_DEG_PER_STEP = 1.8 / 8
V_POS_DIR_STATE = False
V_P_LIMIT_PINS_MAP = {7:90}
V_N_LIMIT_PINS_MAP = {11:-10.5}
V_STOP_STATE = 0

H_PUL_PIN = 13
H_DIR_PIN = 15
H_ENABLE_PIN = 31
H_GEARBOX_NUM = 102
H_DEG_PER_STEP = 1.8 / 8
H_POS_DIR_STATE = False
H_P_LIMIT_PINS_MAP = {}
H_N_LIMIT_PINS_MAP = {}
H_STOP_STATE = 0

ANTENNA_AIMING_PERIOD = 1
