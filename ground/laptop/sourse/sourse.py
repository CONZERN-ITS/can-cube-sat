from PyQt5 import QtWidgets, QtGui, QtCore
from sourse import settings_control
import pyqtgraph as PyQtGraph
import numpy as NumPy
import time

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

    def setup_ui_design(self):
        pass

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
            self.exeption_action(e)

    def exeption_action(self, e):
        if self.status_bar is not None:
            self.status_bar.showMessage(str(e))
        print(e)


class CommonProperties(AbstractProperties):
    def __init__(self):
        super(CommonProperties, self).__init__()

    def setup_ui(self):
        super(CommonProperties, self).setup_ui()
        self.main_window_size = self.add_edit_group(2)
        self.grid_layout.addWidget(self.apply_btn, self.grid_layout.rowCount(), self.grid_layout.columnCount() - 1)
        self.grid_layout.addWidget(self.default_btn, self.grid_layout.rowCount() - 1, self.grid_layout.columnCount() - 2)

    def setup_ui_design(self):
        super(CommonProperties, self).setup_ui_design()
        self.settings.beginGroup("MainWindow")
        self.main_window_size[0].setText('Window size')
        for i in range(2):
            self.main_window_size[i + 1].setText(str(self.settings.value('size')[i]))
        self.settings.endGroup()
        self.apply_btn.setText("Apply")
        self.default_btn.setText("Set to default")

    def save_properties(self):
        self.settings.beginGroup("MainWindow")
        self.read_from_edit_group(self.main_window_size, 'size', int)
        self.settings.endGroup()


class GraphProperties(AbstractProperties):
    def setup_ui(self):
        super(GraphProperties, self).setup_ui()
        self.graph_check_box = self.add_check_box(False)

        self.grid_layout.addWidget(self.apply_btn, self.grid_layout.rowCount(), self.grid_layout.columnCount() - 1)
        self.grid_layout.addWidget(self.default_btn, self.grid_layout.rowCount()-1, self.grid_layout.columnCount() - 2)

    def setup_ui_design(self):
        super(GraphProperties, self).setup_ui_design()
        self.settings.beginGroup("CentralWidget/GraphWidget")
        self.graph_check_box[0].setText('Graph widget')
        self.graph_check_box[1].setCheckState(int(self.settings.value('is_on')))
        self.settings.endGroup()
        self.apply_btn.setText("Apply")
        self.default_btn.setText("Set to default")

    def save_properties(self):
        self.settings.beginGroup("CentralWidget/GraphWidget")
        self.settings.setValue('is_on', self.graph_check_box[1].checkState()) 
        self.settings.endGroup()


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

    def set_properties(self):
        self.central_widget.save_properties()
        self.change_settings.emit()

    def removeToolBar(self, toolbar):
        super(SettingsWindow, self).removeToolBar()

    def show(self):
        self.hide()
        super(SettingsWindow, self).show()


class GraphWidget(PyQtGraph.GraphicsLayoutWidget):
    class Curve():
        def __init__(self, plot):
            self.plot = plot
            self.pen = 'r'
            self.arr = None
            self.curve = None

        def show_data(self, data):
            if self.arr is None:
                self.arr = NumPy.array([data[0], self.data_extractor(data[self.number])])
                if self.curve is None:
                    self.curve = self.plot.plot(self.arr, pen=self.pen)
                    return
            else:
                self.arr = NumPy.vstack((self.arr, NumPy.array([data[0], self.data_extractor(data[self.number])])))
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

    def setup_curves(self, plot, count):
        curves = []
        for i in range(count):
            curves += [self.Curve(plot)]
        return curves

    def setup_ui_design(self):
        self.plot_dict.clear()
        for plot in self.plot_list:
            self.removeItem(plot)
        self.setup_ui()

        self.settings.beginGroup("CentralWidget/GraphWidget")
        self.settings.beginGroup("Graph")
        for group in self.settings.childGroups():
            self.settings.beginGroup(group)
            if self.settings.value("is_on"):
                self.plot_list.append(self.setup_graph(self.settings.value("position"), "Height"))
                self.plot_dict.update([(group, self.setup_curves(self.plot_list[-1],
                                                                 self.settings.value("count")))])
            self.settings.endGroup()
        self.settings.endGroup()
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
        for key in self.widgets_dict.keys():
            self.settings.beginGroup(key)
            if int(self.settings.value("is_on")):
                pos = self.settings.value("position")
                self.grid_layout.addWidget(self.widgets_dict[key], int(pos[0]), int(pos[1]), int(pos[2]), int(pos[3]))
                self.widgets_dict[key].setup_ui_design()
            self.settings.endGroup()
        self.settings.endGroup()

    def setup_ui_design(self):
        self.settings.beginGroup("CentralWidget")
        self.settings.endGroup()


class MainWindow(QtWidgets.QMainWindow):
    def __init__(self):
        super(MainWindow, self).__init__()
        self.settings = settings_control.init_settings()

        if not settings_control.settings_test(self.settings):
            settings_control.set_to_default(self.settings)
            self.settings = settings_control.init_settings()
            self.settings.setValue('MainWindow/size', [1000, 800])
        self.settings.setValue('CentralWidget/GraphWidget/position', [0,0,0,0])

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

        self.settings_window.change_settings.connect(self.setup_ui_design)

    def setup_ui_design(self):
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

    def closeEvent(self, evnt):
        self.settings_window.close()
        super(MainWindow, self).closeEvent(evnt)