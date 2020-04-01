from PyQt5 import QtWidgets, QtGui, QtCore
import pyqtgraph as PyQtGraph
import numpy as NumPy
import time
from sourse import settings_control
from sourse.data_control import *
from sourse.map_sourse.open_street_map import *

APP_ICON_PATH = "./sourse/images/StrelA_MS.png"
WINDOW_ICON_PATH = "./sourse/images/window.png"
MAP_ICON_PATH = "./sourse/images/map.png"
MODEL_ICON_PATH = "./sourse/images/model.png"
GRAPH_ICON_PATH = "./sourse/images/graph.png"


class AbstractProperties(QtWidgets.QWidget):
    def __init__(self):
        super(AbstractProperties, self).__init__()
        self.settings = settings_control.init_settings()

        self.setup_ui()
        self.setup_ui_design()

    def setup_ui(self):
        self.grid_layout = QtWidgets.QGridLayout(self)
        self.apply_btn = QtWidgets.QPushButton()
        self.apply_btn.setSizePolicy(QtWidgets.QSizePolicy.Fixed, QtWidgets.QSizePolicy.Fixed)
        self.default_btn = QtWidgets.QPushButton()
        self.default_btn.setSizePolicy(QtWidgets.QSizePolicy.Fixed, QtWidgets.QSizePolicy.Fixed)

    def add_btn(self):
        self.grid_layout.setRowStretch(self.grid_layout.rowCount(), 1)
        self.grid_layout.addWidget(self.default_btn, self.grid_layout.rowCount(), 0)
        self.grid_layout.addWidget(self.apply_btn, self.grid_layout.rowCount() - 1, self.grid_layout.columnCount() - 1)

    def setup_ui_design(self):
        self.apply_btn.setText("Apply")
        self.default_btn.setText("Set to default")

    def add_h_line(self, width):
        frame = QtWidgets.QFrame()
        frame.setFrameStyle(QtWidgets.QFrame.Plain)
        frame.setFrameShape(QtWidgets.QFrame.HLine)
        frame.setLineWidth(width)
        self.grid_layout.addWidget(frame, self.grid_layout.rowCount(), 0, 1, -1)

    def add_edit_group(self, edit_num):
        edit_group = [QtWidgets.QLabel()]
        self.grid_layout.addWidget(edit_group[0], self.grid_layout.rowCount(), 0)
        for i in range(1, edit_num + 1):
            edit_group.append(QtWidgets.QLineEdit())
            self.grid_layout.addWidget(edit_group[i], self.grid_layout.rowCount() - 1, i)
        return edit_group

    def add_check_box(self, mode):
        check_box = [QtWidgets.QLabel(), QtWidgets.QCheckBox()]
        check_box[1].setTristate(mode)
        self.grid_layout.addWidget(check_box[0], self.grid_layout.rowCount(), 0)
        self.grid_layout.addWidget(check_box[1], self.grid_layout.rowCount() - 1, 1)
        return check_box

    def add_combo_box(self):
        combo_box = [QtWidgets.QLabel(), QtWidgets.QComboBox()]
        self.grid_layout.addWidget(combo_box[0], self.grid_layout.rowCount(), 0)
        self.grid_layout.addWidget(combo_box[1], self.grid_layout.rowCount() - 1, 1, 1, -1)
        return combo_box

    def read_from_edit_group(self, edit_group, value_name, func):
        try:
            if (len(edit_group) > 2):
                value = []
                for edit in edit_group[1:]:
                    value.append(func(edit.text()))
            else:
                value = func(edit_group[1].text())
            self.settings.setValue(value_name, value)
        except Exception as e:
            print(e)


class CommonProperties(AbstractProperties):
    def __init__(self):
        super(CommonProperties, self).__init__()

    def setup_ui(self):
        super(CommonProperties, self).setup_ui()
        self.main_window_size = self.add_edit_group(2)
        self.add_btn()

    def setup_ui_design(self):
        super(CommonProperties, self).setup_ui_design()
        self.settings.beginGroup("MainWindow")
        self.main_window_size[0].setText('Window size')
        for i in range(2):
            self.main_window_size[i + 1].setText(str(self.settings.value('size')[i]))
        self.settings.endGroup()

    def save_properties(self):
        self.settings.beginGroup("MainWindow")
        self.read_from_edit_group(self.main_window_size, 'size', int)
        self.settings.endGroup()


