from PyQt5 import QtWidgets, QtGui, QtCore
from open_street_map import *

class CentralWidget(OpenStreetMap):
    def __init__(self):
        super(CentralWidget, self).__init__()


class MainWindow(QtWidgets.QMainWindow):
    def __init__(self):
        super(MainWindow, self).__init__()

        self.setup_ui()
        self.setup_ui_design()

        self.move_to_center()

    def setup_ui(self):
        self.central_widget = CentralWidget()
        self.setCentralWidget(self.central_widget)

    def setup_ui_design(self):
        pass#self.central_widget.setup_ui_design()

    def move_to_center(self):
        frame = self.frameGeometry()
        frame.moveCenter(QtWidgets.QDesktopWidget().availableGeometry().center())
        self.move(frame.topLeft())