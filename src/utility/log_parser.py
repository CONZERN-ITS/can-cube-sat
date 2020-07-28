import serial 
import os

import argparse

os.environ["MAVLINK_DIALECT"] = "its"
os.environ["MAVLINK20"] = "1"

from pymavlink.dialects.v20 import its as its_mav
from pymavlink import mavutil
	

def arguments():
    parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument("-lp", "--logs_path", help="path to log file")
    args = parser.parse_args()
    return args


def read_log_file(path_to_log_file):
	mav_read_file = mavutil.mavlogfile(path_to_log_file, robust_parsing=True, notimestamps=False)

	while True:
	    msg_read = mav_read_file.recv_match(blocking=False)
	    print(msg_read)

if __name__ == '__main__':
	args = arguments()
	read_log_file(args.logs_path)