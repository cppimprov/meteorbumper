
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
from bpy_extras import (node_shader_utils)
from bpy_extras.io_utils import (ExportHelper, orientation_helper, axis_conversion)
from mathutils import (Matrix, Vector, Color)
import itertools

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
	
@dataclass
class MaterialData:
	name: str()
	base_color: []
	emissive_color: []
	metallic: 0.0
	specular: 0.0
	roughness: 0.0
	alpha: 1.0
	ior: 1.45

@dataclass
class VertexData:
	vertex: []
	texture_coords: []
	normal: []
	tangent: []
	bitangent: []

@dataclass
class SumbeshData:
	vertex_data: []
	face_data: []

@orientation_helper(axis_forward = '-Z', axis_up = 'Y')
class ExportMBP(bpy.types.Operator, ExportHelper):
	"""Save an MBP File"""

	bl_idname = 'export_scene.mbp'
	bl_label = 'Export MBP'
	bl_options = {'PRESET'}

	filename_ext = '.mbp_model'
	filter_glob: StringProperty(default = '*.mbp_model', options = {'HIDDEN'})

	def do_json_export(self, filepath, material_data, transform_data, submesh_data, has_normals, has_tangents):
		
		# arrange data for export
		json_submeshes = []
		for m, s in zip(material_data, submesh_data):

			# mesh may have material assigned, but no vertices using the material
			if len(s.vertex_data) == 0: 
				continue

			# submesh material
			json_material = {
				'name': m.name,
				'base_color': m.base_color,
				'emissive_color': m.emissive_color,
				'metallic': m.metallic,
				'specular': m.specular,
				'roughness': m.roughness,
				'alpha': m.alpha,
				'ior': m.ior,
			}

			# submesh mesh
			json_mesh = {
				'vertices': [],
				'texture_coords': [ [] for l in s.vertex_data[0].texture_coords ],
				'normals': [],
				'tangents': [],
				'bitangents': [],
				'indices': [],
			}

			for v in s.vertex_data:
				json_mesh['vertices'].extend(v.vertex)
				[json_mesh['texture_coords'][i].extend(v.texture_coords[i]) for i in range(0, len(v.texture_coords))]
				if has_normals:
					json_mesh['normals'].extend(v.normal)
				if has_tangents:
					json_mesh['tangents'].extend(v.tangent)
					json_mesh['bitangents'].extend(v.bitangent)
			
			for f in s.face_data:
				json_mesh['indices'].extend(f)

			# add submesh to array
			json_submesh = { 'material': json_material, 'mesh': json_mesh }
			json_submeshes.append(json_submesh)
		
		json_mbp = { 'transform': transform_data, 'submeshes': json_submeshes }

		# write!
		out_file = open(filepath, 'w')
		json.dump(json_mbp, out_file, indent = 4)
		out_file.close()

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
		transform_data = matrix_to_array(matrix_to_opengl(export_obj.matrix_world))

		# negative scale: flip normals
		if export_obj.matrix_world.determinant() < 0.0:
			eval_mesh.flip_normals()

		# calculate tangents (may fail if invalid normals or no uvs)
		try:
			eval_mesh.calc_tangents() # todo: specify uv map to use?
		except:
			self.report({'INFO'}, 'Failed to calculate tangents for exported object.')
		
		# gather material data
		material_data = []
		for mat in eval_mesh.materials:
			eval_mat = node_shader_utils.PrincipledBSDFWrapper(mat, is_readonly = True)

			# no PBSDF - add a dummy material
			if not eval_mat:
				self.report({'INFO'}, 'Failed to create PrincipledBSDFWrapper for material: ' + mat.name)
				
				m = MaterialData('[missing]', [0.0, 0.0, 0.0], [1.0, 0.0, 1.0], 0.0, 0.0, 0.0, 1.0, 1.0)
				material_data.append(m)
				continue

			m = MaterialData(
				name = mat.name,
				base_color = eval_mat.base_color[:3],
				emissive_color = eval_mat.emission_color[:3],
				metallic = eval_mat.metallic,
				specular = eval_mat.specular,
				roughness = eval_mat.roughness,
				alpha = eval_mat.alpha,
				ior = eval_mat.ior,
			)

			material_data.append(m)

		# no materials - add a dummy material
		if len(material_data) == 0:
			self.report({'INFO'}, 'No materials found. Using "[missing]" material.')
			m = MaterialData('[missing]', [0.0, 0.0, 0.0], [1.0, 0.0, 1.0], 0.0, 0.0, 0.0, 1.0, 1.0)
			material_data.append(m)

		# convert and copy mesh data
		submesh_data = [SumbeshData([], []) for _ in material_data]

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

				# get the vertex data for this material
				vertex_data = submesh_data[polygon.material_index].vertex_data
				
				# add vertex and get the index (preventing duplicates)
				try:
					vi = vertex_data.index(v)
				except ValueError:
					vi = len(vertex_data)
					vertex_data.append(v)
				
				# add the vertex index to the face
				face.append(vi)
			
			# store the face
			face_data = submesh_data[polygon.material_index].face_data
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
		if not math.isclose(Vector(vertex_data[0].normal).length, 1.0, rel_tol = 1e-04):
			self.report({'INFO'}, 'Skipping export of normals, tangents and bitangents.')
			has_normals = False
		
		# check for invalid tangents (length != 1.0) (only check the first one for now)
		# bitangents should be valid if both normals and tangents are
		has_tangents = True
		if not has_normals or not math.isclose(Vector(vertex_data[0].tangent).length, 1.0, rel_tol = 1e-04):
			self.report({'INFO'}, 'Skipping export of tangents and bitangents.')
			has_tangents = False
		
		assert len(submesh_data) == len(material_data)

		# export! (json for now)
		self.do_json_export(filepath, material_data, transform_data, submesh_data, has_normals, has_tangents)

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


# todo:

	# load json in game

	# implement binary export
	# load binary in game

	# remove orientation_helper (we're not using it...)