import random
import math
from mathutils import Vector
import bpy
import bmesh
import bpy_extras
from bpy import context
    
def angles(cone_x, camera_x, fov):
    distance = camera_x - cone_x
    c = distance/math.cos(fov)
    x = c*math.sin(fov)
    print("A: " + repr(fov) + " C: 90 b: " + repr(distance) + " c: " + repr(c) + " c*sin(A): " + repr(x))
    return x
    
cone = bpy.data.objects["Cube.001"]
camera = bpy.data.objects["Camera"]
plane = bpy.data.objects["BBox"]
mesh = cone.data
mat_world_cone = cone.matrix_world
cs, ce = camera.data.clip_start, camera.data.clip_end

path = '/home/bp/Documents/Renders/'
within = False
for i in range(1, 11):

    x = random.uniform(-50.0, 0)
    width_from_center = angles(x, camera.location.x, (camera.data.angle_x*0.5))

    y = random.uniform(-width_from_center, width_from_center)
    
    height_from_center = angles(x, camera.location.x, (camera.data.angle_y*0.5))

    z = random.uniform(-height_from_center, height_from_center)
    print(repr(x) + ' ' + repr(y) + ' ' + repr(z))
    cone.location = (x, y, z)
    plane.location = (x, y, z+1.368449)
    
    #Face camera
    direction = camera.matrix_world.to_translation() - plane.matrix_world.to_translation()
    rot_quat = direction.to_track_quat('-Z', 'Y')
    plane.rotation_euler = rot_quat.to_euler() 
    
    bpy.context.scene.objects.active = cone
    if bpy.ops.object.mode_set.poll():
        bpy.ops.object.mode_set(mode='EDIT')
    assert cone.mode == 'EDIT'
    bm = bmesh.from_edit_mesh(mesh)
    
    for v in bm.verts:
        co_ndc = bpy_extras.object_utils.world_to_camera_view(bpy.context.scene, camera, mat_world_cone * v.co)
        
        if(0.0 < co_ndc.x < 1.0 and 0.0 < co_ndc.y < 1.0 and cs < co_ndc.z < ce):
            within = True
        else:
            within = False
            break
    
    if(within == False):
        continue
    
    point1 = Vector((x, y-1.6, z-0.3))
    point2 = Vector((x, y+1.6, z+3.2))
    bpy.data.scenes['Scene'].render.filepath = path + str(i) + '.jpg'
    bpy.ops.render.render( write_still=True )
    cone_center = Vector( (cone.location.x, cone.location.y, cone.location.z+1.5) )
    cone_coord = bpy_extras.object_utils.world_to_camera_view(bpy.context.scene, camera, cone_center)
    coords1 = bpy_extras.object_utils.world_to_camera_view(bpy.context.scene, camera, point1)
    coords2 = bpy_extras.object_utils.world_to_camera_view(bpy.context.scene, camera, point2)
    width = coords2.x-coords1.x
    height = coords2.y-coords1.y
    position = repr(cone_coord.x) + ' ' + repr(1.0 - cone_coord.y) + ' ' + repr(width) + ' ' + repr(height) + "\n"
    f = open(path + str(i) + '.txt', "w+")
    f.write(position)
    f.close()
