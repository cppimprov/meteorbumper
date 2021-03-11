
bl_info = {
	'name': 'MBP model format',
	'version': (0, 0, 1),
	'blender': (2, 83, 1),
	'location': 'File > Import-Export',
	'category': 'Import-Export',
}

from dataclasses import dataclass
import json
import math

import bpy
from bpy.props import (StringProperty)
from bpy_extras.io_utils import (ExportHelper, orientation_helper, axis_conversion)
from mathutils import (Matrix, Vector, Color)

def matrix_to_opengl(matrix):
	m = matrix.copy()
	for i in range(0, 4): m.col[1][i], m.col[2][i] = m.col[2][i], m.col[1][i]
	for i in range(0, 4): m.row[1][i], m.row[2][i] = m.row[2][i], m.row[1][i]
	for i in range(0, 4): m.col[2][i] = -m.col[2][i]
	for i in range(0, 4): m.row[2][i] = -m.row[2][i]
	return m

def matrix_to_array(matrix):
	return [x for x in itertools.chain.from_iterable(matrix.col)]

def vector_to_opengl(vector):
	v = vector.copy()
	v[1], v[2] = v[2], -v[1]
	return v
	
def vector_to_array(vector):
	return [x for x in vector]

@dataclass
class VertexData:
	vertex: []
	texture_coords: []
	normal: []
	tangent: []
	bitangent: []

@orientation_helper(axis_forward = '-Z', axis_up = 'Y')
class ExportMBP(bpy.types.Operator, ExportHelper):
	"""Save an MBP File"""

	bl_idname = 'export_scene.mbp'
	bl_label = 'Export MBP'
	bl_options = {'PRESET'}

	filename_ext = '.mbp_model'
	filter_glob: StringProperty(default = '*.mbp_model', options = {'HIDDEN'})

	def do_export(self, context, filepath, *, global_matrix):
		
		depsgraph = context.evaluated_depsgraph_get()
		scene = context.scene

		# exit edit mode
		if bpy.ops.object.mode_set.poll():
			bpy.ops.object.mode_set(mode = 'OBJECT')
		
		# only export single meshes for now
		if len(context.selected_objects) != 1:
			self.report({'ERROR'}, 'Select a single object for export (multi-object export not currently supported).')
			return {'CANCELLED'}
		
		export_obj = context.selected_objects[0]

		if export_obj.type != 'MESH':
			self.report({'ERROR'}, 'Selected object must be a mesh.')
			return {'CANCELLED'}

		self.report({'INFO'}, 'Exporting object: ' + export_obj.name)
		
		# apply modifiers
		eval_obj = export_obj.evaluated_get(depsgraph)
		eval_mesh = eval_obj.to_mesh()
		#eval_mesh.transform(global_matrix @ export_obj.matrix_world)

		# negative scale: flip normals
		if export_obj.matrix_world.determinant() < 0.0:
			eval_mesh.flip_normals()

		# calculate tangents (may fail if invalid normals or no uvs)
		try:
			eval_mesh.calculate_tangents()
		except:
			self.report({'INFO'}, 'Failed to calculate tangents for exported object.')
		
		# todo: material data! what to do for multiple materials?!

		# convert and copy mesh data
		vertex_data = []
		face_data = []

		for polygon in eval_mesh.polygons:

			# ensure mesh is triangulated
			if len(polygon.loop_indices) != 3:
				self.report({'ERROR'}, 'Only triangle meshes supported.')
				return {'CANCELLED'}
			
			# loop over the face
			face = []
			for loop_index in polygon.loop_indices:
				loop = eval_mesh.loops[loop_index]

				# convert the data to opengl-style coordinates
				vertex = vector_to_opengl(eval_mesh.vertices[loop.vertex_index].co)
				texture_coords = [layer.data[loop.vertex_index].uv for layer in eval_mesh.uv_layers]
				normal = vector_to_opengl(loop.normal)
				tangent = vector_to_opengl(loop.tangent)
				bitangent = vector_to_opengl(loop.bitangent_sign * normal.cross(tangent))

				# copy the vertex data
				v = VertexData(vertex[:], [uv_layer[:] for uv_layer in texture_coords], normal[:], tangent[:], bitangent[:])

				# add to vertex data and get the index (preventing duplicates)
				try:
					vi = vertex_data.index(v)
				except ValueError:
					vi = len(vertex_data)
					vertex_data.append(v)
				
				# add the vertex index to the face
				face.append(vi)
			
			# store the face
			face_data.append(face)
		
		# cleanup
		eval_obj.to_mesh_clear()

		# check that we have vertices:
		if len(vertex_data) == 0:
			self.report({'WARNING'}, 'No vertices found for export!')
			return {'CANCELLED'}
		
		# check that we have faces:
		if len(face_data) == 0:
			self.report({'WARNING'}, 'No faces found for export!')
			return {'CANCELLED'}

		# check for invalid normals (length != 1.0) (only check the first one for now)
		has_normals = True
		if not math.isclose(Vector(vertex_data[0].normal).length, 1.0, rel_tol = 1e-07):
			self.report({'INFO'}, 'Skipping export of normals, tangents and bitangents.')
			has_normals = False
		
		# check for invalid tangents (length != 1.0) (only check the first one for now)
		# bitangents should be valid if both normals and tangents are
		has_tangents = True
		if not has_normals or not math.isclose(Vector(vertex_data[0].tangent).length, 1.0, rel_tol = 1e-07):
			self.report({'INFO'}, 'Skipping export of tangents and bitangents.')
			has_tangents = False
		
		# prepare for export (json for now)
		mesh_data = {
			'vertices': [],
			'texture_coords': [ [] for l in vertex_data[0].texture_coords ],
			'normals': [],
			'tangents': [],
			'bitangents': [],
			'indices': [],
		}

		for v in vertex_data:
			mesh_data['vertices'].append(v.vertex)
			[mesh_data['texture_coords'][i].append(v.texture_coords[i]) for i in range(0, len(v.texture_coords))]
			if has_normals:
				mesh_data['normals'].append(v.normal)
			if has_tangents:
				mesh_data['tangents'].append(v.tangent)
				mesh_data['bitangents'].append(v.bitangent)
		
		for f in face_data:
			mesh_data['indices'].extend(f)

		# write!
		out_file = open(filepath, 'w')
		json.dump(mesh_data, out_file, indent = 4)
		out_file.close()

		return {'FINISHED'}
	
	def execute(self, context):

		keywords = self.as_keywords(ignore = ('axis_forward', 'axis_up', 'check_existing', 'filter_glob'))
		keywords['global_matrix'] = axis_conversion(to_forward = self.axis_forward, to_up = self.axis_up).to_4x4()

		return self.do_export(context, **keywords)
	
	def draw(self, context):
		pass

def menu_func_export(self, context):
	self.layout.operator(ExportMBP.bl_idname, text = 'MBP model (.mbp_model)')

classes = [ExportMBP]

def register():
	for c in classes:
		bpy.utils.register_class(c)
		
	bpy.types.TOPBAR_MT_file_export.append(menu_func_export)

def unregister():
	bpy.types.TOPBAR_MT_file_export.remove(menu_func_export)

	for c in classes:
		bpy.utils.unregister_class(c)

if __name__ == '__main__':
	register()
