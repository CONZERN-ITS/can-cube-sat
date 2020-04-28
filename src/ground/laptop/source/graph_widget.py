import pyqtgraph as PyQtGraph
import numpy as NumPy

from source import settings_control


class GraphWidget(PyQtGraph.GraphicsLayoutWidget):
    class Curve():
        def __init__(self, plot, pen='w', max_arr_len=None):
            self.plot = plot
            self.arr = None
            self.curve = None
            self.set_pen(pen)
            self.max_arr_len = max_arr_len

        def set_pen(self, pen):
            self.pen = pen

        def show_data(self, data):
            if self.arr is None:
                self.arr = NumPy.array(data)
                if self.curve is None:
                    self.curve = self.plot.plot(self.arr, pen=self.pen)
                    return
            else:
                self.arr = NumPy.vstack((self.arr, NumPy.array(data)))

            if (self.max_arr_len is not None) and (len(self.arr) > self.max_arr_len):
                self.arr = self.arr[len(self.arr) - self.max_arr_len:-1]

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
        self.plot_dict.clear()
        for plot in self.plot_list:
            try:
                self.removeItem(plot)
            except Exception as e:
                print(e)
        self.setup_ui()

        self.settings.beginGroup("CentralWidget/GraphWidget")
        self.settings.beginGroup("Graph")
        for group in self.settings.childGroups():
            self.settings.beginGroup(group)
            if int(self.settings.value("is_on")):
                self.plot_list.append(self.setup_graph(self.settings.value("position"), group))
                self.plot_dict.update([(group, self.setup_curves(self.plot_list[-1],
                                                                 int(self.settings.value("count")),
                                                                 self.settings.value("colour")[0:-1],
                                                                 int(self.settings.value("max_data_length"))))])
            self.settings.endGroup()
        self.settings.endGroup()
        self.settings.endGroup()

    def new_data_reaction(self, data):
        for plot in self.plot_dict.items():
            plot_data_buf = [[] for i in range(len(plot[1]))]
            self.settings.beginGroup("CentralWidget/GraphWidget/Graph/" + plot[0])
            for pack in data:
                for i in range(1, len(self.settings.value('packet_name')), 2):
                    data_range = (int(self.settings.value("packet_name")[i - 1]), int(self.settings.value("packet_name")[i + 1]))
                    if ((self.settings.value("packet_name")[i] == pack[0]) and ((len(pack) - 2) >= (data_range[1] - data_range[0]))):
                        for i in range(*data_range):
                            plot_data_buf[i].append((pack[1], pack[i - data_range[0] + 2]))
                        break
            for i in range(len(plot_data_buf)):
                if len(plot_data_buf[i]) > 0:
    	            plot[1][i].show_data(plot_data_buf[i])
            self.settings.endGroup()

    def clear_data(self):
        for plot in self.plot_dict.items():
            for curve in plot[1]:
                curve.clear()
