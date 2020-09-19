from PyQt5 import QtWidgets, QtGui, QtCore


class CommandLogPanel(QtWidgets.QTextEdit):
    def __init__(self):
        super(CommandLogPanel, self).__init__()

        self.setup_ui()
        self.setup_ui_design()

    def setup_ui(self):
        self.setReadOnly(True)
        self.setText('Last antenna system manual contrl commands\n')

    def setup_ui_design(self):
        pass

    def add_data(self, str):
        self.append('--> ' + str + '\n')