class GraphProperties(AbstractProperties):
    class PlotPropertiesWidget(AbstractProperties):
        def __init__(self, plot):
            self.change_plot(plot)
            super(GraphProperties.PlotPropertiesWidget, self).__init__()

        def change_plot(self, plot):
            self.plot = plot

        def setup_ui(self):
            super(GraphProperties.PlotPropertiesWidget, self).setup_ui()
            self.name_label = QtWidgets.QLabel()
            self.name_label.setAlignment(QtCore.Qt.AlignHCenter)
            self.grid_layout.addWidget(self.name_label, self.grid_layout.rowCount(), 0, 1, -1)
            self.curve_count = self.add_edit_group(1)
            self.curve_colour = self.add_edit_group(1)

        def setup_ui_design(self):
            super(GraphProperties.PlotPropertiesWidget, self).setup_ui_design()
            self.name_label.setText("Curve properties for " + self.plot + " plot")
            self.settings.beginGroup("CentralWidget/GraphWidget/Graph/" + self.plot)
            self.curve_count[0].setText('Curves count')
            self.curve_count[1].setText(str(self.settings.value('count')))
            self.curve_colour[0].setText('Curves colour\n(b,g,r,c,m,y,k,w)')
            self.curve_colour[1].setText(str(self.settings.value('colour')))
            self.settings.endGroup()

        def save_properties(self):
            self.settings.beginGroup("CentralWidget/GraphWidget/Graph/" + self.plot)
            self.read_from_edit_group(self.curve_count, 'count', int)
            self.read_from_edit_group(self.curve_colour, 'colour', str)
            while len(self.settings.value('colour')) < self.settings.value('count'):
                self.settings.setValue('colour', self.settings.value('colour') + 'g')
            self.settings.endGroup()

    def upload_plots(self):
        self.settings.beginGroup("CentralWidget/GraphWidget/Graph")
        groups = self.settings.childGroups()
        self.settings.endGroup()
        return groups

    def setup_ui(self):
        super(GraphProperties, self).setup_ui()
        self.graph_check_box = self.add_check_box(False)
        self.graph_position = self.add_edit_group(4)
        self.add_h_line(2)
        self.plot_dict = {}
        for group in self.upload_plots():
            self.plot_dict.update([(group, [self.add_check_box(False), self.add_edit_group(4)])])
            self.add_h_line(1)
        self.plot_combo_box = self.add_combo_box()
        self.plot_combo_box[1].addItems(self.upload_plots())
        self.plot_combo_box[1].activated.connect(self.change_plot_properties)
        self.property  = GraphProperties.PlotPropertiesWidget(self.plot_combo_box[1].currentText())
        self.grid_layout.addWidget(self.property, self.grid_layout.rowCount(), 0, 1, -1)
        self.add_btn()

    def upload_plots_position(self):
        for plot in self.plot_dict.items():
            self.settings.beginGroup("CentralWidget/GraphWidget/Graph/" + plot[0])
            check_box = plot[1][0]
            edit_group = plot[1][1]
            check_box[0].setText(plot[0] + ' plot')
            check_box[1].setCheckState(int(self.settings.value('is_on')))
            edit_group[0].setText('Position (position, span)')
            for i in range(4):
                edit_group[i + 1].setText(str(self.settings.value('position')[i]))
            self.settings.endGroup()

    def setup_ui_design(self):
        super(GraphProperties, self).setup_ui_design()
        self.settings.beginGroup("CentralWidget/GraphWidget")
        self.graph_check_box[0].setText('Graph widget')
        self.graph_check_box[1].setCheckState(int(self.settings.value('is_on')))
        self.graph_position[0].setText('Widget position (position, span)')
        for i in range(4):
            self.graph_position[i + 1].setText(str(self.settings.value('position')[i]))
        self.plot_combo_box[0].setText('Graph')
        self.settings.endGroup()
        self.upload_plots_position()

    def change_plot_properties(self):
        self.property.change_plot(self.plot_combo_box[1].currentText())
        self.property.setup_ui_design()

    def save_plots_position(self):
        for plot in self.plot_dict.items():
            self.settings.beginGroup("CentralWidget/GraphWidget/Graph/" + plot[0])
            check_box = plot[1][0]
            edit_group = plot[1][1]
            self.settings.setValue('is_on', check_box[1].checkState())
            self.read_from_edit_group(edit_group, 'position', int)
            self.settings.endGroup()

    def save_properties(self):
        self.settings.beginGroup("CentralWidget/GraphWidget")
        self.settings.setValue('is_on', self.graph_check_box[1].checkState()) 
        self.read_from_edit_group(self.graph_position, 'position', int)
        self.settings.endGroup()
        self.save_plots_position()
        self.property.save_properties()


