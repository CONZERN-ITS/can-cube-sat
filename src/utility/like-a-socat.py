import time
import sys
import socket
import argparse
import logging

from serial import Serial
from serial.threaded import ReaderThread, Protocol

_log = logging.getLogger(__name__)

REPORT_DELTA = 1  # Периодичность отчетов

# Строчка для тестового socat порта, чтобы не потерялась
# sudo socat -d -d
# PTY,raw,echo=0,group=uucp,mode=0660,link=/dev/ttyVA00
# PTY,raw,echo=0,group=uucp,mode=0660,link=/dev/ttyVB00


def build_protocol_class(target_host, target_port):

    class RelayProtocol(Protocol):
        def __init__(self):
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.connect((target_host, target_port,))
            self.last_report = time.time()
            self.bytes_xferred = 0

        def data_received(self, data):
            self.sock.sendall(data)
            self.bytes_xferred += len(data)
            now = time.time()

            if now - self.last_report >= REPORT_DELTA:
                _log.info(f"{self.bytes_xferred} total bytes relayed")
                self.last_report = now

    return RelayProtocol


def main(argv):

    logging.basicConfig(
        stream=sys.stdout,
        level=logging.INFO,
        format='%(asctime)-15s - %(message)s'
    )

    parser = argparse.ArgumentParser(
        description="Primitive socat-like relay from serial to tcp"
    )
    parser.add_argument('--serial', nargs='?', type=str, required=True)
    parser.add_argument('--host', nargs='?', type=str, required=True)
    parser.add_argument('--port', nargs='?', type=int, required=True)
    parser.add_argument('--baud', nargs='?', type=int, default=19200)

    args = parser.parse_args(argv)

    serial_name = args.serial
    serial_baud = args.baud
    target_host = args.host
    target_port = args.port

    _log.info(f"launching for serial port {serial_name}, baud {serial_baud}.")
    _log.info(f"relaying to {target_host}:{target_port}")

    tty = Serial(
        port=serial_name,
        baudrate=serial_baud,
        bytesize=8,
        parity='N',
        stopbits=1,
        timeout=None,
        xonxoff=0,
        rtscts=0,
    )

    ProtoClass = build_protocol_class(target_host, target_port)
    reader = ReaderThread(tty, ProtoClass)
    reader.start()
    print("reader thread started")
    reader.join()


if __name__ == "__main__":
    argv = sys.argv[1:]
    argv = [
        '--serial=/dev/ttyVB00',
        '--baud=19200',
        '--host=localhost',
        '--port=2020'
    ]

    exit(main(argv))
