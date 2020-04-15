import pyqtgraph as PyQtGraph
import numpy as NumPy

from source import settings_control


class GraphWidget(PyQtGraph.GraphicsLayoutWidget):
    class Curve():
        def __init__(self, plot, pen):
            self.plot = plot
            self.arr = None
            self.curve = None
            self.pen = pen

        def show_data(self, data):
            if self.arr is None:
                self.arr = NumPy.array(data)
                if self.curve is None:
                    self.curve = self.plot.plot(self.arr, pen=self.pen)
                    return
            else:
                self.arr = NumPy.vstack((self.arr, NumPy.array(data)))
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

    def setup_curves(self, plot, count, colour):
        curves = []
        for i in range(count):
            curves.append(GraphWidget.Curve(plot, colour[i]))
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
                                                                 self.settings.value("colour")))])
            self.settings.endGroup()
        self.settings.endGroup()
        self.settings.endGroup()

    def new_data_reaction(self, data):
        for plot in self.plot_dict.items():
            plot_buf = []
            self.settings.beginGroup("CentralWidget/GraphWidget/Graph/" + plot[0])
            for i in range(len(data)):
                if (self.settings.value("packet_name") == data[i][0]) and ((len(data[i]) - 2) >= len(plot[1])):
                    plot_buf.append(data[i])
            if len(plot_buf) > 0:
	            for i in range(len(plot[1])):
	                curve_buf = []
	                for pack in plot_buf:  
	                    curve_buf.append((pack[1], pack[i + 2]))
	                plot[1][i].show_data(curve_buf)
            self.settings.endGroup()

    def clear_data(self):
        for plot in self.plot_dict.items():
            for curve in plot[1]:
                curve.clear()