class MapProperties(AbstractProperties):
    def __init__(self):
        super(MapProperties, self).__init__()


class ModelProperties(AbstractProperties):
    def __init__(self):
        super(ModelProperties, self).__init__()


class SettingsWindow(QtWidgets.QMainWindow):
    change_settings = QtCore.pyqtSignal()

    def __init__(self):
        super(SettingsWindow, self).__init__()
        self.settings = settings_control.init_settings()

        self.setWindowIcon(QtGui.QIcon(APP_ICON_PATH))
        self.resize(400, 600)
        self.setSizePolicy(QtWidgets.QSizePolicy.Fixed, QtWidgets.QSizePolicy.Fixed)
        self.move_to_center()

        self.setup_ui()
        self.setup_ui_design()

    def setup_ui(self):
        self.toolbar = self.addToolBar('Settings')
        self.toolbar.setMovable(False)
        self.toolbar.setIconSize(QtCore.QSize(50, 50))
        self.toolbar.setContextMenuPolicy(QtCore.Qt.PreventContextMenu)

        self.central_widget_btn = self.toolbar.addAction(QtGui.QIcon(WINDOW_ICON_PATH), 'Common')
        self.central_widget_btn.triggered.connect(self.show_common_properties)
        self.GraphProperties_btn = self.toolbar.addAction(QtGui.QIcon(GRAPH_ICON_PATH), 'Graph')
        self.GraphProperties_btn.triggered.connect(self.show_graph_properties)
        self.MapProperties_btn = self.toolbar.addAction(QtGui.QIcon(MAP_ICON_PATH), 'Map')
        self.MapProperties_btn.triggered.connect(self.show_map_properties)
        self.ModelProperties_btn = self.toolbar.addAction(QtGui.QIcon(MODEL_ICON_PATH), 'Model')
        self.ModelProperties_btn.triggered.connect(self.show_model_properties)

        self.show_common_properties()

    def setup_ui_design(self):
        self.central_widget.setup_ui_design()

    def set_settings_to_default(self):
        settings_control.set_to_default(self.settings)
        self.settings = settings_control.init_settings()
        self.setup_ui_design()
        self.change_settings.emit()
        
    def show_common_properties(self):
        self.central_widget = CommonProperties()
        self.show_central_widget()

    def show_graph_properties(self):
        self.central_widget = GraphProperties()
        self.show_central_widget()

    def show_map_properties(self):
        self.central_widget = MapProperties()
        self.show_central_widget()

    def show_model_properties(self):
        self.central_widget = ModelProperties()
        self.show_central_widget()

    def show_central_widget(self):
        self.central_widget.default_btn.clicked.connect(self.set_settings_to_default)
        self.central_widget.apply_btn.clicked.connect(self.set_properties)
        if self.centralWidget() is not None:
            self.takeCentralWidget()

        self.setCentralWidget(self.central_widget)

    def settings_enabled(self, mode):
        self.central_widget.setEnabled(mode)

    def set_properties(self):
        self.central_widget.save_properties()
        self.change_settings.emit()

    def removeToolBar(self, toolbar):
        super(SettingsWindow, self).removeToolBar()

    def show(self):
        self.hide()
        super(SettingsWindow, self).show()

    def move_to_center(self):
        frame = self.frameGeometry()
        frame.moveCenter(QtWidgets.QDesktopWidget().availableGeometry().center())
        self.move(frame.topLeft())


