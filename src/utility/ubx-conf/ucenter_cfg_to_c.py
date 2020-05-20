#!/usr/bin/env python3
import argparse


def make_msg_line(msg_name, msg_bytes):
    msg_bytes_text = ", ".join(["0x%02X" % x for x in msg_bytes])
    msg_line = "static const uint8_t %(msg_name)s[] = {%(msg_data)s};" % {
        "msg_name": msg_name,
        "msg_data": msg_bytes_text
    }

    return msg_line


_file_template = """
/*! Конфигурация gps модуля, сгенерированная из конфигурации в
    формате ucenter.
    Файл сгенерирован автоматически, поэтому менять его не рекомендуется,
    так как все изменения могут быть затерты перегенерацией */

#include <stdint.h>

/*! Статические переменные для отдельных сообщений.
    Эти массивы содержат сообщение полностью, без синхрослова в начале
    и контрольной суммы в конце.
    Однако такие поля как cls_id, msg_id, len в сообщениях содержаться */

%(msg_lines)s

/*! Эта переменная не статическая и предполагается её использование в
    прочих файлах проекта. Эта переменная является этаким
    "двумерным массивом" конфигурационных сообщений.

    Конец массива терменирован нулевым указателем */
const uint8_t * ublox_cfg_msgs[] = {%(msg_names)s, 0};
""".lstrip()


def make_msg_file(msgs):

    msg_names = []
    msg_lines = []

    for msg_no, (msg_name, msg_data,) in enumerate(msgs):
        msg_name += "_%d" % msg_no
        msg_names.append(msg_name)
        msg_lines.append(make_msg_line(msg_name, msg_data))

    return _file_template % {
        "msg_lines": "\n".join(msg_lines),
        "msg_names": ", ".join(msg_names)
    }


def parse_line(line):
    line = line.strip()
    msg_name, msg_data = line.split(" - ")

    # приводим имя сообщения к сишному формату
    msg_name = msg_name.strip().lower().replace("-", "_")
    # кастуем байты в собственно байты
    msg_data = bytearray.fromhex(msg_data.replace(" ", ""))
    return msg_name, msg_data


def convert(input_stream, output_stream):
    lines = input_stream.readlines()

    msgs = []
    for line in lines:
        msg_name, msg_bytes = parse_line(line)
        if not msg_name.startswith("cfg"):
            # Сообщения, которые не cfg, мы пропускаем
            continue

        msgs.append((msg_name, msg_bytes,))
    msg_file = make_msg_file(msgs)
    output_stream.write(msg_file)


def main(argv):
    parser = argparse.ArgumentParser("ucenter-cfg-to-c")

    parser.add_argument(
        "-i,--input", dest="istream", type=argparse.FileType('r'),
        required=True,
    )
    parser.add_argument(
        "-o,--output,-o", dest="ostream", type=argparse.FileType('w'),
        default='-'
    )

    args = parser.parse_args(argv)
    return convert(args.istream, args.ostream)


if __name__ == "__main__":
    import sys
    exit(main(sys.argv[1:]))
