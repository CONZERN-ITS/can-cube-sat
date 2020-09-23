from PyQt5 import QtWidgets, QtGui, QtCore


class PositionControlPanel(QtWidgets.QWidget):
    top_btn_clicked = QtCore.pyqtSignal(int)
    bottom_btn_clicked = QtCore.pyqtSignal(int)
    left_btn_clicked = QtCore.pyqtSignal(int)
    right_btn_clicked = QtCore.pyqtSignal(int)
    central_btn_clicked = QtCore.pyqtSignal()

    text = ''
    def __init__(self):
        super(PositionControlPanel, self).__init__()
        self.setMinimumSize(300, 300)
        palette = QtGui.QPalette()
        palette.setColor(QtGui.QPalette.Background, QtGui.QColor(225, 225, 225, 255))
        self.setPalette(palette)
        self.setAutoFillBackground(True) 

        self.setMouseTracking(True)

        self.passiv_color = QtGui.QColor(225, 225, 0, 255)
        self.activ_zone_3_color = QtGui.QColor(50, 0, 200, 255)
        self.activ_zone_2_color = QtGui.QColor(100, 0, 100, 255)
        self.activ_zone_1_color = QtGui.QColor(150, 0, 50, 255)
        self.activ_center_color = QtGui.QColor(200, 0, 0, 255)
        self.passiv_center_color = QtGui.QColor(225, 200, 0, 255)
        self.background_color = QtGui.QColor(10, 10, 10, 255)

        self.setup_ui()
        self.setup_ui_design()
        self.mouce_pos = QtCore.QPoint(0, 0)
        
        self.central_btn_state = True
        self.last_params = None

    def setup_ui(self):
        pass

    def setup_ui_design(self):
        pass

    def mouseMoveEvent(self, event):
        super(PositionControlPanel, self).mouseMoveEvent(event)
        self.mouce_pos = event.pos()
        self.update()

    def mousePressEvent(self, event):
        super(PositionControlPanel, self).mousePressEvent(event)
        self.param = self.count_region(self.rect().center(), QtCore.QPoint(event.pos()), self.count_workspace(self))

    def mouseReleaseEvent(self, event):
        super(PositionControlPanel, self).mouseReleaseEvent(event)
        if self.param is not None:
            param = self.count_region(self.rect().center(), QtCore.QPoint(event.pos()), self.count_workspace(self))
            if param is not None:
                for i in range(2):
                    if param[i] != self.param[i]:
                        self.param = None
                        return
                if param[1] == 0:
                    self.central_btn_clicked.emit()
                elif param[0] == 0:
                    self.right_btn_clicked.emit(param[1])
                elif param[0] == 1:
                    self.top_btn_clicked.emit(param[1])
                elif param[0] == 2:
                    self.left_btn_clicked.emit(param[1])
                elif param[0] == 3:
                    self.bottom_btn_clicked.emit(param[1])

    def control_action(self, param):
        if param[1] == 0:
            self.soft_control = not self.soft_control

    def paintEvent(self, event):
        super(PositionControlPanel, self).paintEvent(event)
        painter = QtGui.QPainter(self)
        painter.setRenderHint(QtGui.QPainter.Antialiasing)

        self.draw_background(painter, event.rect().center(), event)

    def count_workspace(self, event):
        diam = min(event.rect().height(), event.rect().width())
        return diam / 2 - 10

    def setup_pen(self, painter, brush=None, width=None, style=None, cap_style=None, join=None):
        pen = painter.pen()
        if brush is not None:
            pen.setBrush(brush)
        if width is not None:
            pen.setWidth(width)
        if style is not None:
            pen.setStyle(style)
        if cap_style is not None:
            pen.setCapStyle(cap_style)
        if join is not None:
            pen.setJoinStyle(join)
        painter.setPen(pen)

    def setup_brush(self, painter, color=None, texture=None, style=None):
        brush = painter.brush()
        if color is not None:
            brush.setColor(color)
        if texture is not None:
            brush.setTexture(texture)
        if style is not None:
            brush.setStyle(style)
        painter.setBrush(brush)

    def setText(self, text):
        self.text = text
        self.update()

    def draw_background(self, painter, center, event):
        rad = self.count_workspace(event)
        painter.setPen(QtCore.Qt.NoPen)
        painter.setBrush(self.passiv_color)

        param = self.count_region(center, QtCore.QPoint(self.mouce_pos), rad)

        if param is not None:
            painter.save()
            painter.translate(center)
            painter.rotate(-45 + param[0] * -90)
            if param[1] == 3:
                self.setup_brush(painter, color=self.activ_zone_3_color)
                self.draw_petal(painter, QtCore.QPoint(0, 0), rad * 2)
            if param[1] >= 2:
                self.setup_brush(painter, color=self.activ_zone_2_color)
                self.draw_petal(painter, QtCore.QPoint(0, 0), rad * 1.5)
            if param[1] >= 1:
                self.setup_brush(painter, color=self.activ_zone_1_color)
                self.draw_petal(painter, QtCore.QPoint(0, 0), rad)
            painter.restore()

        painter.setPen(self.background_color)
        self.setup_pen(painter, width=4, cap_style=QtCore.Qt.RoundCap)
        self.setup_brush(painter, style=QtCore.Qt.NoBrush)
        self.draw_circle(painter, center, rad * 2)

        self.setup_pen(painter, width=2)
        painter.save()
        painter.translate(center)
        painter.rotate(45)
        painter.drawLine(-rad, 0, rad, 0)
        painter.rotate(90)
        painter.drawLine(-rad, 0, rad, 0)
        painter.restore()

        self.setup_pen(painter, width=4)
        self.setup_brush(painter, style=QtCore.Qt.SolidPattern)
        if self.central_btn_state:
            self.setup_brush(painter, color=self.activ_center_color)
        else:
            self.setup_brush(painter, color=self.passiv_center_color)

        self.draw_circle(painter, center, rad * 0.5)

        rect = self.setup_rect(center, QtCore.QSize(rad * 0.47, rad * 0.47))
        font = QtGui.QFont()
        font.setWeight(10)
        width = QtGui.QFontMetrics(font).width(self.text)
        if self.text != '':
            font.setPointSize(int(rect.width() / width * font.pointSize()))
        painter.setFont(font)
        painter.drawText(rect, QtCore.Qt.AlignHCenter | QtCore.Qt.AlignVCenter, self.text)

    def count_region(self, center, abs_pos, rad):
        pos = abs_pos - center
        sqr_norm = pos.x() ** 2 + pos.y() ** 2
        if (sqr_norm < rad ** 2):
            if (abs(pos.x()) < 0.1) or (abs(pos.y() / pos.x()) < 1):
                if pos.x() < 0:
                    reg = 2
                else:
                    reg = 0
            else:
                if pos.y() < 0:
                    reg = 1
                else:
                    reg = 3
            if sqr_norm > ((rad * 0.75) ** 2):
                rad = 3
            elif sqr_norm > ((rad * 0.5) ** 2):
                rad = 2
            elif sqr_norm > ((rad * 0.25) ** 2):
                rad = 1
            else:
                rad = 0
            return (reg, rad)
        else:
            return None

    def setup_rect(self, center, size):
        rect = QtCore.QRect(center, size)
        rect.moveCenter(center)
        return rect

    def draw_petal(self, painter, center, size):
        rect = self.setup_rect(center, QtCore.QSize(size, size))
        painter.drawPie(rect, 0, -1440)

    def draw_circle(self, painter, center, diam):
        rect = self.setup_rect(center, QtCore.QSize(diam, diam))
        painter.drawEllipse(rect)


