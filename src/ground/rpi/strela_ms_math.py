import numpy as NumPy
from math import sin, cos, radians, acos, asin

def dec_to_top_matix(lat, lon):
    lat = radians(lat)
    lon = radians(lon)
    return  NumPy.array([[-sin(lat)*cos(lon), -sin(lat)*sin(lon) , cos(lat)],
                         [sin(lon), -cos(lon), 0],
                         [cos(lat)*cos(lon), cos(lat)*sin(lon), sin(lat)]])

def top_to_gcscs_matix(mag, accel, decl):
    accel = accel / NumPy.linalg.norm(accel)
    mag = mag / NumPy.linalg.norm(mag)

    mag = mag - accel * NumPy.dot(accel, mag)
    mag = mag / NumPy.linalg.norm(mag)
    mag = mag * cos(decl) + accel * (1 - cos(decl)) * NumPy.dot(accel, mag) + sin(decl) * NumPy.cross(accel, mag)

    b = NumPy.cross(accel, mag)
    b = b / NumPy.linalg.norm(b)

    return NumPy.linalg.inv(NumPy.array([mag, b, accel]))
