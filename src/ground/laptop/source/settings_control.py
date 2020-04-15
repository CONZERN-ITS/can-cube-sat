from PyQt5 import QtWidgets, QtGui, QtCore
import os.path
from source import RES_ROOT, USER_SETTINGS_PATH

DEFAULT_SETTINGS_PATH = os.path.join(RES_ROOT, "StrelA_MS_default.ini")

APP_ICON_PATH = os.path.join(RES_ROOT, "images/StrelA_MS.png")
WINDOW_ICON_PATH = os.path.join(RES_ROOT, "images/window.png")
MAP_ICON_PATH = os.path.join(RES_ROOT, "images/map.png")
MODEL_ICON_PATH = os.path.join(RES_ROOT, "images/model.png")
GRAPH_ICON_PATH = os.path.join(RES_ROOT, "images/graph.png")

def init_settings():
    settings = QtCore.QSettings(USER_SETTINGS_PATH, QtCore.QSettings.IniFormat)
    return settings

def set_to_default(settings):
    config = open(settings.fileName(), 'w')
    def_config = open(DEFAULT_SETTINGS_PATH, 'r')
    for line in def_config:
        print(line)
        config.write(line)
    config.close()
    def_config.close()

def settings_test(settings):
    return os.path.exists(settings.fileName())


class AbstractProperties(QtWidgets.QWidget):
    def __init__(self):
        super(AbstractProperties, self).__init__()
        self.settings = init_settings()

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
        self.settings = init_settings()

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
        self.settings = init_settings()
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
