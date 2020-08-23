import bpy

exportTitles = 0

#bones = bpy.data.armatures["Armature"].bones
arm = bpy.data.objects['Armature']
obj = bpy.data.objects["Cube"]

f = open("animation2.txt", "w+")

#armature = bpy.context.scene.objects['Armature']
#action = bpy.data.actions["Walk_blocking"]
#
#
#for fcurve in action.fcurves:
#    pose_bone_path = fcurve.data_path.rpartition('.')[0]
#    pose_bone = armature.path_resolve(pose_bone_path)
#    
#    bone = pose_bone.bone
#    print("%s" % bone.name, file=f)
#    
#    ################################################################################
#    # Export Bone Coordinates
#    ################################################################################
#    if exportTitles == 1:
#        print("Bone Coordinates Start:", file=f)
#    
#    print("%f" % bone.head_local.x, file=f)
#    print("%f" % bone.head_local.y, file=f)
#    print("%f" % bone.head_local.z, file=f)
#
#    print("%f" % bone.tail_local.x, file=f)
#    print("%f" % bone.tail_local.y, file=f)
#    print("%f" % bone.tail_local.z, file=f)
#    
#    print("%f" % pose_bone.location[0], file=f)
#    print("%f" % pose_bone.location[1], file=f)
#    print("%f" % pose_bone.location[2], file=f)
#    
#    if exportTitles == 1:
#        print("Bone Coordinates End:", file=f)
#
#    print("", file=f)



bpy.data.scenes["Scene"].frame_current = 0

for i in range(3):
    if exportTitles == 1:
        print("Frame (%d)" % i, file=f)
        
    for poseBone in arm.pose.bones:
        bone = poseBone.bone
        print("%s" % bone.name, file=f)

        ################################################################################
        # Export Bone Coordinates
        ################################################################################
        if exportTitles == 1:
            print("Bone Coordinates Start:", file=f)

        #print("%f" % bone.head_local.x, file=f)
        #print("%f" % bone.head_local.y, file=f)
        #print("%f" % bone.head_local.z, file=f)

        #print("%f" % bone.tail_local.x, file=f)
        #print("%f" % bone.tail_local.y, file=f)
        #print("%f" % bone.tail_local.z, file=f)

        print("%f" % poseBone.location[0], file=f)
        print("%f" % poseBone.location[1], file=f)
        print("%f" % poseBone.location[2], file=f)


        print("%f" % poseBone.rotation_quaternion[0], file=f)
        print("%f" % poseBone.rotation_quaternion[1], file=f)
        print("%f" % poseBone.rotation_quaternion[2], file=f)
        print("%f" % poseBone.rotation_quaternion[3], file=f)
        
        if exportTitles == 1:
            print("Bone Coordinates End:", file=f)
        
    
    bpy.ops.screen.keyframe_jump(next=True)
        
print("END", file=f)      