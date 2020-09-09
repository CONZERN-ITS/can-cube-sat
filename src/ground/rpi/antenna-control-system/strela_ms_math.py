import numpy as NumPy
from math import sin, cos, radians, acos, asin

def dec_to_top_matix(lat, lon):
    lat = radians(lat)
    lon = radians(lon)
    return  NumPy.array([[-sin(lat)*cos(lon),  -sin(lat)*sin(lon), cos(lat)],
                         [ sin(lon),           -cos(lon),          0       ],
                         [ cos(lat)*cos(lon),   cos(lat)*sin(lon), sin(lat)]])

def top_to_gcscs_matix(mag, accel, decl):
    accel = accel / NumPy.linalg.norm(accel)
    mag = mag / NumPy.linalg.norm(mag)
    decl = radians(decl)

    mag = mag - accel * NumPy.dot(accel.T, mag)
    mag = mag / NumPy.linalg.norm(mag)
    mag = mag * cos(decl) + accel * (1 - cos(decl)) * NumPy.dot(accel.T, mag) + sin(decl) * NumPy.cross(accel.T, mag.T).T

    b = NumPy.cross(accel.T, mag.T).T
    b = b / NumPy.linalg.norm(b)

    return NumPy.column_stack([mag, b, accel])
