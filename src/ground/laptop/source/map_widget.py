from source.map_source.open_street_map import *

from source import settings_control


class MapWidget(OpenStreetMap):
    def __init__(self):
        super(MapWidget, self).__init__()
        self.settings = settings_control.init_settings()

        self.key = None

        self.loadFinished.connect(self.setup_ui_design)

    def setup_ui_design(self):
        self.settings.beginGroup("CentralWidget/MapWidget")
        self.set_center(float(self.settings.value("center")[0]), float(self.settings.value("center")[1]))
        self.set_zoom(self.settings.value("zoom"))
        self.settings.endGroup()

    def new_data_reaction(self, data):
        points = []
        for i in range(len(data)):
            if (self.settings.value("CentralWidget/MapWidget/packet_name") == data[i][0]) and ((len(data[i]) - 2) >= 2):
                points.append(data[i][2:4])
        if len(points) > 0:
            if self.key is None:
                self.key = 0
                self.add_marker(self.key, points[-1][0], points[-1][1], **dict())
                self.add_polyline(self.key, points, **dict(color="red"))
            else:
                self.move_marker(self.key, points[-1][0], points[-1][1])
                self.add_points_to_polyline(self.key, points)

    def clear_data(self):
        if self.key is not None:
            self.delete_marker(self.key)
            self.delete_polyline(self.key)