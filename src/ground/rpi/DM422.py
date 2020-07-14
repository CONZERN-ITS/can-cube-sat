import RPi.GPIO as GPIO
import time

PUL_PIN = 12
DIR_PIN = 14
ENABLE_PIN = 16

class DM422_control_client ():
	def __init__ (self, pul_pin=PUL_PIN, dir_pin=DIR_PIN, enable_pin=ENABLE_PIN, gearbox_num=1, deg_per_step=1):
		self.pul_pin = pul_pin
		self.dir_pin = dir_pin
		self.enable_pin = enable_pin
		self.gearbox_num = gearbox_num
		self.deg_per_step = deg_per_step

	def setup (self):
		GPIO.setmode(GPIO.BOARD)

        GPIO.setup(self.pul_pin, GPIO.OUT)
        GPIO.output(self.pul_pin, True)

        GPIO.setup(self.dir_pin, GPIO.OUT)
        GPIO.output(self.dir_pin, True)

        GPIO.setup(self.enable_pin, GPIO.OUT)
        GPIO.output(self.enable_pin, False)

    def rotate_using_angle (self, ang):
    	steps = self.gearbox_num * ang / self.deg_per_step
    	if steps > 0:
    		self.rotate_using_steps(round(steps), False)
    	else:
    		self.rotate_using_steps(round(abs(steps)), True)

    def rotate_using_steps (self, steps, direction=False):
    	GPIO.output(self.pul_pin, True)
    	GPIO.output(self.enable_pin, True)
    	time.sleep(0.000005)

    	GPIO.output(self.dir_pin, direction)
    	time.sleep(0.000005)

    	for i in range(steps):
    		GPIO.output(self.pul_pin, False)
    		time.sleep(0.0000075)
    		GPIO.output(self.pul_pin, True)
    		time.sleep(0.0000075)

    	GPIO.output(self.enable_pin, False)
    	time.sleep(0.000005)



        
if __name__ == '__main__':
    dm422 = DM422_control_client()
    dm422.setup(gearbox_num=102, deg_per_step=1.8)
    while True:
        dm422.rotate_using_angle(90)