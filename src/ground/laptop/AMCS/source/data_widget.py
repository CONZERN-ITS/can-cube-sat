from PyQt5 import QtWidgets, QtGui, QtCore

import math
import time


class PositionPanel(QtWidgets.QWidget):
    def __init__(self):
        super(PositionPanel, self).__init__()
        self.setMinimumSize(150, 150)
        palette = QtGui.QPalette()
        palette.setColor(QtGui.QPalette.Background, QtGui.QColor(225, 225, 225, 255))
        self.setPalette(palette)
        self.setAutoFillBackground(True) 

        self.target_color = QtGui.QColor(50, 50, 200, 255)
        self.antenna_color = QtGui.QColor(200, 50, 50, 255)
        self.background_color = QtGui.QColor(10, 10, 10, 255)

        self.setup_ui()
        self.setup_ui_design()

    def setup_ui(self):
        pass

    def setup_ui_design(self):
        self.set_antenna_pos()
        self.set_target_pos()

    def set_antenna_pos(self, azimuth=None, elevation=None):
        self.antenna_elevation = elevation
        self.antenna_azimuth = azimuth 
        self.update()

    def set_target_pos(self, azimuth=None, elevation=None):
        self.target_elevation = elevation
        self.target_azimuth = azimuth 
        self.update()

    def paintEvent(self, event):
        super(PositionPanel, self).paintEvent(event)
        painter = QtGui.QPainter(self)
        painter.setRenderHint(QtGui.QPainter.Antialiasing)
        painter.translate(event.rect().center())
        painter.rotate(270)
        diam = min(event.rect().height(), event.rect().width())

        self.draw_background(painter, event)
        self.draw_antenna(painter, event)
        self.draw_target(painter, event)

    def draw_antenna(self, painter, event):
        if (self.antenna_elevation is not None) and (self.antenna_azimuth is not None):
            self.setup_pen(painter, brush=self.antenna_color, width=5)
            painter.setBrush(QtCore.Qt.NoBrush)
            ws_rad = self.count_workspace(event)
            rad = math.cos(math.radians(self.antenna_elevation)) * ws_rad
            painter.save()
            painter.rotate(self.antenna_azimuth)
            self.draw_circle(painter, QtCore.QPoint(rad, 0), ws_rad * 0.2)
            painter.setBrush(self.antenna_color)
            self.draw_circle(painter, QtCore.QPoint(rad, 0), ws_rad * 0.05)
            painter.restore()

    def draw_target(self, painter, event):
        if (self.target_elevation is not None) and (self.target_azimuth is not None):
            self.setup_pen(painter, brush=self.target_color, width=5, cap_style=QtCore.Qt.RoundCap)
            painter.setBrush(QtCore.Qt.NoBrush)
            ws_rad = self.count_workspace(event)
            rad = math.cos(math.radians(self.target_elevation)) * ws_rad
            painter.save()
            painter.rotate(self.target_azimuth)
            self.draw_circle(painter, QtCore.QPoint(rad, 0), ws_rad * 0.2)
            self.draw_cross(painter, QtCore.QPoint(rad, 0), ws_rad * 0.2)
            painter.restore()

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

    def draw_background(self, painter, event):
        self.setup_pen(painter, brush=self.background_color, width=7, cap_style=QtCore.Qt.RoundCap)
        self.setup_brush(painter, style=QtCore.Qt.NoBrush)
        rad = self.count_workspace(event)
        center = event.rect().center()
        self.draw_circle(painter, QtCore.QPoint(0, 0), rad * 2)
        color = QtGui.QColor(self.background_color)
        color.setAlpha(color.alpha() * 0.60)
        self.setup_pen(painter, brush=color, width=5)
        self.draw_circle(painter, QtCore.QPoint(0, 0), rad * 0.1)
        painter.save()
        for i in range(60):
            if i % 5 == 0:
                self.setup_pen(painter, width=5)
                painter.drawLine(rad, 0, rad * 0.80, 0)
            else:
                self.setup_pen(painter, width=3)
                painter.drawLine(rad, 0, rad * 0.90, 0)
            painter.rotate(6.0)
        painter.restore()

    def draw_cross(self, painter, center, diam):
        rad = diam / 2
        painter.save()
        painter.translate(center)
        painter.rotate(90)
        painter.drawLine(-rad, 0, rad, 0)
        painter.drawLine(0, -rad, 0, rad)
        painter.restore()

    def draw_circle(self, painter, center, diam):
        rect = QtCore.QRect(center, QtCore.QSize(diam, diam))
        rect.moveCenter(center)
        painter.drawEllipse(rect)


