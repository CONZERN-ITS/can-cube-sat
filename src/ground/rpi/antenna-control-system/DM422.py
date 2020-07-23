import RPi.GPIO as GPIO
import time

PUL_PIN = 18
DIR_PIN = 16
ENABLE_PIN = 32

class DM422_control_client ():
    def __init__ (self, pul_pin=PUL_PIN,
                        dir_pin=DIR_PIN,
                        enable_pin=ENABLE_PIN,
                        gearbox_num=1,
                        deg_per_step=1,
                        stop_state=0):
        self.pul_pin = pul_pin
        self.dir_pin = dir_pin
        self.enable_pin = enable_pin
        self.gearbox_num = gearbox_num
        self.deg_per_step = deg_per_step
        self.pos_dir_stop_triggers = []
        self.neg_dir_stop_triggers = []
        self.stop_state = stop_state
        self.last_steps_num = None
        self.last_steps_direction = None

    def setup_stop_triggers(self, pos_dir_triggers=[], neg_dir_triggers=[]):
        self.pos_dir_stop_triggers = pos_dir_triggers
        self.neg_dir_stop_triggers = neg_dir_triggers

        for pin in self.pos_dir_stop_triggers:
            GPIO.setup(pin, GPIO.IN)

        for pin in self.neg_dir_stop_triggers:
            GPIO.setup(pin, GPIO.IN)

    def setup (self):
        GPIO.setmode(GPIO.BOARD)

        GPIO.setup(self.pul_pin, GPIO.OUT)
        GPIO.output(self.pul_pin, True)

        GPIO.setup(self.dir_pin, GPIO.OUT)
        GPIO.output(self.dir_pin, True)

        GPIO.setup(self.enable_pin, GPIO.OUT)
        GPIO.output(self.enable_pin, True)

    def set_enable(self, mode=True):
        GPIO.output(self.enable_pin, mode)

    def angle_to_steps (self, ang):
        return round(abs(self.gearbox_num * ang / self.deg_per_step))

    def steps_to_angle (self, steps, direction=True):
        ang = steps * self.deg_per_step / self.gearbox_num
        if not direction:
            ang = ang * -1
        return ang

    def rotate_using_angle (self, ang):
        steps = self.angle_to_steps(ang)
        if ang > 0:
            return self.rotate_using_steps(steps, True)
        else:
            return self.rotate_using_steps(steps, False)

    def rotate_using_steps (self, steps, direction=True):
        self.last_steps_num = steps
        self.last_steps_direction = direction
        if direction:
            triggers = self.pos_dir_stop_triggers
        else:
            triggers = self.neg_dir_stop_triggers
        GPIO.output(self.pul_pin, True)
        self.set_enable()
        time.sleep(0.00001)

        GPIO.output(self.dir_pin, direction)
        time.sleep(0.00001)

        for i in range(steps):
            for pin in triggers:
                if GPIO.input(pin) == self.stop_state:
                    self.last_steps_num = i
                    return pin
            GPIO.output(self.pul_pin, False)
            time.sleep(0.00001)
            GPIO.output(self.pul_pin, True)
            time.sleep(0.00001)
        return None

    def get_last_steps_num(self):
        return self.last_steps_num

    def get_last_steps_direction(self):
        return self.last_steps_direction

if __name__ == '__main__':
    dm422 = DM422_control_client(gearbox_num=102, deg_per_step=(1.8/8))
    dm422.setup()
    dm422.setup_stop_triggers([11], [7])
    while True:
        value1 = dm422.rotate_using_angle(360)
        print(dm422.get_last_steps_num())
        if value1 is not None:
            print('stop --------')
            break
    while True:
        value = dm422.rotate_using_angle(-360)
        print(dm422.get_last_steps_num())
        if (value is not None) and (value != value1):
            print('end --------')
            break