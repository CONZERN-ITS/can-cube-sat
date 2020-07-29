import os
import sys


import time

import argparse

os.environ["MAVLINK_DIALECT"] = "its"
os.environ["MAVLINK20"] = "True"

from pymavlink.dialects.v20 import its as its_mav
from pymavlink import mavutil
	

def arguments():
    parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument("-lp", "--logs_path", help="path to log file")

    parser.add_argument("-ts", "--timestamps", help="if packet have timestamps before packet body", action="store_true", default=False)
    args = parser.parse_args()
    return args


def read_log_file(path_to_log_file, timestamps):
	mav_read_file = mavutil.mavlogfile(path_to_log_file, robust_parsing=True, notimestamps=(not timestamps))

	while True:
	    msg_read = mav_read_file.recv_match(blocking=False)
	    if msg_read == None:
	    	break
	    print(msg_read)

def main():
	args = arguments()
	read_log_file(args.logs_path, args.timestamps)

if __name__ == '__main__':
	main()
	sys.exit()