class PositionWidget(QtWidgets.QWidget):
    def __init__(self):
        super(PositionWidget, self).__init__()

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
        self.layout = QtWidgets.QHBoxLayout(self)

        visualization_layout = QtWidgets.QVBoxLayout()
        self.layout.addLayout(visualization_layout)
        self.layout.setStretch(0, 3)

        self.pos_panel = PositionPanel()
        visualization_layout.addWidget(self.pos_panel)
        visualization_layout.setStretch(0, 3)

        pos_layout = QtWidgets.QHBoxLayout()
        visualization_layout.addLayout(pos_layout)

        frame = self.setup_frame(pos_layout)
        frame_layout = QtWidgets.QVBoxLayout(frame)
        self.setup_label(frame_layout, 'Target', QtCore.Qt.AlignHCenter)
        self.setup_h_line(frame_layout)
        text_list = ['Azimuth:', 'Elevation:', 'Time:']
        self.target_param_lbl = []
        for i in range(3):
            layout = QtWidgets.QHBoxLayout()
            frame_layout.addLayout(layout)
            self.setup_label(layout, text_list[i])
            self.target_param_lbl.append(self.setup_label(layout, 'None'))

        frame = self.setup_frame(pos_layout)
        frame_layout = QtWidgets.QVBoxLayout(frame)
        self.setup_label(frame_layout, 'Antenna', QtCore.Qt.AlignHCenter)
        self.setup_h_line(frame_layout)
        text_list = ['Azimuth:', 'Elevation:', 'Time:']
        self.antenna_param_lbl = []
        for i in range(3):
            layout = QtWidgets.QHBoxLayout()
            frame_layout.addLayout(layout)
            self.setup_label(layout, text_list[i])
            self.antenna_param_lbl.append(self.setup_label(layout, 'None'))


        data_layout = QtWidgets.QVBoxLayout()
        self.layout.addLayout(data_layout)

        frame = self.setup_frame(data_layout)
        frame_layout = QtWidgets.QVBoxLayout(frame)
        self.setup_label(frame_layout, 'Topocentric to antenna system\ncoordinate system transition matrix', QtCore.Qt.AlignHCenter)
        self.top_to_ascs_lbl = []
        for i in range(3):
            layout = QtWidgets.QHBoxLayout()
            frame_layout.addLayout(layout)
            lbl_list = []
            for j in range(3):
                lbl_list.append(self.setup_label(layout,'None'))
            self.top_to_ascs_lbl.append(lbl_list)

        frame = self.setup_frame(data_layout)
        frame_layout = QtWidgets.QVBoxLayout(frame)
        text_list = ['Latitude:', 'Longitude:', 'Altitude:']
        self.lat_lon_alt_lbl = []
        for i in range(3):
            layout = QtWidgets.QHBoxLayout()
            frame_layout.addLayout(layout)
            self.setup_label(layout, text_list[i])
            self.lat_lon_alt_lbl.append(self.setup_label(layout, 'None'))

        frame = self.setup_frame(data_layout)
        frame_layout = QtWidgets.QHBoxLayout(frame)
        self.setup_label(frame_layout, 'Ecef:')
        self.ecef_lbl = []
        for j in range(3):
            self.ecef_lbl.append(self.setup_label(frame_layout,'None'))

    def setup_ui_design(self):
        pass

    def change_antenna_pos(self, data):
        for i in range(3):
            self.antenna_param_lbl[i].setText(str(data[i]))

    def change_target_pos(self, data):
        for i in range(3):
            self.target_param_lbl[i].setText(str(data[i]))

    def change_lat_lon_alt(self, data):
        for i in range(3):
            self.lat_lon_alt_lbl[i].setText(str(data[i]))

    def change_ecef(self, data):
        for i in range(3):
            self.ecef_lbl[i].setText(str(data[i]))

    def change_top_to_ascs_matrix(self, data):
        for i in range(3):
            for j in range (3):
                self.top_to_ascs_lbl[i][j].setText(str(data[i * 3 + j]))