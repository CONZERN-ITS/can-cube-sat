import pyqtgraph as PyQtGraph
import numpy as NumPy

from source import settings_control

import math


class GraphWidget(PyQtGraph.GraphicsLayoutWidget):
    class Plot():
        def __init__(self, plot):
            self.plot = plot
            self.curve_groups = {}
            self.max_data_length = None
            self.legend = None

        def set_max_data_len(self, length):
            self.max_data_length = length

        def setup_axis(self, name_x, name_y):
            axis_x = PyQtGraph.AxisItem(orientation='bottom')
            axis_x.setLabel(name_x)
            axis_y = PyQtGraph.AxisItem(orientation='left')
            axis_y.setLabel(name_y)
            self.plot.setAxisItems({'bottom': axis_x, 'left': axis_y})

        def add_curve_group(self, name, curves_count, pen_color=[], curves_names=[]):
            curves = []
            for i in range(curves_count):
                curves.append(self.plot.plot(NumPy.array([[0, 0]])))
            if self.legend is not None:
                for i in range(len(curves_names)):
                    self.legend.addItem(curves[i], curves_names[i]) 
            curve_group = GraphWidget.CurveGroup(curves)
            curve_group.set_max_data_len(self.max_data_length)
            curve_group.setup_pen(pen_color)

            self.curve_groups.update([[name, curve_group]])

        def setup_plot_legend(self):
            self.legend = self.plot.addLegend()

        def get_curve_groups(self):
            return self.curve_groups

        def clear(self):
            for group in self.curve_groups.items():
                group[1].clear()

    class CurveGroup():
        def __init__(self, curves):
            self.curves = []
            for curve in curves:
                self.curves.append(GraphWidget.Curve(curve))
            self.arr = None

        def setup_pen(self, color=[]):
            for i in range(len(color)):
                self.curves[i].setup_pen(color[i])

        def get_curves(self):
            return self.curves

        def set_max_data_len(self, length):
            for curve in self.curves:
                curve.set_max_data_len(length)

        def show_data(self, data, x_shift=0):
            if ((data.shape[1] - 1) >= len(self.curves)):
                for i in range(len(self.curves)):
                    line = data[:,[0, i + 1]]
                    line = line[~NumPy.isnan(line).any(axis=1)]
                    line = line[~NumPy.isinf(line).any(axis=1)]
                    line[:, 0] -= x_shift
                    self.curves[i].show_data(line)

        def clear(self):
            for curve in self.curves:
                curve.clear()

    class Curve():
        def __init__(self, curve):
            self.curve = curve
            self.arr = None

        def set_max_data_len(self, length):
            if length <= 0:
                self.max_data_length = None
            else:
                self.max_data_length = length

        def setup_pen(self, color):
            self.curve.setPen(color=color)

        def show_data(self, data):
            if self.arr is None:
                self.arr = data
            else:
                self.arr = NumPy.vstack((self.arr, data))

            if (self.max_data_length is not None) and (len(self.arr) > self.max_data_length):
                self.arr = self.arr[len(self.arr) - self.max_data_length:-1]

            self.curve.setData(self.arr[NumPy.argsort(self.arr[:,0])])

        def clear(self):
            self.arr = None
            self.curve.setData(NumPy.array([[0, 0]]))

    def __init__(self):
        super(GraphWidget, self).__init__()
        self.settings = settings_control.init_settings()

        self.setup_ui()
        self.setup_ui_design()
        self.update_current_values()

    def setup_ui(self):
        self.plots = []

    def get_param(self, beg_num, count, param_list):
        if len(param_list) >= beg_num:
            param = param_list[beg_num:]
            if len(param) > count:
                param = param[:count]
        else:
            param = []
        return param

    def setup_ui_design(self):
        for plot in self.plots:
            self.removeItem(plot.plot)
        self.plots = []

        self.settings.beginGroup("CentralWidget/GraphWidget")
        units = self.settings.value("time_units")
        self.settings.beginGroup("Graph")
        for group in self.settings.childGroups():
            self.settings.beginGroup(group)
            if int(self.settings.value("is_on")):
                self.plots.append(GraphWidget.Plot(self.addPlot(*[int(num) for num in self.settings.value("position")])))
                self.plots[-1].setup_axis('Time ' + units, group + ' ' + self.settings.value("units"))
                self.plots[-1].set_max_data_len(int(self.settings.value("max_data_length")))
                if int(self.settings.value("Legend/is_on")):
                    self.plots[-1].setup_plot_legend()
                for i in range(1, len(self.settings.value("packet_name")), 2):
                    packet_name = self.settings.value("packet_name")
                    curves_count = int(packet_name[i + 1]) - int(packet_name[i - 1])
                    colors = self.get_param(int(packet_name[i - 1]), curves_count, self.settings.value("colour")[0:-1])
                    legend = self.get_param(int(packet_name[i - 1]), curves_count, self.settings.value("Legend/name")[0:-1])
                    self.plots[-1].add_curve_group(packet_name[i], curves_count, pen_color=colors, curves_names=legend)
            self.settings.endGroup()
        self.plots = tuple(self.plots)
        self.settings.endGroup()
        self.settings.endGroup()

    def update_current_values (self):
        self.time_shift = float(self.settings.value("CurrentValues/time_shift"))

    def new_data_reaction(self, data):
        for plot in self.plots:
            for curve_group in plot.get_curve_groups().items():
                pack = data.get(curve_group[0], None)
                if pack is not None:
                    curve_group[1].show_data(pack, x_shift=self.time_shift)

    def clear_data(self):
        for plot in self.plots:
            plot.clear()
