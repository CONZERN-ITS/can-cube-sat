import argparse


def arguments():
    parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)

    # input
    input_group = parser.add_argument_group("arguments for input connection")
    input_group.add_argument("-s", "--serial_device", help="your input serial device", required=True)
    input_group.add_argument("-b", "--serial_baudrate", help="your serial baudrate", default="115200")

    # output
    outout_group = parser.add_argument_group("arguments for output connection")
    outout_group.add_argument("-pr", "--output_protocol", help="your output protocol", default="udp")
    outout_group.add_argument("-a", "--output_address", help="your output address", default="127.0.0.1")
    outout_group.add_argument("-p", "--output_port", help="your output port", default="13131")

    # logs
    logs_group = parser.add_argument_group("arguments for logs")
    logs_group.add_argument("-lp", "--logs_path", help="path to logs directory with name logs directory",
                            default="logs")
    logs_group.add_argument("-plf", "--mavlink_packet_log_filename", help="name for file with logs without extantion",
                            default="mavlink_packet_log_")
    logs_group.add_argument("-tplf", "--mavlink_time_packet_log_filename",
                            help="name for file with logs without extantion",default="mavlink_time_packet_log_")
    logs_group.add_argument("-e", "--extantion", help="extantion for log files without dot", default="txt")

    # other
    other_group = parser.add_argument_group("other arguments")
    other_group.add_argument("--print", help="print recieve packets", type=bool, default=0)

    args = parser.parse_args()
    return args
