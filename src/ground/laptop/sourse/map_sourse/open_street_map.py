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

        self.loadFinished.connect(self.on_load_finished)

    def on_load_finished(self, ok):
        self.set_center(55.9, 37.8)
        self.set_zoom(11)
        print(self.get_center())
        self.add_marker('2', 55.9, 37.8, **dict(icon="http://maps.gstatic.com/mapfiles/ridefinder-images/mm_20_red.png"))
        self.move_marker('2', 55.93, 37.81)
        print(self.marker_position('2'))

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
                              "latitude= {}, "
                              "longitude= {}, "
                              "{});".format(key, latitude, longitude, json.dumps(extra)))

    def move_marker(self, key, latitude, longitude):
        self.run_script("move_marker(key={!r},"
                       "latitude= {}, "
                       "longitude= {});".format(key, latitude, longitude))

# ToDo: заставить работать насильно
    def marker_position(self, key):
        return self.run_script("pos_marker(key={!r});".format(key))


