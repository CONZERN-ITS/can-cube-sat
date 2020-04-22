#!/usr/bin/env python3

import os
import sys
import argparse
import logging
from collections import namedtuple
from threading import Thread
from queue import Queue


_log = logging.getLogger(__name__)


os.environ["MAVLINK_DIALECT"] = "its"
os.environ["MAVLINK20"] = "1"

from pymavlink import mavutil


def main(argv):

    logging.basicConfig(
        stream=sys.stdout,
        level=logging.INFO,
        format='%(asctime)-15s - %(message)s'
    )

    parser = argparse.ArgumentParser()

    parser.add_argument("--src-ports", type=int, nargs='+')
    parser.add_argument("--src-iface", type=str, nargs='?', default='0.0.0.0')
    parser.add_argument("--dst-mavuri", type=str, nargs='?', required=True)

    args = parser.parse_args(argv)

    if not args.src_ports:
        parser.error("at least one port should be specified")

    src_ports = args.src_ports
    src_iface = args.src_iface
    dst_mavuri = args.dst_mavuri

    _log.info(f"relaying to {dst_mavuri}")
    dst_con = mavutil.mavtcp(dst_mavuri)
    _log.info(f"outgoing connection established")

    msg_queue = Queue()
    threads = []
    for port in src_ports:
        def thread_entry():
            constring = f"{src_iface}:{port}"
            con = mavutil.mavtcpin(constring)
            _log.info("listening on %s" % constring)

            while True:
                msg = con.recv_match(blocking=True)
                _log.info(f"got message {msg.get_header().msgId} on {constring}")
                msg_queue.put(msg)

        thread = Thread(target=thread_entry)
        thread.start()
        threads.append(thread)

    while True:
        msg = msg_queue.get()
        dst_con.mav.send(msg)


if __name__ == "__main__":
    # Запускать. например, так
    # ./mav_relay.py --dst-mavuri=localhost:2020 --src-ports 3000 3001 3002
    argv = sys.argv[1:]
    exit(main(argv))
