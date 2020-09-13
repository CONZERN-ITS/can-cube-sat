import pyqtgraph as PyQtGraph
import numpy as NumPy

from source import settings_control

import math


class GraphWidget(PyQtGraph.GraphicsLayoutWidget):
    class Curve():
        def __init__(self, plot, pen='w', max_arr_len=None):
            self.plot = plot
            self.arr = None
            self.curve = None
            self.set_pen(pen)
            self.max_arr_len = max_arr_len
            self.time_shift = 0

        def set_pen(self, pen):
            self.pen = pen

        def show_data(self, data):
            if self.arr is None:
                self.arr = data
                if self.curve is None:
                    self.curve = self.plot.plot(self.arr, pen=self.pen)
                    return
            else:
                self.arr = NumPy.vstack((self.arr, data))

            if (self.max_arr_len is not None) and (len(self.arr) > self.max_arr_len):
                self.arr = self.arr[len(self.arr) - self.max_arr_len:-1]

            self.curve.setData(self.arr[NumPy.argsort(self.arr[:,0])])

        def clear(self):
            self.arr = None

    def __init__(self):
        super(GraphWidget, self).__init__()
        self.settings = settings_control.init_settings()

        self.setup_ui()
        self.setup_ui_design()
        self.update_current_values()

    def setup_ui(self):
        self.plot_tuple = []

    def setup_graph(self, pos, name):
        axis_x = PyQtGraph.AxisItem(orientation='left')
        axis_x.setLabel(name)
        axis_y = PyQtGraph.AxisItem(orientation='bottom')
        axis_y.setLabel("Time")
        return self.addPlot(int(pos[0]), int(pos[1]), int(pos[2]), int(pos[3]), axisItems={'left': axis_x, 'bottom': axis_y})

    def setup_curves(self, plot, count, color, max_data_length):
        if max_data_length == 0:
            max_data_length = None
        curves = []
        for i in range(count):
            curves.append(GraphWidget.Curve(plot, max_arr_len=max_data_length))
            if i < len(color):
                curves[i].set_pen(color[i])
        return tuple(curves)

    def setup_ui_design(self):
        for plot in self.plot_tuple:
            try:
                self.removeItem(plot[0])
            except Exception as e:
                print(e)
        self.plot_tuple = []

        self.settings.beginGroup("CentralWidget/GraphWidget")
        self.settings.beginGroup("Graph")
        for group in self.settings.childGroups():
            self.settings.beginGroup(group)
            if int(self.settings.value("is_on")):
                graph = self.setup_graph(self.settings.value("position"), group)
                curves = self.setup_curves(graph,
                                           int(self.settings.value("packet_name")[-1]),
                                           self.settings.value("colour")[0:-1],
                                           int(self.settings.value("max_data_length")))
                self.plot_tuple.append((graph, curves, self.settings.value("packet_name")))
            self.settings.endGroup()
        self.plot_tuple = tuple(self.plot_tuple)
        self.settings.endGroup()
        self.settings.endGroup()

    def update_current_values (self):
        self.time_shift = float(self.settings.value("CurrentValues/time_shift"))

    def new_data_reaction(self, data):
        for plot in self.plot_tuple:
            for i in range(1, len(plot[2]), 2):
                pack = data.get(plot[2][i], None)
                if pack is not None:
                    data_len = int(plot[2][i + 1]) - int(plot[2][i - 1])
                    if ((pack.shape[1] - 1) >= data_len):
                        for j in range(data_len):
                            curve = pack[:,[0, j + 1]]
                            curve = curve[~NumPy.isnan(curve).any(axis=1)]
                            curve = curve[~NumPy.isinf(curve).any(axis=1)]
                            curve[:, 0] -= self.time_shift
                            plot[1][int(plot[2][i - 1]) + j].show_data(curve)

    def clear_data(self):
        for plot in self.plot_tuple:
            for curve in plot[1]:
                curve.clear()
