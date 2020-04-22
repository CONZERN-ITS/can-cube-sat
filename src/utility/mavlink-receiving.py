import os
os.environ['MAVLINK_DIALECT'] = "its"
os.environ['MAVLINK20'] = "its"

import time

from pymavlink.dialects.v20 import its as mavlink2
from pymavlink import mavutil

the_connection = mavutil.mavlink_connection('tcpin:127.0.0.1:14540')
while True:
    msg = the_connection.recv_match(blocking=True)
    print("Message with id {:d} was received at {}".format(msg.id, time.asctime()))