import os
import argparse
import sys
import csv
import collections

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
        if isinstance(the_list, collections.Iterable) and not isinstance(the_list, str):
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

    def __init__(self, base_path, msg_id):
        self.msg_id = msg_id
        self.msg_class = mavlink.mavlink_map[msg_id]
        self.fpath = os.path.join(base_path, self.msg_class.name + ".csv")
        # Инициализация этих откладывается до тех пор, пока мы не не определимся с полями.
        # А определимся мы в первом сообщении, как только развернем в нем все массивы
        # TODO: Можно определиться сразу по полю masg_class.array_lengths
        self.stream = None
        self.writer = None
        self.message_count = 0

    def accept(self, msg):
        msg_dict = msg.to_dict()
        msg_dict = self.expand_arrays(msg_dict)

        # Добавляем поля из загловка, которые нас интересуют
        hdr = msg.get_header()
        msg_dict.update({
            "seq": hdr.seq,
            "srcSystem": hdr.srcSystem,
            "srcComponent": hdr.srcComponent,
        })

        if "mavpackettype" in msg_dict:
            del msg_dict["mavpackettype"]  # Совершенно излишне в нашем случае

        if not self.writer:
            self.stream = open(self.fpath, mode="w", newline='')
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

        msg_dict = msg.to_dict()
        if msg_id not in processors:
            processor = MsgProcessor(base_path, msg_id)
            processors.update({msg_id: processor})

        processor = processors[msg_id]
        processor.accept(msg)

    print("=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-")
    print("Stats in total:")
    for processor in processors.values():
        print("%s: %d messages" % (processor.msg_class.name, processor.message_count))
    print("bad data %s bytes in total" % bad_data_bytes)


if __name__ == "__main__":
    exit(main(sys.argv[1:]))