class MapWidget(OpenStreetMap):
    def __init__(self):
        super(MapWidget, self).__init__()
        self.settings = settings_control.init_settings()

        self.setup_ui()
        self.setup_ui_design()

    def setup_ui(self):
        pass

    def setup_ui_design(self):
        pass

    def new_data_reaction(self, data):
        pass               

    def clear_data(self):
        pass


class GraphWidget(PyQtGraph.GraphicsLayoutWidget):
    class Curve():
        def __init__(self, plot, pen):
            self.plot = plot
            self.arr = None
            self.curve = None
            self.pen = pen

        def show_data(self, data):
            if self.arr is None:
                self.arr = NumPy.array(data)
                if self.curve is None:
                    self.curve = self.plot.plot(self.arr, pen=self.pen)
                    return
            else:
                self.arr = NumPy.vstack((self.arr, NumPy.array(data)))
            self.curve.setData(self.arr)

        def clear(self):
            self.arr = None

    def __init__(self):
        super(GraphWidget, self).__init__()
        self.settings = settings_control.init_settings()

        self.setup_ui()
        self.setup_ui_design()

    def setup_ui(self):
        self.plot_list = []
        self.plot_dict = {}

    def setup_graph(self, pos, name):
        axis_x = PyQtGraph.AxisItem(orientation='left')
        axis_x.setLabel(name)
        axis_y = PyQtGraph.AxisItem(orientation='bottom')
        axis_y.setLabel("Time")
        return self.addPlot(int(pos[0]), int(pos[1]), int(pos[2]), int(pos[3]), axisItems={'left': axis_x, 'bottom': axis_y})

    def setup_curves(self, plot, count, colour):
        curves = []
        for i in range(count):
            curves.append(GraphWidget.Curve(plot, colour[i]))
        return tuple(curves)

    def setup_ui_design(self):
        self.plot_dict.clear()
        for plot in self.plot_list:
            try:
                self.removeItem(plot)
            except Exception as e:
                print(e)
        self.setup_ui()

        self.settings.beginGroup("CentralWidget/GraphWidget")
        self.settings.beginGroup("Graph")
        for group in self.settings.childGroups():
            self.settings.beginGroup(group)
            if int(self.settings.value("is_on")):
                self.plot_list.append(self.setup_graph(self.settings.value("position"), group))
                self.plot_dict.update([(group, self.setup_curves(self.plot_list[-1],
                                                                 int(self.settings.value("count")),
                                                                 self.settings.value("colour")))])
            self.settings.endGroup()
        self.settings.endGroup()
        self.settings.endGroup()

    def new_data_reaction(self, data):
        for plot in self.plot_dict.items():
            plot_buf = []
            self.settings.beginGroup("CentralWidget/GraphWidget/Graph/" + plot[0])
            for i in range(len(data)):
                if (self.settings.value("packet_name") == data[i][0]) and ((len(data[i]) - 2) >= len(plot[1])):
                    plot_buf.append(data[i])
            if len(plot_buf) > 0:
	            for i in range(len(plot[1])):
	                curve_buf = []
	                for pack in plot_buf:  
	                    curve_buf.append((pack[1], pack[i + 2]))
	                plot[1][i].show_data(curve_buf)
            self.settings.endGroup()

    def clear_data(self):
        for plot in self.plot_dict.items():
            for curve in plot[1]:
                curve.clear()


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
            self.widgets_dict.update([("GraphWidget", GraphWidget())])
        if self.settings.value("MapWidget/is_on"):
            self.widgets_dict.update([("MapWidget", MapWidget())])

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

        self.setWindowIcon(QtGui.QIcon(APP_ICON_PATH))

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

        self.settings_window = SettingsWindow()

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



