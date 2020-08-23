import bpy

#prop1 = 10
#prop2 = 4

x = 2
y = 4
z = 2

cube = bpy.data.objects["Cube"]
    
x = cube.location.x
y = cube.location.y
z = cube.location.z

#cube.scale.x = 3

#x = cube.location.x
#y = cube.location.y
#z = cube.location.z
#cube.dimensions.x = 3

#v = cube.data.vertices[0]
#co_final = cube.matrix_world * v.co
#print(co_final)

f = open("test.txt", "w+")

for i in range(4):
    for j in range(4):
        print("%f" % cube.matrix_world[i][j], file=f)
    
for v in cube.data.vertices:
    co_final = cube.matrix_world * v.co
    print(co_final)
    print("%f" % v.co.x, file=f)
    print("%f" % v.co.y, file=f)
    print("%f" % v.co.z, file=f)
    #print("(%f %f %f)" % (co_final.x,co_final.y,co_final.z), file=f)

print("len(cube.data.polygons) = %d" % len(cube.data.polygons))

for i in range(len(cube.data.polygons)):  
    print("len(cube.data.polygons[%d].vertices) = %d" % (i, len(cube.data.polygons[i].vertices)))
    for j in range(len(cube.data.polygons[i].vertices)):
        print("%d" % cube.data.polygons[i].vertices[j], file=f)
        print(cube.data.polygons[i].vertices[j])
    
print("len(cube.data.uv_layers.active.data) = %d" % len(cube.data.uv_layers.active.data))    
    
for i in range(len(cube.data.uv_layers.active.data)):
    print("%f" % cube.data.uv_layers.active.data[i].uv.x, file=f)
    print("%f" % cube.data.uv_layers.active.data[i].uv.y, file=f)
        
#bpy.data.objects["Cube"].data.polygons[0].vertices[0]


#for polygon in cube.data.polygons:
    #print("", polygon.index)

# write to file
#f = open("C:\Users\joonkang\Downloads\test.txt", "w")

#print("%f\n%f\n%f" % (x,y,z), file=f)
f.close()

print("Succesfully done")
