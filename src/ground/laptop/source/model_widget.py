import os

from PyQt5 import QtGui, QtCore
import pyqtgraph as PG
import pyqtgraph.opengl as OpenGL
import numpy as NumPy
from stl import mesh as StlMesh
from itertools import chain
from math import acos, trunc

from source import settings_control
from source import RES_ROOT

MESH_PATH = os.path.join(RES_ROOT, "models/general_assembly_sw0001.stl")

class ModelWidget(OpenGL.GLViewWidget):
    def __init__(self):
        super(ModelWidget, self).__init__()

        self.setup_ui()
        self.setup_ui_design()

    def setup_ui(self):
        self.gird = OpenGL.GLGridItem()
        self.gird.scale(10, 10, 10)
        self.gird.translate(0, 0, -2)
        self.addItem(self.gird)

        verts = self._get_mesh_points(MESH_PATH)
        faces = NumPy.array([(i, i + 1, i + 2,) for i in range(0, len(verts), 3)])

        self.mesh = OpenGL.GLMeshItem(vertexes=verts, faces=faces, drawEdges=True, smooth=False, shader='edgeHilight', computeNormals=True)
        self.addItem(self.mesh)

    def setup_ui_design(self):
        self.setCameraPosition(distance=225, elevation=20, azimuth=270)

    def _get_mesh_points(self, mesh_path):
        mesh = StlMesh.Mesh.from_file(mesh_path)
        points = mesh.points
        points = NumPy.array(list(chain(*points)))
        nd_points = NumPy.ndarray(shape=(len(points) // 3, 3,))
        for i in range(0, len(points) // 3):
            nd_points[i] = points[i * 3: (i + 1) * 3]
        return nd_points

    def new_data_reaction(self, data):
        self.clear_data()
        quat = QtGui.QQuaternion(*data[-1])
        self._rotate_object(self.mesh, *quat.getAxisAndAngle())

    def _rotate_object(self, obj, axis, angle):
        obj.rotate(angle, axis[0], axis[1], axis[2])

    def clear_data(self):
        self.mesh.resetTransform()