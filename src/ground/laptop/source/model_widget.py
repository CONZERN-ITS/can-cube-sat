import os

from PyQt5 import QtGui
import pyqtgraph as PG
import pyqtgraph.opengl as OpenGL
import numpy as NumPy
from stl import mesh as StlMesh
from itertools import chain
import struct

from source import settings_control
from source import RES_ROOT

MESH_PATH = os.path.join(RES_ROOT, "models/CanCubeSat-for-GKS.stl")
MESH_COLOR_PATH = os.path.join(RES_ROOT, "models/CanCubeSat-for-GKS_color.mfcl")
SCENE_MESH_PATH = os.path.join(RES_ROOT, "models/axis.stl")
SCENE_MESH_COLOR_PATH = os.path.join(RES_ROOT, "models/axis.mfcl")

class ModelWidget(OpenGL.GLViewWidget):
    def __init__(self):
        super(ModelWidget, self).__init__()
        self.settings = settings_control.init_settings()

        self.setBackgroundColor(self.settings.value("CentralWidget/ModelWidget/background_color"))

        self.setup_ui()
        self.setup_ui_design()
        self.update_current_values()

    def setup_ui(self):
        self.gird = OpenGL.GLGridItem()
        self.addItem(self.gird)

        self.axis = OpenGL.GLAxisItem()
        self.addItem(self.axis)

        self.mesh = OpenGL.GLMeshItem()
        self.addItem(self.mesh)

        self.scene = OpenGL.GLMeshItem()
        self.addItem(self.scene)

    def setup_ui_design(self):
        self.settings.beginGroup("CentralWidget/ModelWidget/Grid")
        if int(self.settings.value("is_on")):
            self.gird.show()
            self.gird.scale(*[float(num) for num in self.settings.value("scale")])
            self.gird.translate(*[float(num) for num in self.settings.value("translate")])
        else:
            self.gird.hide()
        self.settings.endGroup()

        self.settings.beginGroup("CentralWidget/ModelWidget/Axis")
        if int(self.settings.value("is_on")):
            self.axis.show()
            self.axis.scale(*[float(num) for num in self.settings.value("scale")])
            self.axis.translate(*[float(num) for num in self.settings.value("translate")])
        else:
            self.axis.hide()
        self.settings.endGroup()

        self.settings.beginGroup("CentralWidget/ModelWidget/Mesh")
        model_color = None
        try:
            verts = self._get_mesh_points(self.settings.value("path"))
            if int(self.settings.value("Colors/is_on")):
                model_color = self._get_face_colors(self.settings.value("Colors/path"))                
        except Exception:
            verts = self._get_mesh_points(MESH_PATH)
            model_color = self._get_face_colors(MESH_COLOR_PATH)

        faces = NumPy.array([(i, i + 1, i + 2,) for i in range(0, len(verts), 3)])

        self.mesh.setMeshData(vertexes=verts,
                              faces=faces, 
                              faceColors=model_color,
                              edgeColor=(0, 0, 0, 1),
                              drawEdges=int(self.settings.value("draw_edges")), 
                              drawFaces=int(self.settings.value("draw_faces")),
                              smooth=int(self.settings.value("smooth")), 
                              shader=self.settings.value("shader"), 
                              computeNormals=int(self.settings.value("compute_normals")))
        self.mesh.meshDataChanged()
        self.settings.endGroup()

        self.settings.beginGroup("CentralWidget/ModelWidget/Scene")
        if int(self.settings.value("is_on")):
            model_color = None
            try:
                verts = self._get_mesh_points(self.settings.value("path"))
                if int(self.settings.value("Colors/is_on")):
                    model_color = self._get_face_colors(self.settings.value("Colors/path"))                
            except Exception:
                verts = self._get_mesh_points(SCENE_MESH_PATH)
                model_color = self._get_face_colors(SCENE_MESH_COLOR_PATH)

                faces = NumPy.array([(i, i + 1, i + 2,) for i in range(0, len(verts), 3)])

                self.scene.setMeshData(vertexes=verts,
                                       faces=faces, 
                                       faceColors=model_color,
                                       edgeColor=(0, 0, 0, 1),
                                       drawEdges=int(self.settings.value("draw_edges")), 
                                       drawFaces=int(self.settings.value("draw_faces")),
                                       smooth=int(self.settings.value("smooth")), 
                                       shader=self.settings.value("shader"), 
                                       computeNormals=int(self.settings.value("compute_normals")))
                self.scene.meshDataChanged()
        self.settings.endGroup()

        self.settings.beginGroup("CentralWidget/ModelWidget/Camera")
        self.setCameraPosition(distance=int(self.settings.value("distance")),
                               elevation=int(self.settings.value("elevation")),
                               azimuth=int(self.settings.value("azimuth")))
        self.pan(*[int(pan_item) for pan_item in self.settings.value("pan")])
        self.settings.endGroup()
        
        self.packet_name = self.settings.value("CentralWidget/ModelWidget/packet_name")

    def update_current_values (self):
        pass

    def _get_face_colors(self, color_path):
        color_file = open(color_path, 'rb')
        bin_data = color_file.read()
        color_file.close()

        color = NumPy.ndarray(shape=(len(bin_data) // 16, 4,))
        for i in range(0, len(bin_data), 16):
            color[i // 16] = struct.unpack(">4f", bin_data[i: i + 16])
    
        return color

    def _get_mesh_points(self, mesh_path):
        mesh = StlMesh.Mesh.from_file(mesh_path)
        points = mesh.points
        points = NumPy.array(list(chain(*points)))
        nd_points = NumPy.ndarray(shape=(len(points) // 3, 3,))
        for i in range(0, len(points) // 3):
            nd_points[i] = points[i * 3: (i + 1) * 3]
        return nd_points

    def new_data_reaction(self, data):
        quat = None
        pack = data.get(self.packet_name, None)
        if pack is not None:
            if (pack.shape[1] - 1) >= 4:
                quat = pack[-1][1:]
                quat = QtGui.QQuaternion(*quat)
                self.clear_data()
                self._rotate_object(self.mesh, *quat.getAxisAndAngle())

    def _rotate_object(self, obj, axis, angle):
        obj.rotate(angle, axis[0], axis[1], axis[2])

    def clear_data(self):
        self.mesh.resetTransform()