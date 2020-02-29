import i2cdev
import ctypes

srtela_ms_rpi_c = ctypes.CDLL("./native/build/libstrela-ms-rpi.so")

PORT_I2C = 0
I2C_TIMEOUT = 1

LIS3MDL_ADRESS = 0x1C
LSM6DS3_ADRESS = 0x6A

stmdev_ctx_func_type = ctypes.CFUNCTYPE(ctypes.c_uint32,
                                        ctypes.c_void_p,
                                        ctypes.c_uint8,
                                        ctypes.POINTER(ctypes.c_uint8),
                                        ctypes.c_uint16)
data_buf_type = ctypes.c_float * 3

srtela_ms_rpi_c.lis3mdl_init.argtypes = (stmdev_ctx_func_type, stmdev_ctx_func_type)
srtela_ms_rpi_c.lis3mdl_init.restype = ctypes.c_int32

srtela_ms_rpi_c.lis3mdl_get_data_G.argtypes = (ctypes.POINTER(ctypes.c_float))
srtela_ms_rpi_c.lis3mdl_get_data_G.restype = ctypes.c_int32

srtela_ms_rpi_c.lsm6ds3_init.argtypes = (stmdev_ctx_func_type, stmdev_ctx_func_type)
srtela_ms_rpi_c.lsm6ds3_init.restype = ctypes.c_int32

srtela_ms_rpi_c.lsm6ds3_get_accel_data_mg.argtypes = (ctypes.POINTER(ctypes.c_float))
srtela_ms_rpi_c.lsm6ds3_get_accel_data_mg.restype = ctypes.c_int32

class Lis3mdl_context():
    def __inti__(self, i2c, i2c_addr):
        self.i2c_line = i2c
        self.i2c_addr = i2c_addr
        self.last_error = None

    def read(self, handle, reg, bufp, buf_len):
        try:
            self.i2c_line.set_addr(self.i2c_addr)
            reg |= 0x40
            self.i2c_line.write(reg)
            buf = self.i2c_line.read(buf_len)
            for i in range(buf_len):
                bufp[i] = buf[i]
        except Exception as e:
            self.last_error = e
            return 1
        return 0

    def write(self, handle, reg, bufp, buf_len):
        try:
            self.i2c_line.set_addr(self.i2c_addr)
            reg |= 0xC0
            self.i2c_line.write(reg)
            buf_type = ctypes.c_uint8 * buf_len
            buf = buf_type()
            for i in range(buf_len):
                buf[i] = bufp[i]
            self.i2c_line.write(buf)
        except Exception as e:
            self.last_error = e
            return 1
        return 0

class Lsm6ds3_context():
    def __inti__(self, i2c, i2c_addr):
        self.i2c_line = i2c
        self.i2c_addr = i2c_addr
        self.last_error = None

    def read(self, handle, reg, bufp, buf_len):
        try:
            self.i2c_line.set_addr(self.i2c_addr)
            reg |= 0x80
            self.i2c_line.write(reg)
            buf = self.i2c_line.read(buf_len)
            for i in range(buf_len):
                bufp[i] = buf[i]
        except Exception as e:
            self.last_error = e
            return 1
        return 0

    def write(self, handle, reg, bufp, buf_len):
        try:
            self.i2c_line.set_addr(self.i2c_addr)
            self.i2c_line.write(reg)
            buf_type = ctypes.c_uint8 * buf_len
            buf = buf_type()
            for i in range(buf_len):
                buf[i] = bufp[i]
            self.i2c_line.write(buf)
        except Exception as e:
            self.last_error = e
            return 1
        return 0

if __name__ == '__main__':
    i2c = i2cdev.I2C(PORT_I2C)
    i2c.set_timeout(I2C_TIMEOUT)

    accel_buf = data_buf_type()
    mag_buf = data_buf_type()

    error = ctypes.c_int32()

    lis3mdl = Lis3mdl_context(i2c, LIS3MDL_ADRESS)
    read = lambda handle, reg, bufp, buf_len: lis3mdl.read(handle, reg, bufp, buf_len)
    write = lambda handle, reg, bufp, buf_len: lis3mdl.write(handle, reg, bufp, buf_len)
    error = srtela_ms_rpi_c.lis3mdl_init(stmdev_ctx_func_type(read), 
                                         stmdev_ctx_func_type(write))
    if (error != 0):
        raise lis3mdl.last_error

    lsm6ds3 = Lsm6ds3_context(i2c, LSM6DS3_ADRESS)
    read = lambda handle, reg, bufp, buf_len: lsm6ds3.read(handle, reg, bufp, buf_len)
    write = lambda handle, reg, bufp, buf_len: lsm6ds3.write(handle, reg, bufp, buf_len)
    error = srtela_ms_rpi_c.lsm6ds3_init(stmdev_ctx_func_type(read), 
                                         stmdev_ctx_func_type(write))
    if (error != 0):
        raise lsm6ds3.last_error

    while True:
        error = srtela_ms_rpi_c.lis3mdl_get_data_G(mag_buf)
        if (error != 0):
            print("Lis3mdl error : " + str(error))

        error = srtela_ms_rpi_c.lsm6ds3_get_accel_data_mg(accel_buf)
        if (error != 0):
            print("Lsm6ds3 error: " + str(error))

        print("Mag  :")
        print(mag_buf)
        print("Accel:")
        print(accel_buf)


    i2c.close()
