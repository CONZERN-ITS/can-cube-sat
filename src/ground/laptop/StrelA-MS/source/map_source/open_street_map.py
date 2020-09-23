from PyQt5 import QtCore, QtWebEngineWidgets, QtNetwork, QtGui

import os
import json
import sys
import re


class OpenStreetMap(QtWebEngineWidgets.QWebEngineView):
    class WebEnginePage(QtWebEngineWidgets.QWebEnginePage):
        def acceptNavigationRequest(self, url, navtype, mainframe):
            return False

    def __init__(self):
        super(OpenStreetMap, self).__init__()

        self.setPage(OpenStreetMap.WebEnginePage())

        self.page().profile().setCachePath(os.path.abspath(os.path.dirname(__file__)) + "/cache")
        self.page().profile().setHttpCacheType(QtWebEngineWidgets.QWebEngineProfile.DiskHttpCache)

        if (os.name == "posix"):
            url = 'file://' + os.path.abspath(os.path.dirname(__file__)) + '/map.html'
        else:
            pattern = r"\\"
            url = re.sub(pattern, '/', 'file:///' + os.path.abspath(os.path.dirname(__file__)) + '/map.html')

        self.load(QtCore.QUrl(url))

    def run_script(self, script):
        return self.page().runJavaScript(script)

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

    def delete_first_n_points(self, key, num):
        self.run_script("delete_first_n_points(key={!r},"
                              "n={});".format(key, num))

    def add_point_to_polyline(self, key, latitude, longitude):
        self.run_script("add_point_to_polyline(key={!r},"
                       "latitude={}, "
                       "longitude={});".format(key, latitude, longitude))

    def add_points_to_polyline(self, key, points):
        self.run_script("add_points_to_polyline(key={!r},"
                              "{});".format(key, json.dumps(points)))
