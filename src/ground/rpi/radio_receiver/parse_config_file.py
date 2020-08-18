import json


def read_config(path):
    with open(path) as config_file:
        data = json.load(config_file)
    return data


def parse_config(arg, config_data):
    arg.serial_device = config_data['serial_device']
    arg.serial_baudrate = config_data['serial_baudrate']
    arg.output = config_data['output']
    arg.logs_path = config_data['logs_path']
    arg.packet_logfile_basename = config_data['packet_logfile_basename']
    arg.raw_logfile_basename = config_data['raw_logfile_basename']
    arg.logfile_extension = config_data['logfile_extension']
    arg.print = config_data['print']
    arg.RSSI = config_data['RSSI']
    return arg