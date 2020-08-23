import bpy

exportTitles = 0

#bones = bpy.data.armatures["Armature"].bones
arm = bpy.data.objects['Armature']
obj = bpy.data.objects["Cube"]


f = open("bone.txt", "w+")

#for i in range(4):
#    for j in range(4):
#        print("%f" % bone.matrix_world[i][j], file=f)
        
#print("\n", file=f)
    

#print("%s" % bones[0].name , file=f)    

#for bone in bones:
for poseBone in arm.pose.bones:
    bone = poseBone.bone
    print("%s" % bone.name, file=f)
    
    ################################################################################
    # Export Bone Coordinates
    ################################################################################
    if exportTitles == 1:
        print("Bone Coordinates Start:", file=f)
    
    print("%f" % bone.head_local.x, file=f)
    print("%f" % bone.head_local.y, file=f)
    print("%f" % bone.head_local.z, file=f)

    print("%f" % bone.tail_local.x, file=f)
    print("%f" % bone.tail_local.y, file=f)
    print("%f" % bone.tail_local.z, file=f)
    
    #print("%f" % poseBone.location[0], file=f)
    #print("%f" % poseBone.location[1], file=f)
    #print("%f" % poseBone.location[2], file=f)
    
    if exportTitles == 1:
        print("Bone Coordinates End:", file=f)
        
    ################################################################################
    # Export Bone Parent Names
    ################################################################################
    if exportTitles == 1:
        print("Bone Parent Names Start:", file=f)
  
    if bone.parent is not None:
        print("%s" % bone.parent.name , file=f)
    else:
        print("None", file=f)
            
    if exportTitles == 1:
        print("Bone Parent Names End:", file=f)
        
    ################################################################################
    #print("", file=f)
    
print("END", file=f)

################################################################################
# Export Vertex Group Weight Paint
################################################################################
#
#if exportTitles == 1:
#    print("Bone Parent Names Start:", file=f)
#  
#index = obj.vertex_groups[bone.name].index
#obj.data.vertices[0].groups
#for v in obj.data.vertices[index].groups:
#    w = v.groups[index].weight
#    print('Vertex',v.index,'has a weight of',w,'for bone',bone.name)
#
#if exportTitles == 1:
#    print("Bone Parent Names End:", file=f)
        
################################################################################
# Source: https://blender.stackexchange.com/questions/74461/exporting-weight-and
#         -bone-elements-to-a-text-file
################################################################################
obj_verts = obj.data.vertices
obj_group_names = [g.name for g in obj.vertex_groups]

for bone in arm.pose.bones:
    if bone.name not in obj_group_names:
        continue

    gidx = obj.vertex_groups[bone.name].index

    bone_verts = [v for v in obj_verts if gidx in [g.group for g in v.groups]]

    print("n%s" % bone.name, file=f)
    
    for v in bone_verts:
        print("%d" % v.index, file=f)  
        for g in v.groups:
            if g.group == gidx: 
                
                w = g.weight
                #print('Vertex',v.index,'has a weight of',w,'for bone',bone.name)
                print("%f" % w, file=f)
        #for g in v.co:
            #w = g
            #print('Vertex',v.index,'has a weight of',w,'for bone',bone.name)
            #print("v%f" % w, file=f)    
                
print("END", file=f)      
################################################################################