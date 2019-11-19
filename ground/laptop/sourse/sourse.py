from PyQt5 import QtWidgets, QtGui, QtCore
from sourse import settings_control

APP_ICON_PATH = "./sourse/images/StrelA_MS.png"
WINDOW_ICON_PATH = "./sourse/images/window.png"
MAP_ICON_PATH = "./sourse/images/map.png"
MODEL_ICON_PATH = "./sourse/images/model.png"
GRAPH_ICON_PATH = "./sourse/images/graph.png"


class Abstract_properties(QtWidgets.QWidget):
    def __init__(self):
        super(Abstract_properties, self).__init__()
        self.settings = settings_control.init_settings()

        self.setup_ui()
        self.setup_ui_design()

    def setup_ui(self):
        self.layout = QtWidgets.QGridLayout(self)
        self.apply_btn = QtWidgets.QPushButton()
        self.apply_btn.setSizePolicy(QtWidgets.QSizePolicy.Fixed, QtWidgets.QSizePolicy.Fixed)
        self.default_btn = QtWidgets.QPushButton()
        self.default_btn.setSizePolicy(QtWidgets.QSizePolicy.Fixed, QtWidgets.QSizePolicy.Fixed)

    def setup_ui_design(self):
        pass

    def add_edit_group(self, edit_num):
        edit_group = [QtWidgets.QLabel()]
        self.layout.addWidget(edit_group[0], self.layout.rowCount(), 0)
        for i in range(edit_num):
            edit_group.append(QtWidgets.QLineEdit())
            self.layout.addWidget(edit_group[i + 1], self.layout.rowCount() - 1, i + 1)
        return edit_group

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

class Common_properties(Abstract_properties):
    def __init__(self):
        super(Common_properties, self).__init__()

    def setup_ui(self):
        super(Common_properties, self).setup_ui()
        self.main_window_size = self.add_edit_group(2)
        self.layout.addWidget(self.apply_btn, self.layout.rowCount(), self.layout.columnCount() - 1)
        self.layout.addWidget(self.default_btn, self.layout.rowCount()-1, self.layout.columnCount() - 2)

    def setup_ui_design(self):
        self.settings.beginGroup("main_window")
        self.main_window_size[0].setText('Window size')
        for i in range(2):
            self.main_window_size[i + 1].setText(str(self.settings.value('size')[i]))
        self.apply_btn.setText("Apply")
        self.default_btn.setText("Set to default")
        self.settings.endGroup()

    def save_properties(self):
        self.settings.beginGroup("main_window")
        self.read_from_edit_group(self.main_window_size, 'size', int)
        self.settings.endGroup()

class Graph_properties(Abstract_properties):
    def __init__(self):
        super(Graph_properties, self).__init__()

class Map_properties(Abstract_properties):
    def __init__(self):
        super(Map_properties, self).__init__()

class Model_properties(Abstract_properties):
    def __init__(self):
        super(Model_properties, self).__init__()

class Settings_window(QtWidgets.QMainWindow):
    change_settings = QtCore.pyqtSignal()

    def __init__(self):
        super(Settings_window, self).__init__()
        self.settings = settings_control.init_settings()
        
        self.setWindowIcon(QtGui.QIcon(APP_ICON_PATH))

        self.setup_ui()
        self.setup_ui_design()

    def setup_ui(self):
        self.toolbar = self.addToolBar('Settings')
        self.toolbar.setMovable(False)
        self.toolbar.setIconSize(QtCore.QSize(50, 50))
        self.toolbar.setContextMenuPolicy(QtCore.Qt.PreventContextMenu)

        self.common_properties_btn = self.toolbar.addAction(QtGui.QIcon(WINDOW_ICON_PATH), 'Common')
        self.common_properties_btn.triggered.connect(self.show_common_properties)
        self.graph_properties_btn = self.toolbar.addAction(QtGui.QIcon(GRAPH_ICON_PATH), 'Common')
        self.graph_properties_btn.triggered.connect(self.show_graph_properties)
        self.map_properties_btn = self.toolbar.addAction(QtGui.QIcon(MAP_ICON_PATH), 'Common')
        self.map_properties_btn.triggered.connect(self.show_map_properties)
        self.model_properties_btn = self.toolbar.addAction(QtGui.QIcon(MODEL_ICON_PATH), 'Common')
        self.model_properties_btn.triggered.connect(self.show_model_properties)

        self.show_common_properties()

    def setup_ui_design(self):
        self.central_widget.setup_ui_design()

    def set_settings_to_default(self):
        settings_control.set_to_default(self.settings)
        self.settings = settings_control.init_settings()
        self.setup_ui_design()
        self.change_settings.emit()
        
    def show_common_properties(self):
        self.central_widget = Common_properties()
        self.setup_central_widget()

    def show_graph_properties(self):
        self.central_widget = Graph_properties()
        self.setup_central_widget()

    def show_map_properties(self):
        self.central_widget = Map_properties()
        self.setup_central_widget()

    def show_model_properties(self):
        self.central_widget = Model_properties()
        self.setup_central_widget()

    def setup_central_widget(self):
        self.central_widget.default_btn.clicked.connect(self.set_settings_to_default)
        self.central_widget.apply_btn.clicked.connect(self.set_properties)
        self.setCentralWidget(self.central_widget)

    def set_properties(self):
        self.central_widget.save_properties()
        self.change_settings.emit()

    def removeToolBar(self, toolbar):
        super(Settings_window, self).removeToolBar()
    def show(self):
        self.hide()
        super(Settings_window, self).show()

class Main_window(QtWidgets.QMainWindow):
    def __init__(self):
        super(Main_window, self).__init__()
        self.settings = settings_control.init_settings()

        if not settings_control.settings_test(self.settings):
            settings_control.set_to_default(self.settings)
            self.settings = settings_control.init_settings()
            self.settings.setValue('main_window/size', [1000, 800])

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

        self.settings_window = Settings_window()

        self.menu_bar = self.menuBar()
        self.menu_file = self.menu_bar.addMenu("&File")
        self.action_settings = self.menu_file.addAction("&Settings")
        self.action_settings.setShortcut('Ctrl+S')
        self.action_settings.triggered.connect(self.settings_window.show)
        self.action_exit = self.menu_file.addAction("&Exit")
        self.action_exit.setShortcut('Ctrl+Q')
        self.action_exit.triggered.connect(QtWidgets.qApp.quit)

        self.settings_window.change_settings.connect(self.setup_ui_design)

    def setup_ui_design(self):
        self.resize(int(self.settings.value('main_window/size')[0]), int(self.settings.value('main_window/size')[1]))
        self.setWindowTitle("StrelA MS")
        self.menu_file.setTitle("&File")
        self.action_settings.setText("&Settings")
        self.action_settings.setStatusTip("Settings")
        self.action_exit.setText("&Exit")
        self.action_exit.setStatusTip("Exit")

        self.settings_window.setup_ui_design()

    def move_to_center(self):
        frame = self.frameGeometry()
        frame.moveCenter(QtWidgets.QDesktopWidget().availableGeometry().center())
        self.move(frame.topLeft())

    def closeEvent(self, evnt):
        self.settings_window.close()
        super(Main_window, self).closeEvent(evnt)