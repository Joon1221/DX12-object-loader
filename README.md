# DX12 Object Loader

An object loader that can display models in a 3d space using the DX12 library. 

<p align="center">
  <img src="./assets/demo2.gif" alt="Size Limit CLI" width="600">
</p>

Features
--------

- Load 3d models from **.obj** and **.md5mesh** files.
- Camera movement (**rotating**, **panning**, **translating**, **zooming**).
- **Animation** from **.md5anim** files using **gpu-skinning**.
- Load **multiple** 3d models at once in the same world.
- Create **custom** 3d models.


How it Works
------------
1. Component based system is used to give the 3d models different attirbutes. These attributes store data of the 3d model such as animation frame data and uv texture coordinate data.
<p align="center">
  <img src="./assets/uml.png" alt="Size Limit CLI" width="738">
</p>

2. Positional data of 3d models during an animation are calculated using bone armatures and weighted vertices. Each frame, the bones in the bone armature are translated and rotated to a different position, and each bone applies a different weight to every vertex in the 3d model which determines how much the vertex should move according to that specific bone. 

3. Animations are handled on `VertexShader.hlsl` via the process of gpu-skinning. 

``` HLSL
  float4 tempPos;
  tempPos.x = 0.0f;
  tempPos.y = 0.0f;
  tempPos.z = 0.0f;
  tempPos.w = 1.0f;

  // apply weight and rotation
  for (int i = 0; i < 4; i++) {
    float3 weightPos = float3(input.weightsPosX[i], input.weightsPosY[i], input.weightsPosZ[i]);

    float3 rotPos = rotateVectorByQuat(boneRot[input.boneIndices[i]], weightPos);

    tempPos.x += (bonePos[input.boneIndices[i]].x + rotPos.x) * input.weights[i];
    tempPos.y += (bonePos[input.boneIndices[i]].y + rotPos.y) * input.weights[i];
    tempPos.z += (bonePos[input.boneIndices[i]].z + rotPos.z) * input.weights[i];
  }

  float temp = tempPos.y;
  tempPos.y = tempPos.z;
  tempPos.z = temp;

  // set position 
  output.pos = mul(tempPos, wvpMat);
    
```


Usage
-----

### Importing 3d Models


### Camera Movement

Rotate: `RMB + Move`\
Zoom: `Scroll Wheel`\
Pan: `Ctrl + RMB + Move`


Staff
-----

Programmer: Joon Kang


Mentor: Samil Chai


Work Period
-----------

Start Date: 2018-03-11


End Date: 2019-01-13


Privacy
-------

*Portions of source code have been removed for privacy purposes.*

