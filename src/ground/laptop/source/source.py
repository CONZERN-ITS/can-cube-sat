from PyQt5 import QtWidgets, QtGui, QtCore
import time
from source import settings_control
from source import map_widget
from source import graph_widget
from source import model_widget
from source.data_control import *


class CentralWidget(QtWidgets.QWidget):
    def __init__(self):
        super(CentralWidget, self).__init__()
        self.settings = settings_control.init_settings()
        self.widgets_dict = {}

        self.setup_ui()
        self.setup_ui_design()

    def setup_ui(self):
        self.grid_layout = QtWidgets.QGridLayout(self)
        self.settings.beginGroup("CentralWidget")

        if self.settings.value("GraphWidget/is_on"):
            self.widgets_dict.update([("GraphWidget", graph_widget.GraphWidget())])
        if self.settings.value("MapWidget/is_on"):
            self.widgets_dict.update([("MapWidget", map_widget.MapWidget())])
        if self.settings.value("ModelWidget/is_on"):
            self.widgets_dict.update([("ModelWidget", model_widget.ModelWidget())])

        for key in self.widgets_dict.keys():
            self.settings.beginGroup(key)
            if int(self.settings.value("is_on")):
                pos = self.settings.value("position")
                self.grid_layout.addWidget(self.widgets_dict[key], int(pos[0]), int(pos[1]), int(pos[2]), int(pos[3]))
            self.settings.endGroup()
        self.settings.endGroup()

    def new_data_reaction(self, data):
        for widget in self.widgets_dict.items():
            widget[1].new_data_reaction(data)

    def clear_data(self):
        for widget in self.widgets_dict.items():
            widget[1].clear_data()

    def setup_ui_design(self):
        for widget in self.widgets_dict.items():
            widget[1].setup_ui_design()


class MainWindow(QtWidgets.QMainWindow):
    class DataManager(QtCore.QObject):
        new_data = QtCore.pyqtSignal(tuple)
        autoclose = QtCore.pyqtSignal(str)
        def __init__(self, data_obj):
            super(MainWindow.DataManager, self).__init__()
            self.data_obj = data_obj
            self.mutex = QtCore.QMutex()
            self.set_close_flag(False)

        def set_close_flag(self, mode):
            self.mutex.lock()
            self.close_flag = mode
            self.mutex.unlock()

        def change_data_obj(self, data_obj):
            self.data_obj = data_obj

        def start(self):
            self.set_close_flag(False)
            close = False
            try:
                self.data_obj.start()
            except Exception as e:
                self.autoclose.emit(str(e))
                return
            start_time = time.time()
            data_buf = []
            while not close:
                try:
                    data = self.data_obj.read_data()
                except RuntimeError:
                    pass
                except EOFError as e:
                    self.autoclose.emit(str(e))
                    break
                except Exception as e:
                    print(e)
                else:
                    data_buf.append(data)
                    if (time.time() - start_time) > 0.1:
                        self.new_data.emit(tuple(data_buf))
                        start_time = time.time()
                        data_buf = []
                self.mutex.lock()
                close = self.close_flag
                self.mutex.unlock()

        def quit(self):
            self.set_close_flag(True)
            time.sleep(0.01)
            try:
                self.data_obj.stop()
            except Exception as e:
                pass

    def __init__(self):
        super(MainWindow, self).__init__()
        self.settings = settings_control.init_settings()

        if not settings_control.settings_test(self.settings):
            settings_control.set_to_default(self.settings)
            self.settings = settings_control.init_settings()

        self.setWindowIcon(QtGui.QIcon(settings_control.APP_ICON_PATH))

        self.setup_ui()
        self.setup_ui_design()

        self.move_to_center()

    def setup_ui(self):
        self.toolbar = self.addToolBar('Common')
        self.toolbar.setMovable(False)
        self.toolbar.setIconSize(QtCore.QSize(50, 50))
        self.toolbar.setContextMenuPolicy(QtCore.Qt.PreventContextMenu)
        self.connection_btn = self.toolbar.addAction('Connect')
        self.connection_btn.triggered.connect(self.connection_action)

        self.settings_window = settings_control.SettingsWindow()

        self.menu_bar = self.menuBar()
        self.menu_file = self.menu_bar.addMenu("&File")
        self.action_settings = self.menu_file.addAction("&Settings")
        self.action_settings.setShortcut('Ctrl+S')
        self.action_settings.triggered.connect(self.settings_window.show)
        self.action_exit = self.menu_file.addAction("&Exit")
        self.action_exit.setShortcut('Ctrl+Q')
        self.action_exit.triggered.connect(QtWidgets.qApp.quit)

        self.central_widget = CentralWidget()
        self.setCentralWidget(self.central_widget)
        self.data_obj = self.get_data_object()
        self.data_manager = MainWindow.DataManager(self.data_obj)
        self.data_thread = QtCore.QThread(self)
        self.data_manager.moveToThread(self.data_thread)
        self.data_thread.started.connect(self.data_manager.start)
        self.data_manager.new_data.connect(self.central_widget.new_data_reaction)
        self.data_manager.autoclose.connect(self.connection_action)

        self.settings_window.change_settings.connect(self.setup_ui_design)

    def setup_ui_design(self):
        if not self.data_thread.isRunning():
            self.resize(int(self.settings.value('MainWindow/size')[0]), int(self.settings.value('MainWindow/size')[1]))
            self.setWindowTitle("StrelA MS")
            self.menu_file.setTitle("&File")
            self.action_settings.setText("&Settings")
            self.action_settings.setStatusTip("Settings")
            self.action_exit.setText("&Exit")
            self.action_exit.setStatusTip("Exit")

            self.central_widget.setup_ui_design()
            self.settings_window.setup_ui_design()

    def move_to_center(self):
        frame = self.frameGeometry()
        frame.moveCenter(QtWidgets.QDesktopWidget().availableGeometry().center())
        self.move(frame.topLeft())

    def get_data_object(self):
        self.settings.beginGroup('MainWindow/DataSourse')
        sourse = self.settings.value('type')
        if sourse == 'Log':
            log = self.settings.value('Log/type')
            if log == "TXT":
                data = TXTLogDataSource(self.settings.value('Log/path'),
                                 int(self.settings.value('Log/real_time')),
                                 float(self.settings.value('Log/time_delay')),
                                 int(self.settings.value('Log/time_from_zero')))
            elif log == "MAV":
                data = MAVLogDataSource(self.settings.value('Log/path'),
                                     int(self.settings.value('Log/real_time')),
                                     float(self.settings.value('Log/time_delay')),
                                     int(self.settings.value('Log/time_from_zero')))
        elif sourse == 'MAVLink':
            data = MAVDataSource(self.settings.value('MAVLink/connection'))
        self.settings.endGroup()
        return data

    def closeEvent(self, evnt):
        self.settings_window.close()
        super(MainWindow, self).closeEvent(evnt)

    def connection_action(self, stat_bar_msg=None):
        if not self.data_thread.isRunning():
            self.central_widget.clear_data()
            self.settings_window.settings_enabled(False)
            self.data_thread.start()
            self.connection_btn.setText("&Disconnect")
        else:
            self.data_manager.quit()
            self.data_thread.quit()
            self.connection_btn.setText("&Connect")
            time.sleep(0.1)
            self.settings_window.settings_enabled(True)
        if (stat_bar_msg is not None) and (stat_bar_msg != False):
            print(stat_bar_msg)



