import bmesh
import bpy
 
# read back

#prop1 = 0
#prop2 = 0

x = 0
y = 0
z = 0


f = open("test.txt", "r")

#x = float(f.readline())
#y = float(f.readline())
#z = float(f.readline())


bpy.ops.mesh.primitive_cube_add(location=(x,y,z))
cube = bpy.data.objects["Cube"]

for i in range(4):
    for j in range(4):
        cube.matrix_world[i][j] = float(f.readline())
        

for v in cube.data.vertices:

    x = float(f.readline())
    y = float(f.readline())
    z = float(f.readline())
    
    print(x)
    print(y)
    print(z)
    
    v.co.x = x
    v.co.y = y
    v.co.z = z
    #(cube.matrix_world * v.co).x = x
    #(cube.matrix_world * v.co).y = y
    #(cube.matrix_world * v.co).z = z

for i in range(6):
    #reads in face data but doesn't use it yet
    v1 = float(f.readline())
    v2 = float(f.readline())
    v3 = float(f.readline())
    v4 = float(f.readline())

#bpy.ops.mesh.primitive_cylinder_add(vertices=5,radius=pro)

print("read_file.py: start reading uv coordinates")    

bpy.ops.mesh.uv_texture_add()

img = bpy.data.images.load("//cube.jpg")

for uv_tex_face in bpy.context.object.data.uv_textures.active.data:
    uv_tex_face.image = img


#cube.data.uv_texture_add()

#bm = bmesh.new()
#bm.from_mesh(cube.data)
#bm.uv_texture_add("hello")
#bm.to_mesh(cube.data)

for i in range(24):
    x = cube.data.uv_layers.active.data[i].uv.x = float(f.readline())
    y = cube.data.uv_layers.active.data[i].uv.y = float(f.readline())
    
    print("x = %f y = %f" % (x,y))   
   
print("read_file.py: end reading uv coordinates")
 
f.close()

print("read_file.py: Succesfully done")
 