class PositionControlWidget(QtWidgets.QWidget):
    mode_changed = QtCore.pyqtSignal(bool)
    def __init__(self):
        super(PositionControlWidget, self).__init__()

        self.setup_ui()
        self.setup_ui_design()

    def setup_frame(self, layout):
        frame = QtWidgets.QFrame()
        frame.setFrameStyle(QtWidgets.QFrame.Panel|QtWidgets.QFrame.Raised)
        layout.addWidget(frame)
        return frame

    def setup_h_line(self, layout):
        line = QtWidgets.QFrame()
        line.setFrameStyle(QtWidgets.QFrame.HLine|QtWidgets.QFrame.Raised)
        line.setLineWidth(2)
        layout.addWidget(line)
        return line

    def setup_label(self, layout, text, alignment=QtCore.Qt.AlignLeft):
        label = QtWidgets.QLabel(text)
        label.setAlignment(alignment)
        layout.addWidget(label)
        return label

    def setup_ui(self):
        self.layout = QtWidgets.QVBoxLayout(self)

        self.pos_control_panel = PositionControlPanel()
        self.pos_control_panel.setText('Park')
        self.layout.addWidget(self.pos_control_panel)
        self.layout.setStretch(0, 3)

        frame = self.setup_frame(self.layout)
        frame_layout = QtWidgets.QHBoxLayout(frame)

        self.mode_true_btn = QtWidgets.QPushButton('Soft manual control')
        self.mode_true_btn.setCheckable(True)
        frame_layout.addWidget(self.mode_true_btn)

        self.mode_false_btn = QtWidgets.QPushButton('Hard manual control')
        self.mode_false_btn.setCheckable(True)
        frame_layout.addWidget(self.mode_false_btn)

        self.mode_true_btn.toggle()

        self.mode_true_btn.toggled.connect(self.mode_true_btn_action)
        self.mode_false_btn.toggled.connect(self.mode_false_btn_action)
        
        frame = self.setup_frame(self.layout)
        frame_layout = QtWidgets.QGridLayout(frame)
        text_list = ['Azimuth:', 'Elevation:']
        self.pos_control_line_edit = []
        for i in range(2):
            label = QtWidgets.QLabel(text_list[i])
            frame_layout.addWidget(label, 0, i)
            self.pos_control_line_edit.append(QtWidgets.QLineEdit())
            self.pos_control_line_edit[-1].setText('0')
            self.pos_control_line_edit[-1].setValidator(QtGui.QDoubleValidator())
            frame_layout.addWidget(self.pos_control_line_edit[-1], 1, i)
        self.control_btn = QtWidgets.QPushButton('Send')
        frame_layout.addWidget(self.control_btn, 2, 0, 1, 2)

    def setup_ui_design(self):
        pass

    def mode_true_btn_action(self, checked):
        if self.mode_false_btn.isChecked() == checked:
            self.mode_false_btn.toggle()

    def mode_false_btn_action(self, checked):
        if self.mode_true_btn.isChecked() == checked:
            self.mode_true_btn.toggle()

