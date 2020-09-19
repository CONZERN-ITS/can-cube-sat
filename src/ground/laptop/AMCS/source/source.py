from PyQt5 import QtWidgets, QtGui, QtCore
import os.path
from source import RES_ROOT
APP_ICON_PATH = os.path.join(RES_ROOT, "images/StrelA_MS.png")

from source import antenna
from source import data_widget
from source import pos_control_widget
from source import log_widget


class CentralWidget(QtWidgets.QWidget):
    def __init__(self, antenna):
        super(CentralWidget, self).__init__()

        self.antenna = antenna

        self.setup_ui()
        self.setup_ui_design()

    def setup_ui(self):
        self.h_layout = QtWidgets.QHBoxLayout(self)

        self.position_widget = data_widget.PositionWidget()
        self.position_control_widget = pos_control_widget.PositionControlWidget()
        self.command_log = log_widget.CommandLogPanel()

        self.h_layout.addWidget(self.position_widget)
        self.h_layout.addWidget(self.position_control_widget)
        self.h_layout.addWidget(self.command_log)
        self.h_layout.setStretch(0, 2)
        self.h_layout.setStretch(1, 1)
        self.h_layout.setStretch(2, 1)

        self.antenna.antenna_pos_changed.connect(self.position_widget.change_antenna_pos)
        self.antenna.target_pos_changed.connect(self.position_widget.change_target_pos)
        self.antenna.lat_lon_alt_changed.connect(self.position_widget.change_lat_lon_alt)
        self.antenna.ecef_changed.connect(self.position_widget.change_ecef)
        self.antenna.top_to_ascs_matrix_changed.connect(self.position_widget.change_top_to_ascs_matrix)

        self.position_control_widget.pos_control_panel.top_btn_clicked.connect(self.antenna.put_up)
        self.position_control_widget.pos_control_panel.bottom_btn_clicked.connect(self.antenna.put_down)
        self.position_control_widget.pos_control_panel.right_btn_clicked.connect(self.antenna.turn_right)
        self.position_control_widget.pos_control_panel.left_btn_clicked.connect(self.antenna.turn_left)
        self.position_control_widget.pos_control_panel.central_btn_clicked.connect(self.antenna.park)
        self.position_control_widget.control_btn.clicked.connect(self.control_btn_action)
        self.position_control_widget.mode_true_btn.toggled.connect(self.antenna.set_mode)

        self.antenna.command_sent.connect(self.command_log.add_data)

    def new_state_reaction(self):
        pass

    def setup_ui_design(self):
        pass

    def control_btn_action(self):
        try:
            self.antenna.manual_control(*[float(line.text()) for line in self.position_control_widget.pos_control_line_edit])
        except ValueError:
            pass


class MainWindow(QtWidgets.QMainWindow):
    class DataManager(QtCore.QObject):
        new_msg = QtCore.pyqtSignal(object)
        def __init__(self, ip, port):
            super(MainWindow.DataManager, self).__init__()
            self.mutex = QtCore.QMutex()
            self._set_close_flag(True)
            self.ip = ip
            self.port = port

        def change_ip_and_port(self, ip, port):
            self.ip = ip
            self.port = port

        def _set_close_flag(self, mode):
            self.mutex.lock()
            self.close_flag = mode
            self.mutex.unlock()

        def start(self):
            self._set_close_flag(False)
            close = False
            self.connection = mavutil.mavlink_connection('udpin:' + ip + ':' + port)
            while not close:
                try:
                    msg = self.connection.recv_match(blocking=True)
                except Exception as e:
                    pass
                if msg is not None:
                    self.new_msg.emit(msg)
                
                self.mutex.lock()
                close = self.close_flag
                self.mutex.unlock()

        def quit(self):
            self._set_close_flag(True)
            time.sleep(0.01)

    def __init__(self):
        super(MainWindow, self).__init__()

        self.setWindowIcon(QtGui.QIcon(APP_ICON_PATH))

        self.setup_ui()
        self.setup_ui_design()

        self.move_to_center()

    def setup_ui(self):
        self.antenna = antenna.CommandSystem()
        self.antenna.start_connection('192.168.1.228', '13404')

        self.toolbar = self.addToolBar('Commands')
        self.auto_control_on_btn = self.toolbar.addAction('Turn on\nautomatic\ncontrol')
        self.auto_control_on_btn.triggered.connect(self.antenna.automatic_control_on)

        self.auto_control_off_btn = self.toolbar.addAction('Turn off\nautomatic\ncontrol')
        self.auto_control_off_btn.triggered.connect(self.antenna.automatic_control_off)

        self.elevation_zero_btn = self.toolbar.addAction('Find elevation\nzero')
        self.elevation_zero_btn.triggered.connect(self.antenna.setup_elevation_zero)

        self.target_to_north_btn = self.toolbar.addAction('Turn target\nto north')
        self.target_to_north_btn.triggered.connect(self.antenna.target_to_north)

        self.setup_coord_system_btn = self.toolbar.addAction('Setup coord\nsystem')
        self.setup_coord_system_btn.triggered.connect(self.antenna.setup_coord_system)

        self.state_request_btn = self.toolbar.addAction('State\nrequest')
        self.state_request_btn.triggered.connect(self.antenna.state_request)

        self.menu_bar = self.menuBar()
        self.menu_file = self.menu_bar.addMenu("&File")
        self.action_exit = self.menu_file.addAction("&Exit")
        self.action_exit.setShortcut('Ctrl+Q')
        self.action_exit.triggered.connect(QtWidgets.qApp.quit)

        self.central_widget = CentralWidget(self.antenna)
        self.setCentralWidget(self.central_widget)

        self.data_manager = MainWindow.DataManager('0.0.0.0', '13404')
        self.data_thread = QtCore.QThread(self)
        self.data_manager.moveToThread(self.data_thread)
        self.data_thread.started.connect(self.data_manager.start)
        self.data_manager.new_msg.connect(self.antenna.new_msg_reaction)

    def setup_ui_design(self):
        self.resize(600, 800)
        self.setWindowTitle("Antenna manual control system")

        self.menu_file.setTitle("&File")
        self.action_exit.setText("&Exit")
        self.action_exit.setStatusTip("Exit")

    def move_to_center(self):
        frame = self.frameGeometry()
        frame.moveCenter(QtWidgets.QDesktopWidget().availableGeometry().center())
        self.move(frame.topLeft())

    def closeEvent(self, evnt):
        self.data_manager.quit()
        self.data_thread.quit()
        super(MainWindow, self).closeEvent(evnt)



