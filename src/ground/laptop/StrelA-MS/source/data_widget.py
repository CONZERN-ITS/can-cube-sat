from PyQt5 import QtWidgets, QtGui, QtCore

from source import settings_control


class DataWidget(QtWidgets.QTreeWidget):
    class TreeItem(QtWidgets.QTreeWidgetItem):
        packet_name = []
        data_range = []
        colors = [QtGui.QColor('#0000FF'), QtGui.QColor('#FF0000')]
        background_color = QtGui.QColor('#000000')
        timeout_color = QtGui.QColor('#FF00FF')
        timer = None

        def set_background_color(self, background_color):
            self.background_color = background_color

        def get_packet_name(self):
            return self.packet_name

        def set_packet_name(self, packet_name):
            self.packet_name = packet_name

        def setup_fields(self, names, group_name=None):
            if group_name is not None:
                self.setText(0, group_name)
                self.setText(1, '')
            for name in names:
                item = QtWidgets.QTreeWidgetItem(self)
                item.setText(0, name)
            self.set_data_range([])

        def setup_background(self, color):
            for i in range(self.columnCount()):
                self.setBackground(i, color)

        def set_data(self, data):
            self.setup_background(self.background_color)
            if len(data) <= self.childCount():
                for i in range(len(data)):
                    self.child(i).setText(1, str(data[i]))
                    DataWidget.TreeItem.setup_background(self.child(i), self.background_color)
                    if self.data_range[i] is not None:
                        if data[i] < self.data_range[i][0]:
                            DataWidget.TreeItem.setup_background(self.child(i), self.colors[0])
                        elif data[i] > self.data_range[i][1]:
                            DataWidget.TreeItem.setup_background(self.child(i), self.colors[1])
                    if self.timer is not None:
                        self.timer.start()

        def set_value(self, value):
            self.setup_background(self.background_color)
            self.setText(1, str(value))
            if self.timer is not None:
                self.timer.start()

        def get_value(self):
            return self.text(1)

        def setup_timeout(self, color, time_limit):
            self.timeout_color = color
            self.timer = QtCore.QTimer()
            self.timer.setSingleShot(True)
            self.timer.setInterval(int(time_limit * 1000))
            self.timer.timeout.connect(self.timeout_action)

        def timeout_action(self):
            self.setup_background(self.timeout_color)
            for i in range(self.childCount()):
                DataWidget.TreeItem.setup_background(self.child(i), self.timeout_color)

        def set_data_range(self, data_range):
            self.data_range = data_range
            if len(data_range) < self.childCount():
                for i in range(self.childCount() - len(data_range)):
                    self.data_range.append(None)

        def set_colors(self, colors):
            self.colors = colors

    def __init__(self):
        super(DataWidget, self).__init__()
        self.settings = settings_control.init_settings()

        self.setup_ui()
        self.setup_ui_design()
        self.update_current_values()

    def setup_ui(self):
        self.setColumnCount(2)

    def setup_ui_design(self):
        self.clear()
        self.settings.beginGroup("CentralWidget/DataWidget")
        self.background_color = QtGui.QColor(self.settings.value("background_color"))
        self.headerItem().setText(0, '')
        self.headerItem().setText(1, '')
        palette = QtGui.QPalette()
        palette.setColor(QtGui.QPalette.Base, self.background_color)
        self.setPalette(palette)
        self.setStyleSheet("QHeaderView::section { background-color:" + self.settings.value("background_color") + '}')
        self.colors = tuple([QtGui.QColor(self.settings.value("colors")[i]) for i in range(3)])

        self.table_dict = {}
        self.time_tuple = []

        self.time_table = 0
        self.packet_table = 0

        if int(self.settings.value("Time_table/is_on")):
            self.time_table = 1
            self.settings.beginGroup("Time_table")
            top_item = QtWidgets.QTreeWidgetItem()
            top_item.setText(0, 'Systems time')
            for group in self.settings.childGroups():
                if int(self.settings.value(group + "/is_on")):
                    self.settings.beginGroup(group)
                    item = DataWidget.TreeItem(top_item)
                    item.set_background_color(self.background_color)
                    item.setup_fields([], self.settings.value("name"))
                    item.set_packet_name(self.settings.value("packet_name")[:-1])
                    item.setup_timeout(self.colors[2], float(self.settings.value("time_limit")))
                    self.settings.endGroup()
            self.addTopLevelItem(top_item)
            self.settings.endGroup()

        if int(self.settings.value("Packet_table/is_on")):
            self.packet_table = 1
            self.settings.beginGroup("Packet_table")
            top_item = QtWidgets.QTreeWidgetItem()
            top_item.setText(0, 'Packets count')
            for group in self.settings.childGroups():
                if int(self.settings.value(group + "/is_on")):
                    self.settings.beginGroup(group)
                    item = DataWidget.TreeItem(top_item)
                    item.set_background_color(self.background_color)
                    item.setup_fields([], self.settings.value("name"))
                    item.set_packet_name([self.settings.value("packet_name")])
                    item.set_value(0)
                    self.settings.endGroup()
            self.addTopLevelItem(top_item)
            self.settings.endGroup()

        if int(self.settings.value("Data_table/is_on")):
            self.settings.beginGroup("Data_table")
            top_item = QtWidgets.QTreeWidgetItem()
            top_item.setText(0, 'Data')
            for group in self.settings.childGroups():
                if int(self.settings.value(group + "/is_on")):
                    self.settings.beginGroup(group)
                    item = DataWidget.TreeItem(top_item)
                    item.set_background_color(self.background_color)
                    item.setup_fields(self.settings.value("name")[:-1], group)
                    item.set_packet_name([self.settings.value("packet_name")])
                    item.setup_timeout(self.colors[2], float(self.settings.value("time_limit")))
                    data_range = []
                    if self.settings.value("range") != 'nan':
                        for i in range(0, len(self.settings.value("range")), 2):
                            data_range.append([float(self.settings.value("range")[i]), float(self.settings.value("range")[i + 1])])
                    item.set_data_range(data_range)
                    item.set_colors(self.colors[:2])
                    self.settings.endGroup()
                self.addTopLevelItem(top_item)
            self.settings.endGroup()
        self.settings.endGroup()

        for i in range(self.topLevelItemCount()):
            self.expandItem(self.topLevelItem(i))
            for j in range(self.topLevelItem(i).childCount()):
                self.expandItem(self.topLevelItem(i).child(j))

    def update_current_values (self):
        pass

    def new_data_reaction(self, data):
        if self.time_table:
            for i in range(self.topLevelItem(0).childCount()):
                item = self.topLevelItem(0).child(i)
                time = None
                for packet_name in item.get_packet_name():
                    pack = data.get(packet_name, None)
                    if pack is not None:
                        if (time is None) or (time < pack[-1][0]):
                            time = pack[-1][0]
                if time is not None:
                    item.set_value(time)

        if self.packet_table:
            for i in range(self.topLevelItem(self.time_table).childCount()):
                item = self.topLevelItem(self.time_table).child(i)
                pack = data.get(item.get_packet_name()[0], None)
                if pack is not None:
                    item.set_value(int(item.get_value()) + pack.shape[0])

        for j in range((self.time_table + self.packet_table), self.topLevelItemCount()):
            for i in range(self.topLevelItem(j).childCount()):
                item = self.topLevelItem(j).child(i)
                pack = data.get(item.get_packet_name()[0], None)
                if pack is not None:
                    item.set_data(pack[-1][1:])

    def clear_data(self):
        self.setup_ui_design()
