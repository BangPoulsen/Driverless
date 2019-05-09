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
lamp_data = bpy.data.lamps.new(name="New Lamp", type='POINT')
lamp_object = bpy.data.objects.new(name="New Lamp", object_data=lamp_data)
bpy.context.scene.objects.link(lamp_object)
plane = bpy.data.objects["BBox"]
mesh = plane.data
mat_world_plane = plane.matrix_world
cs, ce = camera.data.clip_start, camera.data.clip_end

path = '/home/josef/Driverless/'
within = False
for i in range(1, 21):

    x = random.uniform(-50.0, 0)
    width_from_center = angles(x, camera.location.x, (camera.data.angle_x*0.5))

    y = random.uniform(-width_from_center, width_from_center)
    
    height_from_center = angles(x, camera.location.x, (camera.data.angle_y*0.5))

    z = random.uniform(-height_from_center, height_from_center)
    print(repr(x) + ' ' + repr(y) + ' ' + repr(z))
    cone.location = (x, y, z)
    lamp_object.location = (x, y, z+1)
    lamp_object.select = True
    bpy.context.scene.objects.active = lamp_object
    plane.location = (x, y, z+1.368449)
    
    bpy.context.scene.objects.active = plane
    if bpy.ops.object.mode_set.poll():
        bpy.ops.object.mode_set(mode='EDIT')
    assert plane.mode == 'EDIT'
    bm = bmesh.from_edit_mesh(mesh)
    
    vert_list = []
    for v in bm.verts:
        #print("mat_world_plane * v.co")
        #print(bpy_extras.object_utils.world_to_camera_view(bpy.context.scene, camera, mat_world_plane * v.co))
        vert_list.append(bpy_extras.object_utils.world_to_camera_view(bpy.context.scene, camera, mat_world_plane * v.co))
        #print("v.co") 
        #print(bpy_extras.object_utils.world_to_camera_view(bpy.context.scene, camera, v.co))
        co_ndc = bpy_extras.object_utils.world_to_camera_view(bpy.context.scene, camera, mat_world_plane * v.co)
        
        if(0.0 < co_ndc.x < 1.0 and 0.0 < co_ndc.y < 1.0 and cs < co_ndc.z < ce):
            within = True
        else:
            within = False
            break
    
    if(within == False):
        continue

    width = vert_list[2].x - vert_list[0].x
    print(width)
    height = vert_list[0].y - vert_list[1].y
    print(height)
    center_x = (vert_list[0].x + (width/2))
    center_y = 1 - (vert_list[1].y + (height/2))
    print("Center = " + repr(1 - (vert_list[0].x + (width/2))) + " " + repr(vert_list[1].y + (height/2)))
    

    local_bbox_center = 0.125 * sum((Vector(b) for b in plane.bound_box), Vector())
    global_bbox_center = plane.matrix_world * local_bbox_center
    print(repr(bpy_extras.object_utils.world_to_camera_view(bpy.context.scene, camera, Vector( (1.0 - global_bbox_center.x, global_bbox_center.y, global_bbox_center.z) ) )))
    print(repr(bpy_extras.object_utils.world_to_camera_view(bpy.context.scene, camera, Vector((1.0 - plane.location.x, plane.location.y, plane.location.z))) ) )

#    point1 = Vector((x, y-1.6, z-0.3))
#    point2 = Vector((x, y+1.6, z+3.2))
    bpy.data.scenes['Scene'].render.filepath = path + str(i) + '.jpg'
    bpy.ops.render.render( write_still=True )
#    cone_center = Vector( (cone.location.x, cone.location.y, cone.location.z+1.5) )
#    cone_coord = bpy_extras.object_utils.world_to_camera_view(bpy.context.scene, camera, cone_center)
#    coords1 = bpy_extras.object_utils.world_to_camera_view(bpy.context.scene, camera, point1)
#    coords2 = bpy_extras.object_utils.world_to_camera_view(bpy.context.scene, camera, point2)
#    width = coords2.x-coords1.x
#    height = coords2.y-coords1.y

    position = repr(center_x) + ' ' + repr(center_y) + ' ' + repr(width) + ' ' + repr(height) + "\n"
    f = open(path + str(i) + '.txt', "w+")
    f.write(position)
    f.close()
