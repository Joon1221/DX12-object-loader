#include "avatar.h"
#include "mesh.h"
#include "util.h"
#include "line.h"

Avatar::Avatar() {
}

Avatar::~Avatar() {
}

void Avatar::Render() {
	//msgBoxCstring("Avatar::Render()");

	for (int i = 0; i < meshes.size(); i++) {
		meshes[i]->Render();
	}
	anims["Anim"]->Update(); // load anim info to bones
	//-----------------------
	// for gpu sknning by kj 
	//-----------------------
	//applyWeights();
	//-----------------------
}

// read md5 object data
bool Avatar::loadMD5Mesh(string filePath) {

	// =======================================================================================================================
	// =======================================================================================================================
	// =======================================================================================================================

	char cStrBuf1[2048];

	//sprintf(cStrBuf1, "Avatar::loadFileMesh(): objFilePathWithFilname = %s", filePath.c_str());
	//msgBoxCstring(cStrBuf1);

	ifstream fin;
	//fin.open("C:\\Users\\chai31\\Documents\\study\\kj\\blender_importer_exporter\\imp_exp\\character_creation.obj");
	//fin.open("..\\..\\imp_exp\\character_creation.obj");
	fin.open(filePath.c_str());
	if (fin.fail()) {
		MessageBox(NULL, L"fin", L"Error", MB_OK | MB_ICONERROR);
		exit(1);
	}

	char buf[1024];

	string curToken;
	int curMeshIndex = -1;
	while (!fin.eof()) {
		fin >> curToken;
		//sprintf(cStrBuf1, "Avatar::loadFileMesh(): curToken = %s", curToken.c_str());
		//msgBoxCstring(cStrBuf1);
		if (curToken == "MD5Version") {
			fin >> curToken;
			//sprintf(cStrBuf1, "Avatar::loadFileMesh(): md5Version = %s", curToken.c_str());
			//msgBoxCstring(cStrBuf1);
		}
		else if (curToken == "commandline") {
			fin.getline(buf, 1024);
			//sprintf(cStrBuf1, "Avatar::loadFileMesh(): buf = %s", buf);
			//msgBoxCstring(cStrBuf1);
		}
		else if (curToken == "numJoints") {
			fin >> numJoints;
			//sprintf(cStrBuf1, "Avatar::loadFileMesh(): numJoints = %d", numJoints);
			//msgBoxCstring(cStrBuf1);
		}
		else if (curToken == "numMeshes") {
			fin >> numMeshes;
			//sprintf(cStrBuf1, "Avatar::loadFileMesh(): md5Version = %d", numMeshes);
			//msgBoxCstring(cStrBuf1);
		}
		else if (curToken == "joints") {
			fin >> curToken; // read {
			for (int i = 0; i < numJoints; i++) {
				Bone *curBone = new Bone();
				fin >> curToken; 

				while (curToken.find("\"", 1) == string::npos) {
					string temp;
					fin >> temp;
					curToken += " " + temp;
				}
				curBone->name = curToken.substr(1, curToken.size()-2);

				int parentIndex;
				fin >> parentIndex;

				if (parentIndex == -1) {
					curBone->parentBone = NULL;
				}
				else {
					curBone->parentBone = bones[parentIndex];
				}

				fin >> curToken; // read (

				fin >> curBone->pos.x;
				fin >> curBone->pos.y;
				fin >> curBone->pos.z;

				fin >> curToken; // read )
				fin >> curToken; // read (

				fin >> curBone->rot.x;
				fin >> curBone->rot.y;
				fin >> curBone->rot.z;

				// calculate rot.w using 3 values
				// 여기에서 쿼터니온의 처음 3개 value를 이용해서, 네번째인 w를 계산해서..
				// 제대로 된 quternion을 만들고, 그것을 4x4 matrix로 바꿔서..
				// 아래에서 pos(vecter4)와 XMVectorTransform()으로 곱한다.
				ComputeQuatW(curBone->rot);

				fin >> curToken; // read )
				fin.getline(buf, 1024); // read any extra comments 

				//sprintf(cStrBuf1, "Avatar::loadFileMesh(): curBone->name = %s", curBone->name);
				//msgBoxCstring(cStrBuf1);			
				//sprintf(cStrBuf1, "Avatar::loadFileMesh(): parentIndex = %d", parentIndex);
				//msgBoxCstring(cStrBuf1);		
				//sprintf(cStrBuf1, "Avatar::loadFileMesh(): curBone->pos.x = %f", curBone->pos.x);
				//msgBoxCstring(cStrBuf1);
				//sprintf(cStrBuf1, "Avatar::loadFileMesh(): curBone->pos.y = %f", curBone->pos.y);
				//msgBoxCstring(cStrBuf1);
				//sprintf(cStrBuf1, "Avatar::loadFileMesh(): curBone->pos.z = %f", curBone->pos.z);
				//msgBoxCstring(cStrBuf1);
				//sprintf(cStrBuf1, "Avatar::loadFileMesh(): curBone->rot.x = %f", curBone->rot.x);
				//msgBoxCstring(cStrBuf1);
				//sprintf(cStrBuf1, "Avatar::loadFileMesh(): curBone->rot.y = %f", curBone->rot.y);
				//msgBoxCstring(cStrBuf1);
				//sprintf(cStrBuf1, "Avatar::loadFileMesh(): curBone->rot.z = %f", curBone->rot.z);
				//msgBoxCstring(cStrBuf1);

				//sprintf(cStrBuf1, "curBone get pos (%f, %f, %f)", curBone->getPos().x, curBone->getPos().y, curBone->getPos().z);
				//msgBoxCstring(cStrBuf1);

				bones.push_back(curBone);

				//Line *curLine = new Line();
				//if (parentIndex == -1) {
				//	curLine->createYAxisLine(XMFLOAT3(0.0f, 0.0f, 0.0f), curBone->getPos());
				//}
				//else {
				//	curLine->createYAxisLine(curBone->parentBone->getPos(), curBone->getPos());
				//}

				//meshes.push_back(curLine);
			}
			fin >> curToken; // read }
		}
		else if (curToken == "mesh") {
			curMeshIndex++;
			Mesh *curMesh = new Mesh();
			curMesh->usesQuadFace = false;

			fin >> curToken; // read {
			while (curToken != "}") {
				fin >> curToken; 
				if (curToken == "shader") {
					fin >> curToken;
					string shaderFilePath = curToken.substr(1, curToken.size() - 2);
					//sprintf(cStrBuf1, "Avatar::loadFileMesh(): shaderFilePath = %s", shaderFilePath.c_str());
					//msgBoxCstring(cStrBuf1);
				}
				else if (curToken == "numverts") {
					int numVerts;
					fin >> numVerts; 

					for (int i = 0; i < numVerts; i++) {
						fin >> curToken; // read vert
						if (curToken == "vert") {
							fin >> curToken; // read vertex index
							fin >> curToken; // read (

							float u;
							float v;
							fin >> u;
							fin >> v;
							//v = 1.0f - v;

							fin >> curToken; // read )


							int weightStartIndex;
							fin >> weightStartIndex;

							int weightCount;
							fin >> weightCount;

							// 에러 잡음. Vertex에 uv를 잘못 넣음.
							//curMesh->finalVertices.push_back(Vertex(u, v, 0, 0, 0, weightStartIndex, weightCount));
							curMesh->finalVertices.push_back(Vertex(0, 0, 0, u, v, weightStartIndex, weightCount));

							//sprintf(cStrBuf1, "Avatar::loadFileMesh(): u = %f", u);
							//msgBoxCstring(cStrBuf1);

							//sprintf(cStrBuf1, "Avatar::loadFileMesh(): v = %f", v);
							//msgBoxCstring(cStrBuf1);

							//sprintf(cStrBuf1, "Avatar::loadFileMesh(): weightStartIndex = %d", weightStartIndex);
							//msgBoxCstring(cStrBuf1);

							//sprintf(cStrBuf1, "Avatar::loadFileMesh(): weightCount = %d", weightCount);
							//msgBoxCstring(cStrBuf1);
						}
					}
				}
				else if (curToken == "numtris") {
					int numTris;
					fin >> numTris;

					for (int i = 0; i < numTris; i++) {
						fin >> curToken; // read tri
						if (curToken == "tri") {
							fin >> curToken; // read vertex index
							int index1;
							int index2;
							int index3;
							fin >> index1;
							fin >> index2;
							fin >> index3; 

							curMesh->triFaces.push_back(TriFace(index1, index2, index3));

							//sprintf(cStrBuf1, "Avatar::loadFileMesh(): index1 = %d", index1);
							//msgBoxCstring(cStrBuf1);

							//sprintf(cStrBuf1, "Avatar::loadFileMesh(): index2 = %d", index2);
							//msgBoxCstring(cStrBuf1);

							//sprintf(cStrBuf1, "Avatar::loadFileMesh(): index3 = %d", index3);
							//msgBoxCstring(cStrBuf1);
						}
					}
				}
				else if (curToken == "numweights") {

					int numWeights;
					fin >> numWeights;

					vector<Weight> tempVector;

					for (int i = 0; i < numWeights; i++) {
						fin >> curToken; // read tri
						if (curToken == "weight") {
							fin >> curToken; // read weight index 

							int jointIndex;
							fin >> jointIndex; 

							float weight;
							fin >> weight;

							fin >> curToken; // read (

							float x;
							float y;
							float z;

							fin >> x;
							fin >> y;
							fin >> z;

							fin >> curToken; // read )

							tempVector.push_back(Weight(jointIndex, weight, x, y, z));

							//sprintf(cStrBuf1, "Avatar::loadFileMesh(): jointIndex = %d", jointIndex);
							//msgBoxCstring(cStrBuf1);

							//sprintf(cStrBuf1, "Avatar::loadFileMesh(): weight = %f", weight);
							//msgBoxCstring(cStrBuf1);

							//sprintf(cStrBuf1, "Avatar::loadFileMesh(): x = %f", x);
							//msgBoxCstring(cStrBuf1);

							//sprintf(cStrBuf1, "Avatar::loadFileMesh(): y = %f", y);
							//msgBoxCstring(cStrBuf1);

							//sprintf(cStrBuf1, "Avatar::loadFileMesh(): z = %f", z);
							//msgBoxCstring(cStrBuf1);
						}
					}
					weights.push_back(tempVector);
				}
			}

			//-----------------------
			// for gpu sknning by kj 
			//-----------------------
			// initializes the weights and bone data of the vertices for gpu skinning 
			// this is only temporary and is just here to test the gpu skinner code
			for (int i = 0; i < curMesh->finalVertices.size(); i++) {
				Vertex &vert = curMesh->finalVertices[i];
				float tempWeights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
				float tempWeightPosX[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
				float tempWeightPosY[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
				float tempWeightPosZ[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
				for (int j = 0; j < vert.numWeights; j++) {
					Weight &weight = weights[meshes.size()][vert.weightIndex + j];

					tempWeights[j] = weight.weight;

					tempWeightPosX[j] = weight.pos.x;
					tempWeightPosY[j] = weight.pos.y;
					tempWeightPosZ[j] = weight.pos.z;

					vert.boneIndices[j] = weight.boneIndex;

					//vert.boneRot[j] = (*bones[weight.boneIndex]).rot;
					//vert.bonePos[j] = XMFLOAT3toXMFLOAT4((*bones[weight.boneIndex]).pos);

					//sprintf(cStrBuf1, "Avatar::loadFileMesh(): weight = %f", temp[j]);
					//msgBoxCstring(cStrBuf1);
				}
				vert.weights = XMFLOAT4(tempWeights[0], tempWeights[1], tempWeights[2], tempWeights[3]);
				vert.weightPosX = XMFLOAT4(tempWeightPosX[0], tempWeightPosX[1], tempWeightPosX[2], tempWeightPosX[3]);
				vert.weightPosY = XMFLOAT4(tempWeightPosY[0], tempWeightPosY[1], tempWeightPosY[2], tempWeightPosY[3]);
				vert.weightPosZ = XMFLOAT4(tempWeightPosZ[0], tempWeightPosZ[1], tempWeightPosZ[2], tempWeightPosZ[3]);
				//vert.weights = XMFLOAT4(1.0f, 2.0f, 3.0f, 4.0f);
			}
			//-----------------------


			meshes.push_back(curMesh);
			
		}
	}

	applyWeights(); // must be done once for t-pose
	
	// =======================================================================================================================
	// =======================================================================================================================
	// =======================================================================================================================
 
	return true;
}

// apply weights to the bone armatures
void Avatar::applyWeights() {
	for (int i = 0; i < numMeshes; ++i) {
		Mesh *curMesh = meshes[i];

		for (int j = 0; j < curMesh->finalVertices.size(); ++j) {
			Vertex &vert = curMesh->finalVertices[j];
			//glm::vec3& pos = mesh.m_PositionBuffer[i];
			XMFLOAT3 &pos = curMesh->finalVertices[j].pos;
			//glm::vec3& normal = mesh.m_NormalBuffer[i];
			//XMFLOAT3 &normal = curMesh->finalVertices[i].normal;

			//pos = glm::vec3(0);
			//pos = XMFLOAT3();

			pos.x = 0.0f;
			pos.y = 0.0f;

			pos.z = 0.0f;

			//normal = glm::vec3(0);
			//normal = XMFLOAT3(0);

			//XMVECTOR finalPosVec = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

			for (int k = 0; k < vert.numWeights; ++k) {
				//const Weight& weight = mesh.m_Weights[vert.m_StartWeight + j];
				//Weight &weight = weights[vert.weightIndex + j];
				Weight &weight = weights[i][vert.weightIndex + k];
				//const MD5Animation::SkeletonJoint& joint = skel.m_Joints[weight.m_JointID];
				Bone &bone = *bones[weight.boneIndex];

				//glm::vec3 rotPos = joint.m_Orient * weight.m_Pos;
				//XMVECTOR rotVec = XMLoadFloat4(&bone.rot);
				//XMMATRIX rotMat = XMMatrixTranslationFromVector(rotVec);

				//XMFLOAT4 weightPosFloat4;
				//weightPosFloat4.x = weight.pos.x;
				//weightPosFloat4.y = weight.pos.y;
				//weightPosFloat4.z = weight.pos.z;
				//weightPosFloat4.w = 1.0f;
				//XMVECTOR weightPosVec = XMLoadFloat4(&weightPosFloat4);

				////XMFLOAT3 rotPos = bone.rot * weight.pos;
				//XMVECTOR rotPosVec = XMVector4Transform(weightPosVec, rotMat);
				//XMFLOAT3 rotPos;
				//XMStoreFloat3(&rotPos, rotPosVec);
				XMFLOAT3 rotPos = rotateVectorByQuat(bone.rot, weight.pos);

				//XMFLOAT4 bonePosFloat4;
				//bonePosFloat4.x = bone.pos.x;
				//bonePosFloat4.y = bone.pos.y;
				//bonePosFloat4.z = bone.pos.z;
				//bonePosFloat4.w = 0.0f;

				//XMVECTOR bonePosVec = XMLoadFloat4(&bonePosFloat4);

				//pos += (joint.m_Pos + rotPos) * weight.m_Bias;
				//finalPosVec += (bonePosVec + rotPosVec) * weight.weight;

				pos.x += (bone.pos.x + rotPos.x) * weight.weight;
				pos.y += (bone.pos.y + rotPos.y) * weight.weight;
				pos.z += (bone.pos.z + rotPos.z) * weight.weight;
				//finalPosVec += ((bonePosVec + rotPosVec) * weight.weight);

				//normal += (joint.m_Orient * vert.m_Normal) * weight.m_Bias;
			}

			float temp = pos.y;
			pos.y = pos.z;
			pos.z = temp;

			//finalPosVec /= vert.numWeights;
			//XMStoreFloat3(&vert.pos, finalPosVec);
			//int a = 10;
		}
	}
}

bool Avatar::loadMD5Anim(string filePath) {
	anims["Anim"] = new Animation();
	anims["Anim"]->avatar = this;
	anims["Anim"]->loadMD5Anim(filePath);
	return true;
}