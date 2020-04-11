from PyQt5 import QtWebKitWidgets, QtCore, QtNetwork, QtGui
import os
import json

class OpenStreetMap(QtWebKitWidgets.QWebView):
    def __init__(self):
        super(OpenStreetMap, self).__init__()

        cache = QtNetwork.QNetworkDiskCache()
        cache.setCacheDirectory(os.path.abspath(os.path.dirname(__file__)) + "/cache")
        self.page().networkAccessManager().setCache(cache)

        url = 'file://' + os.path.abspath(os.path.dirname(__file__)) + '/map.html'
        self.load(QtCore.QUrl(url))

        self.page().setLinkDelegationPolicy(QtWebKitWidgets.QWebPage.DelegateAllLinks)

        self.linkClicked.connect(QtGui.QDesktopServices.openUrl)

    def run_script(self, script):
        return self.page().mainFrame().evaluateJavaScript(script)

    def set_center(self, latitude, longitude):
        self.run_script("set_center({}, {})".format(latitude, longitude))

    def get_center(self):
        center = self.run_script("get_center()")
        return center['lat'], center['lng']

    def set_zoom(self, zoom):
        self.run_script("set_zoom({})".format(zoom))

    def add_marker(self, key, latitude, longitude, **extra):
        return self.run_script("add_marker(key={!r},"
                              "latitude={}, "
                              "longitude={}, "
                              "{});".format(key, latitude, longitude, json.dumps(extra)))

    def delete_marker(self, key):
        self.run_script("delete_marker(key={!r});".format(key))

    def move_marker(self, key, latitude, longitude):
        self.run_script("move_marker(key={!r},"
                       "latitude={}, "
                       "longitude={});".format(key, latitude, longitude))

    def get_marker_position(self, key):
        return self.run_script("get_marker_position(key={!r});".format(key))

    def add_polyline(self, key, points, **extra):
        return self.run_script("add_polyline(key={!r},"
                               "points={}, "
                               "{});".format(key, json.dumps(points), json.dumps(extra)))

    def delete_polyline(self, key):
        self.run_script("delete_polyline(key={!r});".format(key))

    def add_point_to_polyline(self, key, latitude, longitude):
        self.run_script("add_point_to_polyline(key={!r},"
                       "latitude={}, "
                       "longitude={});".format(key, latitude, longitude))

    def add_points_to_polyline(self, key, points):
        self.run_script("add_points_to_polyline(key={!r},"
                              "{});".format(key, json.dumps(points)))
