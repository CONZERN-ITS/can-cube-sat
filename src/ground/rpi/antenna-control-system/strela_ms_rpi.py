import i2cdev
import ctypes
import time

srtela_ms_rpi_c = ctypes.CDLL("./native/build/libstrela-ms-rpi.so")

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
        time.sleep(0.1)

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