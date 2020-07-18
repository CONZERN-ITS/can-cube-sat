from PyQt5 import QtWidgets, QtGui, QtCore

from source import settings_control


class DataWidget(QtWidgets.QTableWidget):
    class Timer(QtCore.QTimer):
        def __init__(self, table, row_num, table_len, color):
            super(DataWidget.Timer, self).__init__()
            self.timeout.connect(self.timeout_reaction)
            self.row_num = row_num
            self.table_len = table_len
            self.color = color
            self.table = table

        def timeout_reaction(self):
            for i in range(self.table_len):
                self.table.item(self.row_num + i, 0).setBackground(self.color)
                self.table.item(self.row_num + i, 1).setBackground(self.color)


    def __init__(self):
        super(DataWidget, self).__init__()
        self.settings = settings_control.init_settings()

        self.setup_ui()
        self.setup_ui_design()
        self.update_current_values()

    def setup_ui(self):
        self.setColumnCount(2)
        self.setEditTriggers(QtWidgets.QTreeView.NoEditTriggers)
        self.setFocusPolicy(QtCore.Qt.NoFocus)
        self.setSelectionMode(QtWidgets.QTreeView.NoSelection)
        self.verticalHeader().hide()
        self.horizontalHeader().hide()
        self.horizontalHeader().setSectionResizeMode(QtWidgets.QHeaderView.Stretch)

    def setup_ui_design(self):
        self.settings.beginGroup("CentralWidget/DataWidget")
        self.background_color = QtGui.QColor(self.settings.value("background_color"))
        palette = QtGui.QPalette()
        palette.setColor(QtGui.QPalette.Base, self.background_color)
        self.setPalette(palette)
        self.colors = tuple([QtGui.QColor(self.settings.value("colors")[i]) for i in range(3)])

        row_count = 0
        if int(self.settings.value("Time_table/is_on")):
            self.settings.beginGroup("Time_table")
            row_count = len(self.settings.childGroups())
            self.settings.endGroup()
        else:
            row_count = 0
            
        self.settings.beginGroup("Data_table")
        for group in self.settings.childGroups():
            row_count = row_count + len(self.settings.value(group +"/name")) - 1
        self.settings.endGroup()

        self.setRowCount(row_count)

        self.table_dict = {}
        self.time_tuple = []

        row_count = 0
        if int(self.settings.value("Time_table/is_on")):
            self.settings.beginGroup("Time_table")
            for group in self.settings.childGroups():
                self.settings.beginGroup(group)
                self.setItem(row_count, 0, QtWidgets.QTableWidgetItem(self.settings.value("name")))
                timer = DataWidget.Timer(self, row_count, 1, self.colors[2])
                timer.setSingleShot(True)
                timer.setInterval(int(float(self.settings.value("time_limit"))*1000))
                self.time_tuple.append(tuple([row_count, timer] + self.settings.value("packet_name")[:-1]))
                row_count += 1
                self.settings.endGroup()
            self.time_tuple = tuple(self.time_tuple)
            self.settings.endGroup()
            
        self.settings.beginGroup("Data_table")
        for group in self.settings.childGroups():
            self.settings.beginGroup(group)
            packet_len = len(self.settings.value("name")) - 1
            for i in range(packet_len):
                self.setItem(row_count + i, 0, QtWidgets.QTableWidgetItem(self.settings.value("name")[i]))
            timer = DataWidget.Timer(self, row_count, packet_len, self.colors[2])
            timer.setSingleShot(True)
            timer.setInterval(int(float(self.settings.value("time_limit"))*1000))
            self.table_dict.update([(self.settings.value("packet_name"), (row_count, 
                                                                          packet_len,
                                                                          tuple([float(num) for num in self.settings.value("range")]),
                                                                          timer))])
            row_count = row_count + packet_len
            self.settings.endGroup()
        self.settings.endGroup()
        self.settings.endGroup()

    def update_current_values (self):
        pass

    def new_data_reaction(self, data):
        for table in self.table_dict.items():
            pack = data.get(table[0], None)
            if pack is not None:
                pack = pack[-1]
                if ((pack.shape[0] - 1) >= table[1][1]):
                    for i in range(table[1][1]):
                        self.setItem(table[1][0] + i, 1, QtWidgets.QTableWidgetItem(str(pack[i + 1])))
                        if (pack[i + 1] < table[1][2][2 * i]):
                            self.item(table[1][0] + i, 0).setBackground(self.colors[0])
                            self.item(table[1][0] + i, 1).setBackground(self.colors[0])
                        elif (pack[i + 1] > table[1][2][2 * i + 1]):
                            self.item(table[1][0] + i, 0).setBackground(self.colors[1])
                            self.item(table[1][0] + i, 1).setBackground(self.colors[1])
                        else:
                            self.item(table[1][0] + i, 0).setBackground(self.background_color)
                            self.item(table[1][0] + i, 1).setBackground(self.background_color)
                    table[1][3].start()
        for row in self.time_tuple:
            time = None
            for packet in row[2:]:
                pack = data.get(packet, None)
                if pack is not None:
                    if (time is None) or (time < pack[-1][0]):
                        time = pack[-1][0]
            if time is not None:
                self.setItem(row[0], 1, QtWidgets.QTableWidgetItem(str(time)))
                self.item(row[0], 0).setBackground(self.background_color)
                self.item(row[0], 1).setBackground(self.background_color)
                row[1].start()

    def clear_data(self):
        self.clear()
        self.clearContents()
        self.setup_ui_design()