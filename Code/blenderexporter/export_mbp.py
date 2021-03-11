
bl_info = {
	"name": "MBP model format",
	"version": (0, 0, 1),
	"blender": (2, 83, 1),
	"location": "File > Import-Export",
	"category": "Import-Export",
}

import bpy

from bpy.props import (StringProperty)
from bpy_extras.io_utils import (ExportHelper, orientation_helper, axis_conversion)
from mathutils import (Matrix, Vector, Color)


def do_export(context, filepath, *, global_matrix):
	print("exporting...!")
	return {"FINISHED"}

@orientation_helper(axis_forward = '-Z', axis_up = 'Y')
class ExportMBP(bpy.types.Operator, ExportHelper):
	"""Save an MBP File"""

	bl_idname = "export_scene.mbp"
	bl_label = "Export MBP"
	bl_options = {"PRESET"}

	filename_ext = ".mbp_model"
	filter_glob: StringProperty(default = "*.mbp_model", options = {"HIDDEN"})

	def execute(self, context):

		keywords = self.as_keywords(ignore = ("axis_forward", "axis_up", "check_existing", "filter_glob"))
		keywords["global_matrix"] = axis_conversion(to_forward = self.axis_forward, to_up = self.axis_up).to_4x4()

		return do_export(context, **keywords)
	
	def draw(self, context):
		pass

def menu_func_export(self, context):
	self.layout.operator(ExportMBP.bl_idname, text = "MBP model (.mbp_model)")

classes = [ExportMBP]

def register():
	for c in classes:
		bpy.utils.register_class(c)
		
	bpy.types.TOPBAR_MT_file_export.append(menu_func_export)

def unregister():
	bpy.types.TOPBAR_MT_file_export.remove(menu_func_export)

	for c in classes:
		bpy.utils.unregister_class(c)

if __name__ == "__main__":
	register()
