#!/usr/bin/python3
from PyQt5 import QtWidgets, QtCore
from sys import argv, exit

from source.source import MainWindow

if __name__ == "__main__":
    QtCore.QCoreApplication.setOrganizationName("ITS")
    QtCore.QCoreApplication.setApplicationName("StrelA_MS")

    application = QtWidgets.QApplication(argv)
    window = MainWindow()
    window.show()
    exit(application.exec_())
    application.exit()
