#!/usr/bin/env python3

import os
import time

os.environ["MAVLINK_DIALECT"] = "its"
os.environ["MAVLINK20"] = "1"

from pymavlink import mavutil

con = mavutil.mavlink_connection("tcp:localhost:3000")

i = 0
while True:
	con.mav.ping_send(time_usec=0, seq=0, target_system=0, target_component=0, force_mavlink1=False)
	print(f"done {i}")
	i += 1
	time.sleep(1)
