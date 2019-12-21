from PyQt5 import QtCore
import os.path

DEFAULT_SETTINGS_PATH = "./sourse/StrelA_MS_default.ini"
USER_SETTINGS_PATH = "./StrelA_MS.ini"

def init_settings():
    settings = QtCore.QSettings(USER_SETTINGS_PATH, QtCore.QSettings.IniFormat)
    return settings

def set_to_default(settings):
    config = open(settings.fileName(), 'w')
    def_config = open(DEFAULT_SETTINGS_PATH, 'r')
    for line in def_config:
        print(line)
        config.write(line)
    config.close()
    def_config.close()

def settings_test(settings):
    return os.path.exists(settings.fileName())
