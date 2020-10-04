import os
import argparse
import sys
import csv
import collections
from datetime import datetime
from wgs84 import wgs84_xyz_to_latlonh

if "MAVLINK20" not in os.environ:
    os.environ["MAVLINK20"] = "true"
if "MAVLINK_DIALECT" not in os.environ:
    os.environ["MAVLINK_DIALECT"] = "its"

from pymavlink import mavutil
from pymavlink.mavutil import mavlink


class MsgProcessor:

    def _expand(self,retval, the_list, base_name=""):
        """ Разворачивает список и каждый его элемент записывает в словарь retval
            с ключем "%s[%d]" % (base_name, i), где i - номер элемента """
        if isinstance(the_list, collections.abc.Iterable) and not isinstance(the_list, str):
            for i, value in enumerate(the_list):
                key = "%s[%d]" % (base_name, i)
                self._expand(retval, value, base_name=key)
        else:
            retval[base_name] = the_list

    def expand_arrays(self, msg_dict):
        """ Метод разворачивает элементы сообщения, которые массивы в плоские списки. """
        retval = {}
        for key, value in msg_dict.items():
            self._expand(retval=retval, the_list=value, base_name=key)

        return retval

    def __init__(self, base_path, notimestamps=False):
        # Инициализация этих откладывается до тех пор, пока мы не не определимся с полями.
        # А определимся мы в первом сообщении, как только развернем в нем все массивы
        # TODO: Можно определиться сразу по полю masg_class.array_lengths
        self.base_path = base_path
        self.stream = None
        self.writer = None
        self.message_count = 0
        self.notimestamps = notimestamps

    def accept(self, msg):
        self.msg_id = msg.get_msgId()
        self.msg_class = mavlink.mavlink_map[self.msg_id]

        msg_dict = msg.to_dict()
        msg_dict = self.expand_arrays(msg_dict)

        # Добавляем поля из загловка, которые нас интересуют
        hdr = msg.get_header()
        msg_dict.update({
            "seq": hdr.seq,
            "srcSystem": hdr.srcSystem,
            "srcComponent": hdr.srcComponent,
        })

        # Добавим поле с красивым таймштампом в читаемом виде
        # Если в сообщении есть time_s и time_us
        if "time_s" in msg_dict and "time_us" in msg_dict:
            ts = msg_dict["time_s"] + msg_dict["time_us"] / (1000 * 1000)
            dt = datetime.fromtimestamp(ts)
            ts_text = dt.strftime("%Y-%m-%dT%H:%M:%S")
            msg_dict.update({
                "time_gregorian": ts_text,
            })

        # Добавляем таймштамп из mavlog файла
        if not self.notimestamps:
            ts = msg._timestamp
            dt = datetime.fromtimestamp(ts)
            ts_text = dt.strftime("%Y-%m-%dT%H:%M:%S")
            msg_dict.update({
                "log_timestamp": ts,
                "log_timestamp_gregorian": ts_text
            })

        # Если это GPS_UBX_NAV_SOL
        if "GPS_UBX_NAV_SOL" == self.msg_class.name:
            # Пересчитываем координаты
            lat, lon, h = wgs84_xyz_to_latlonh(msg.ecefX / 100, msg.ecefY / 100, msg.ecefZ / 100)
            msg_dict.update({
                "lat": lat,
                "lon": lon,
                "h": h,
            })

        if "mavpackettype" in msg_dict:
            del msg_dict["mavpackettype"]  # Совершенно излишне в нашем случае

        if not self.writer:
            msg_hdr = msg.get_header()
            fstem = "%s-%s-%s" % (self.msg_class.name, msg_hdr.srcSystem, msg_hdr.srcComponent)
            fpath = os.path.join(self.base_path, fstem + ".csv")

            self.stream = open(fpath, mode="w", newline='')
            self.writer = csv.DictWriter(self.stream, fieldnames=msg_dict.keys())
            self.writer.writeheader()

        print(msg)  # Напечатаем сообщение для наглядности
        self.writer.writerow(msg_dict)
        self.message_count += 1


def main(argv):
    parser = argparse.ArgumentParser("tm parser to csvs", add_help=True)
    parser.add_argument("-i,--input", nargs="?", dest="input", required=True)
    parser.add_argument("-o,--output_dir", nargs="?", dest="output_dir", help="If output is None, input filename is name for output directory", default=None)
    parser.add_argument("--notimestamps", dest="notimestamps", default=False, action='store_true')
    args = parser.parse_args(argv)

    base_path = args.output_dir
    f = args.input
    notimestamps = args.notimestamps

    if not base_path:
        fname = os.path.split(f)[1]
        base_path = os.path.splitext(fname)[0]

    if not os.path.isdir(base_path):
        os.makedirs(base_path, exist_ok=True)

    con = mavutil.mavlogfile(f, notimestamps=notimestamps)
    processors = {}
    bad_data_bytes = 0

    while True:
        msg = con.recv_msg()
        if not msg:
            break

        msg_id = msg.get_msgId()
        if msg_id < 0:
            bad_data_bytes += 1
            continue

        msg_hdr = msg.get_header()
        msg_key = "%s-%s-%s" % (msg_id, msg_hdr.srcSystem, msg_hdr.srcComponent)
        if msg_key not in processors:
            processor = MsgProcessor(base_path, notimestamps=notimestamps)
            processors.update({msg_key: processor})

        processor = processors[msg_key]
        processor.accept(msg)

    print("=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-")
    print("Stats in total:")
    for msg_key, processor in processors.items():
        print("%s-%s: %d messages" % (processor.msg_class.name, msg_key, processor.message_count))
    print("bad data %s bytes in total" % bad_data_bytes)


if __name__ == "__main__":
    exit(main(sys.argv[1:]))