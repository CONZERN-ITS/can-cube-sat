import argparse


def arguments():
    parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)

    # input
    input_group = parser.add_argument_group("arguments for input connection")
    input_group.add_argument("-s", "--serial_device", help="your input serial device", required=True)
    input_group.add_argument("-b", "--serial_baudrate", help="your serial baudrate", default="115200")

    # output
    outout_group = parser.add_argument_group("arguments for output connection")
    outout_group.add_argument(
        "-o", "--output", nargs="+", help="your output mavutil stream",
        default="udp:localhost:4444"
    )

    # logs
    logs_group = parser.add_argument_group("arguments for logs")
    logs_group.add_argument("-lp", "--logs_path", help="path to logs directory with name logs directory",
                            default="logs")
    logs_group.add_argument("-plf", "--packet_logfile_basename", help="base name for packet logfile",
                            default="its_packet_log_")
    logs_group.add_argument("-rlf", "--raw_logfile_basename", help="base name for raw logfile",
                            default="its_raw_log_")
    logs_group.add_argument("-e", "--logfile_extension", help="extension for log files without dot",
                            default="mavlog")

    # other
    other_group = parser.add_argument_group("other arguments")
    other_group.add_argument("--print", help="print received packets in stdout", action="store_true", default=False)
    other_group.add_argument("--RSSI", help="receive packets have RSSI byte", action="store_true", default=False)

    args = parser.parse_args()
    return args
