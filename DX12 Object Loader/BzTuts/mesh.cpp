#include "mesh.h"
#include "d3dApp.h"

#include <regex>

extern D3DApp* gd3dApp;

Mesh::Mesh() : Renderable() {
	usesQuadFace = true;
}

Mesh::Mesh(string filePath) : Renderable() {
	loadFile(filePath);
	usesQuadFace = true;
}

Mesh::~Mesh() {
}

bool Mesh::Init() {
	//char cStrBuf1[2048];
	//sprintf(cStrBuf1, "Mesh::Init()");
	//msgBoxCstring(cStrBuf1);
	
	//================================================================================
	//================================================================================
	// DX12 related
	//================================================================================
	//================================================================================

	HRESULT hr;

	ID3D12Resource *indexBuffer; // a default buffer in GPU memory that we will load vertex data for our triangle into
	
	Vertex *vList = &(finalVertices[0]);
	//int vBufferSize = sizeof(Vertex) * objModels.at(objIndex).finalVertices.size();
	int vBufferSize = sizeof(Vertex) * finalVertices.size();

	char buf[256];
	//sprintf(buf, "main.cpp::initObject(): objModel.finalVertices.size(): %d", finalVertices.size());
	//msgBoxCstring(buf);

	//sprintf(g_buffer, "main.cpp::initObject(): vBufferSize: %d", vBufferSize);
	//msgBoxCstring(g_buffer);

	// create default heap
	// default heap is memory on the GPU. Only the GPU has access to this memory
	// To get data into this heap, we will have to upload the data using
	// an upload heap

	hr = gd3dApp->device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), // a default heap
		D3D12_HEAP_FLAG_NONE, // no flags
		&CD3DX12_RESOURCE_DESC::Buffer(vBufferSize), // resource description for a buffer
		D3D12_RESOURCE_STATE_COPY_DEST, // we will start this heap in the copy destination state since we will copy data
										// from the upload heap to this heap
		nullptr, // optimized clear value must be null for this type of resource. used for render targets and depth/stencil buffers
		//IID_PPV_ARGS(&vertexBuffers[objIndex]));
		IID_PPV_ARGS(&vertexBuffer));
	if (FAILED(hr))
	{
		gd3dApp->Running = false;
		//return false;
		return false;
	}

	// we can give resource heaps a name so when we debug with the graphics debugger we know what resource we are looking at
	//vertexBuffers[objIndex]->SetName(L"Vertex Buffer Resource Heap");
	vertexBuffer->SetName(L"Vertex Buffer Resource Heap");

	// create upload heap
	// upload heaps are used to upload data to the GPU. CPU can write to it, GPU can read from it
	// We will upload the vertex buffer using this heap to the default heap
	//ID3D12Resource* vBufferUploadHeap; // move to global to reuse in Render()
	hr = gd3dApp->device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // upload heap
		D3D12_HEAP_FLAG_NONE, // no flags
		&CD3DX12_RESOURCE_DESC::Buffer(vBufferSize), // resource description for a buffer
		D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
		nullptr,
		IID_PPV_ARGS(&vBufferUploadHeap));
	if (FAILED(hr))
	{
		gd3dApp->Running = false;
		//return false;
		return false;
	}
	vBufferUploadHeap->SetName(L"Vertex Buffer Upload Resource Heap");

	// store vertex buffer in upload heap
	D3D12_SUBRESOURCE_DATA vertexData = {};
	vertexData.pData = reinterpret_cast<BYTE*>(vList); // pointer to our vertex array
	vertexData.RowPitch = vBufferSize; // size of all our triangle vertex data
	vertexData.SlicePitch = vBufferSize; // also the size of our triangle vertex data

										 // we are now creating a command with the command list to copy the data from
										 // the upload heap to the default heap
	//UpdateSubresources(commandList, vertexBuffers[objIndex], vBufferUploadHeap, 0, 0, 1, &vertexData);
	UpdateSubresources(gd3dApp->commandList, vertexBuffer, vBufferUploadHeap, 0, 0, 1, &vertexData);

	// transition the vertex buffer data from copy destination state to vertex buffer state
	//commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(vertexBuffers[objIndex], D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
	gd3dApp->commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(vertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

	char cStrBuf1[2048];
	sprintf(cStrBuf1, "main.cpp::initObject(): quadFaces.size(): %d", quadFaces.size());
	//msgBoxCstring(cStrBuf1);

	if (usesQuadFace) {
		for (int i = 0; i < quadFaces.size(); i++) {
			//for (int i = 0; i < 437; i++) {
			QuadFace curQuadFace = quadFaces[i];
			if (curQuadFace.indices[3] == -1) { // tri face
				triFaces.push_back(TriFace(curQuadFace.indices[0], curQuadFace.indices[1], curQuadFace.indices[2]));
			}
			else {  // quad face
				triFaces.push_back(TriFace(curQuadFace.indices[3], curQuadFace.indices[0], curQuadFace.indices[1]));
				triFaces.push_back(TriFace(curQuadFace.indices[2], curQuadFace.indices[3], curQuadFace.indices[1]));
			}
		}
	}


	//ObjModel *curObjModel = &objModels.at(0);
	//for (int t = 0; t < 100; t++) {
	//	curObjModel->finalVertices.at(curObjModel->triFaces[t].indices[0]).texCoord.x = 0;
	//	curObjModel->finalVertices.at(curObjModel->triFaces[t].indices[0]).texCoord.y = 0;
	//	curObjModel->finalVertices.at(curObjModel->triFaces[t].indices[1]).texCoord.x = 0;
	//	curObjModel->finalVertices.at(curObjModel->triFaces[t].indices[1]).texCoord.y = 0;
	//	curObjModel->finalVertices.at(curObjModel->triFaces[t].indices[2]).texCoord.x = 0;
	//	curObjModel->finalVertices.at(curObjModel->triFaces[t].indices[2]).texCoord.y = 0;
	//}

	DWORD *iList = (DWORD *)&(triFaces[0]);
	int iBufferSize = sizeof(TriFace) * triFaces.size();

	//numObjIndices.push_back(objModels.at(objIndex).triFaces.size() * 3);
	numIndices = triFaces.size() * 3;

	//sprintf(g_buffer, "main.cpp::initObject(): triFaces.size(): %d", objModels.at(objIndex).triFaces.size());
	//msgBoxCstring(g_buffer);
	//sprintf(g_buffer, "main.cpp::initObject(): iBufferSize: %d", iBufferSize);
	//msgBoxCstring(g_buffer);
	//sprintf(g_buffer, "main.cpp::initObject(): numCubeIndices: %d", numObjIndices);
	//msgBoxCstring(g_buffer);

	// create default heap to hold index buffer
	hr = gd3dApp->device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), // a default heap
		D3D12_HEAP_FLAG_NONE, // no flags
		&CD3DX12_RESOURCE_DESC::Buffer(iBufferSize), // resource description for a buffer
		D3D12_RESOURCE_STATE_COPY_DEST, // start in the copy destination state
		nullptr, // optimized clear value must be null for this type of resource
		//IID_PPV_ARGS(&indexBuffers[objIndex]));
		IID_PPV_ARGS(&indexBuffer));
	if (FAILED(hr))
	{
		gd3dApp->Running = false;
		return false;
	}

	// we can give resource heaps a name so when we debug with the graphics debugger we know what resource we are looking at
	//vertexBuffers[objIndex]->SetName(L"Index Buffer Resource Heap");
	vertexBuffer->SetName(L"Index Buffer Resource Heap");

	// create upload heap to upload index buffer
	//	D3D11_BIND_SHADER_RESOURCE
	//gd3dApp->device->CreateCommittedResource()

	ID3D12Resource* iBufferUploadHeap;
	hr = gd3dApp->device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // upload heap
		D3D12_HEAP_FLAG_NONE, // no flags
		&CD3DX12_RESOURCE_DESC::Buffer(iBufferSize), // resource description for a buffer
		D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
		nullptr,
		IID_PPV_ARGS(&iBufferUploadHeap));
	if (FAILED(hr))
	{
		gd3dApp->Running = false;
		return false;
	}
	vBufferUploadHeap->SetName(L"Index Buffer Upload Resource Heap");

	// store vertex buffer in upload heap
	D3D12_SUBRESOURCE_DATA indexData = {};
	indexData.pData = reinterpret_cast<BYTE*>(iList); // pointer to our index array
	indexData.RowPitch = iBufferSize; // size of all our index buffer
	indexData.SlicePitch = iBufferSize; // also the size of our index buffer

										// we are now creating a command with the command list to copy the data from
										// the upload heap to the default heap
	//UpdateSubresources(commandList, indexBuffers[objIndex], iBufferUploadHeap, 0, 0, 1, &indexData);
	UpdateSubresources(gd3dApp->commandList, indexBuffer, iBufferUploadHeap, 0, 0, 1, &indexData);

	// transition the vertex buffer data from copy destination state to vertex buffer state
	//commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(indexBuffers[objIndex], D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
	gd3dApp->commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(indexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

	// Create the depth/stencil buffer

	//// create a depth stencil descriptor heap so we can get a pointer to the depth stencil buffer
	//D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	//dsvHeapDesc.NumDescriptors = 1;
	//dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	//dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	////hr = device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsDescriptorHeaps[objIndex]));
	//hr = gd3dApp->device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsDescriptorHeap));
	//if (FAILED(hr))
	//{
	//	gd3dApp->Running = false;
	//	return false;
	//}

	//D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
	//depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
	//depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	//depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

	//D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
	//depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	//depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
	//depthOptimizedClearValue.DepthStencil.Stencil = 0;

	////hr = device->CreateCommittedResource(
	//hr = gd3dApp->device->CreateCommittedResource(
	//	&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
	//	D3D12_HEAP_FLAG_NONE,
	//	//&CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, Width, Height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
	//	&CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, gd3dApp->Width, gd3dApp->Height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
	//	D3D12_RESOURCE_STATE_DEPTH_WRITE,
	//	&depthOptimizedClearValue,
	//	IID_PPV_ARGS(&depthStencilBuffer)
	//);
	//if (FAILED(hr))
	//{
	//	gd3dApp->Running = false;
	//	return false;
	//}
	////dsDescriptorHeaps[objIndex]->SetName(L"Depth/Stencil Resource Heap");
	//dsDescriptorHeap->SetName(L"Depth/Stencil Resource Heap");

	////device->CreateDepthStencilView(depthStencilBuffer, &depthStencilDesc, dsDescriptorHeaps[objIndex]->GetCPUDescriptorHandleForHeapStart());
	//gd3dApp->device->CreateDepthStencilView(depthStencilBuffer, &depthStencilDesc, dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	// load the image, create a texture resource and descriptor heap

	// create the descriptor heap that will store our srv
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = 1;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	//hr = device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&mainDescriptorHeaps[objIndex]));
	hr = gd3dApp->device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&mainDescriptorHeap));
	if (FAILED(hr))
	{
		gd3dApp->Running = false;
	}

	// Load the image from file
	D3D12_RESOURCE_DESC textureDesc;
	int imageBytesPerRow;
	//int imageSize = LoadImageDataFromFile(&imageData, textureDesc, L"imp_exp.png", imageBytesPerRow);
	//int imageSize = LoadImageDataFromFile(&imageData, textureDesc, L"..\\..\\imp_exp\\imp_exp.png", imageBytesPerRow);
	//int imageSize = LoadImageDataFromFile(&imageData, textureDesc, L"..\\..\\imp_exp\\ash_uvgrid03.jpg", imageBytesPerRow);

	//size_t len = strlen(objModelImagePaths.at(objIndex).c_str());
	size_t len = strlen(imagePath.c_str());
	WCHAR imagePathWChar[280 + 1];
	//int result = MultiByteToWideChar(CP_OEMCP, 0, objModelImagePaths.at(objIndex).c_str(), -1, imagePath, len + 1);
	int result = MultiByteToWideChar(CP_OEMCP, 0, imagePath.c_str(), -1, imagePathWChar, len + 1);

	//char cStrBuf1[2048];
	//sprintf(cStrBuf1, "main.cpp::initObject(): initObject(): objModelImagePaths[%d] = |%s|", objIndex, objModelImagePaths.at(objIndex).c_str());
	//msgBoxCstring(cStrBuf1);

	BYTE * imageData;

	//int imageSize = LoadImageDataFromFile(&imageData, textureDesc, imagePath, imageBytesPerRow);
	int imageSize = gd3dApp->LoadImageDataFromFile(&imageData, textureDesc, imagePathWChar, imageBytesPerRow);
	//int imageSize = LoadImageDataFromFile(&imageData, textureDesc, L"..\\..\\imp_exp\\character_creation.png", imageBytesPerRow);

	// make sure we have data
	if (imageSize <= 0)
	{
		gd3dApp->Running = false;
		return false;
	}

	ID3D12Resource* textureBuffer; // the resource heap containing our texture

	// create a default heap where the upload heap will copy its contents into (contents being the texture)
	//hr = device->CreateCommittedResource(
	hr = gd3dApp->device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), // a default heap
		D3D12_HEAP_FLAG_NONE, // no flags
		&textureDesc, // the description of our texture
		D3D12_RESOURCE_STATE_COPY_DEST, // We will copy the texture from the upload heap to here, so we start it out in a copy dest state
		nullptr, // used for render targets and depth/stencil buffers
		IID_PPV_ARGS(&textureBuffer));
	if (FAILED(hr))
	{
		gd3dApp->Running = false;
		return false;
	}
	textureBuffer->SetName(L"Texture Buffer Resource Heap");

	UINT64 textureUploadBufferSize;
	// this function gets the size an upload buffer needs to be to upload a texture to the gpu.
	// each row must be 256 byte aligned except for the last row, which can just be the size in bytes of the row
	// eg. textureUploadBufferSize = ((((width * numBytesPerPixel) + 255) & ~255) * (height - 1)) + (width * numBytesPerPixel);
	//textureUploadBufferSize = (((imageBytesPerRow + 255) & ~255) * (textureDesc.Height - 1)) + imageBytesPerRow;
	//device->GetCopyableFootprints(&textureDesc, 0, 1, 0, nullptr, nullptr, nullptr, &textureUploadBufferSize);
	gd3dApp->device->GetCopyableFootprints(&textureDesc, 0, 1, 0, nullptr, nullptr, nullptr, &textureUploadBufferSize);


	ID3D12Resource* textureBufferUploadHeap;
	// now we create an upload heap to upload our texture to the GPU
	//hr = device->CreateCommittedResource(
	hr = gd3dApp->device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // upload heap
		D3D12_HEAP_FLAG_NONE, // no flags
		&CD3DX12_RESOURCE_DESC::Buffer(textureUploadBufferSize), // resource description for a buffer (storing the image data in this heap just to copy to the default heap)
		D3D12_RESOURCE_STATE_GENERIC_READ, // We will copy the contents from this heap to the default heap above
		nullptr,
		IID_PPV_ARGS(&textureBufferUploadHeap));
	if (FAILED(hr))
	{
		gd3dApp->Running = false;
		return false;
	}
	textureBufferUploadHeap->SetName(L"Texture Buffer Upload Resource Heap");

	// store vertex buffer in upload heap
	D3D12_SUBRESOURCE_DATA textureData = {};
	textureData.pData = &imageData[0]; // pointer to our image data
	textureData.RowPitch = imageBytesPerRow; // size of all our triangle vertex data
	textureData.SlicePitch = imageBytesPerRow * textureDesc.Height; // also the size of our triangle vertex data

																	// Now we copy the upload buffer contents to the default heap
	//UpdateSubresources(commandList, textureBuffer, textureBufferUploadHeap, 0, 0, 1, &textureData);
	UpdateSubresources(gd3dApp->commandList, textureBuffer, textureBufferUploadHeap, 0, 0, 1, &textureData);

	// transition the texture default heap to a pixel shader resource (we will be sampling from this heap in the pixel shader to get the color of pixels)
	//commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(textureBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	gd3dApp->commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(textureBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

	// now we create a shader resource view (descriptor that points to the texture and describes it)
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	//device->CreateShaderResourceView(textureBuffer, &srvDesc, mainDescriptorHeaps[objIndex]->GetCPUDescriptorHandleForHeapStart());
	gd3dApp->device->CreateShaderResourceView(textureBuffer, &srvDesc, mainDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	// we are done with image data now that we've uploaded it to the gpu, so free it up
	delete imageData;

	// create a vertex buffer view for the triangle. We get the GPU memory address to the vertex pointer using the GetGPUVirtualAddress() method
	//vertexBufferViews[objIndex].BufferLocation = vertexBuffers[objIndex]->GetGPUVirtualAddress();
	//vertexBufferViews[objIndex].StrideInBytes = sizeof(Vertex);
	//vertexBufferViews[objIndex].SizeInBytes = vBufferSize;
	vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vertexBufferView.StrideInBytes = sizeof(Vertex);
	vertexBufferView.SizeInBytes = vBufferSize;

	// create a index buffer view for the triangle. We get the GPU memory address to the vertex pointer using the GetGPUVirtualAddress() method
	//indexBufferViews[objIndex].BufferLocation = indexBuffers[objIndex]->GetGPUVirtualAddress();
	//indexBufferViews[objIndex].Format = DXGI_FORMAT_R32_UINT; // 32-bit unsigned integer (this is what a dword is, double word, a word is 2 bytes)
	//indexBufferViews[objIndex].SizeInBytes = iBufferSize;
	indexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
	indexBufferView.Format = DXGI_FORMAT_R32_UINT; // 32-bit unsigned integer (this is what a dword is, double word, a word is 2 bytes)
	indexBufferView.SizeInBytes = iBufferSize;

	//char cStrBuf1[2048];
	sprintf(cStrBuf1, "main.cpp::initObject(): initObject(): end!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
	//msgBoxCstring(cStrBuf1);

	return true;
}

bool Mesh::loadFile(string filePath) {

	char cStrBuf1[2048];
	//sprintf(cStrBuf1, "Mesh::loadObjFile(): filePath = %s", filePath.c_str());
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

	//fin.getline(buf, 1024);

	//while (!fin.eof()) {
	//	sprintf(cStrBuf1, "%s", buf);
	//	msgBoxCstring(cStrBuf1);

	//	fin.getline(buf, 1024);
	//}

	//fin.close();

	//exit(1);

/*
	//-------------------------------------------------------------------------
	// load file into a string.
	//-------------------------------------------------------------------------
	HANDLE hFile;
	DWORD _bytes_read;
	char strVal[655360];

	string path = GetWorkingDir();
	size_t len1 = strlen(path.c_str());
	WCHAR unistring1[280 + 1];
	int result1 = MultiByteToWideChar(CP_OEMCP, 0, path.c_str(), -1, unistring1, len1 + 1);

	MessageBox(NULL, unistring1, L"Error", MB_OK);

	size_t objFilePathWithFilnameLen = objFilePathWithFilname.length();
	WCHAR objFilePathWithFilnameL[280 + 1];
	int result = MultiByteToWideChar(CP_OEMCP, 0, objFilePathWithFilname.c_str(), -1, objFilePathWithFilnameL, objFilePathWithFilnameLen + 1);
	hFile = CreateFile(objFilePathWithFilnameL, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	ReadFile(hFile, strVal, 655360, &_bytes_read, NULL);

	char cStrBuf[80 + 1];
	sprintf(cStrBuf, "_bytes_read = %d", _bytes_read);

	CloseHandle(hFile);

	string tempStr = string(strVal);
*/

//================================================================================
//================================================================================
// Obj File Importer
//================================================================================
//================================================================================
//msgBoxCstring((char *)(tempStr.c_str()));

/*
	//-------------------------------------------------------------------------
	// cut off first 2 lines
	//-------------------------------------------------------------------------
	int curIndex = -1;
	for (int i = 0; i < 2; i++) {
		curIndex = tempStr.find("\n", curIndex+1);
	}
	tempStr = tempStr.substr(curIndex + 1);
	//msgBoxCstring((char *)(tempStr.c_str()));
*/
	fin.getline(buf, 1024);
	fin.getline(buf, 1024);

	//-------------------------------------------------------------------------
	//get material name
	//-------------------------------------------------------------------------
	//curIndex = tempStr.find("\n", 0);
	//string matStr = tempStr.substr(0, curIndex + 1);
	//matStr = matStr.substr(7);
	fin.getline(buf, 1024);
	string matStr = string("") + &buf[7];

	//loadMtlFile("..\\..\\imp_exp\\" + matStr);

	//curIndex = tempStr.find("\n", 0);
	//tempStr = tempStr.substr(curIndex + 1);

	//msgBoxCstring((char *)(matStr.c_str()));
	//msgBoxCstring((char *)(tempStr.c_str()));
	//exit(1);


	//-------------------------------------------------------------------------
	// get shape name
	//-------------------------------------------------------------------------
	//curIndex = tempStr.find("\n", 0);
	//string shapeStr = tempStr.substr(0, curIndex + 1);
	fin.getline(buf, 1024);
	//shapeStr = shapeStr.substr(2);
	//tempStr = tempStr.substr(curIndex + 1);
	string shapeStr = string("") + &buf[2];

	//msgBoxCstring((char *)(shapeStr.c_str()));
	//msgBoxCstring((char *)(tempStr.c_str()));

	//get vertex values
	int curIndex = -1;
	int numVertices = 0;
	while (true) {
		//curIndex = tempStr.find("\n", 0);
		//string vertexStr = tempStr.substr(0, curIndex + 1);
		fin.getline(buf, 1024);
		string vertexStr = string("") + buf;

		if (vertexStr.find("v ", 0) == std::string::npos) {
			break;
		}

		vertexStr = vertexStr.substr(2); // cut off "v "

		curIndex = vertexStr.find(" ", 0);
		string xStr = vertexStr.substr(0, curIndex + 1); // x coord

		int prevIndex = curIndex;

		curIndex = vertexStr.find(" ", curIndex + 1);

		string yStr = vertexStr.substr(prevIndex, curIndex + 1); // y coord

		string zStr = vertexStr.substr(curIndex + 1); // z coord

		numVertices++;

		//curIndex = tempStr.find("\n", 0);
		//tempStr = tempStr.substr(curIndex + 1);

		//char cStrBuf1[2048];
		sprintf(cStrBuf1, "(%f, %f, %f) ", atof(xStr.c_str()), atof(yStr.c_str()), atof(zStr.c_str()));

		Vertex curVertex(atof(xStr.c_str()), atof(yStr.c_str()), atof(zStr.c_str()), -1.0f, -1.0f);
		vertices.push_back(curVertex);

		//msgBoxCstring((char *)(cStrBuf1));
	}
	//char cStrBuf1[2048];
	sprintf(cStrBuf1, "numVertices = %d", numVertices);
	//msgBoxCstring(cStrBuf1);

	// get uv mapping value
	int count = 0;
	while (true) {
		//curIndex = tempStr.find("\n", 0);
		//string uvMapStr = tempStr.substr(0, curIndex + 1);
		string uvMapStr = string("") + buf;

		if (uvMapStr.find("vt ", 0) == std::string::npos) {
			break;
		}

		uvMapStr = uvMapStr.substr(3); // cut off "vt "

		curIndex = uvMapStr.find(" ", 0);
		string xStr = uvMapStr.substr(0, curIndex + 1); // x coord

		string yStr = uvMapStr.substr(curIndex + 1); // y coord

		Vertex curVertex(-1.0f, -1.0f, -1.0f, atof(xStr.c_str()), 1.0f - atof(yStr.c_str()));
		uvVertices.push_back(curVertex);

		//curIndex = tempStr.find("\n", 0);
		//tempStr = tempStr.substr(curIndex + 1);

		//char cStrBuf1[2048];
		sprintf(cStrBuf1, "(%f, %f) ", atof(xStr.c_str()), atof(yStr.c_str()));
		//msgBoxCstring((char *)(cStrBuf1));
		count++;
		fin.getline(buf, 1024);
	}

	//char cStrBuf1[2048];
	sprintf(cStrBuf1, "ObjModel::loadObjFile(): num vt: %d", count);
	//msgBoxCstring((char *)(cStrBuf1));
	sprintf(cStrBuf1, "ObjModel::loadObjFile(): uvVertices.size(): %d", uvVertices.size());
	//msgBoxCstring((char *)(cStrBuf1));

	visited = new int[uvVertices.size()];
	for (int i = 0; i < uvVertices.size(); i++) {
		visited[i] = -1;
	}

	// * ¾Æ·¡ÀÇ vnµéÀ» ±×³É skip
	//-------------------------------------------------------------------------
	// get face normal values
	//-------------------------------------------------------------------------
	while (true) {
		//curIndex = tempStr.find("\n", 0);
		//string faceNormalStr = tempStr.substr(0, curIndex + 1);
		string faceNormalStr = string("") + buf;

		if (faceNormalStr.find("vn ", 0) == std::string::npos) {
			break;
		}

		faceNormalStr = faceNormalStr.substr(3); // cut off "vn "

		curIndex = faceNormalStr.find(" ", 0);
		string xStr = faceNormalStr.substr(0, curIndex + 1); // x coord

		int prevIndex = curIndex;

		curIndex = faceNormalStr.find(" ", curIndex + 1);

		string yStr = faceNormalStr.substr(prevIndex, curIndex + 1); // y coord

		string zStr = faceNormalStr.substr(curIndex + 1); // z coord

		//curIndex = tempStr.find("\n", 0);
		//tempStr = tempStr.substr(curIndex + 1);

		//char cStrBuf1[2048];
		//sprintf(cStrBuf1, "(%f, %f, %f) ", atof(xStr.c_str()), atof(yStr.c_str()), atof(zStr.c_str()));
		//msgBoxCstring((char *)(cStrBuf1));

		fin.getline(buf, 1024);
	}

	//-------------------------------------------------------------------------
	// get used material name
	//-------------------------------------------------------------------------
	//curIndex = tempStr.find("\n", 0);
	//string usedMatStr = tempStr.substr(0, curIndex + 1);
	//usedMatStr = usedMatStr.substr(7);
	//fin.getline(buf, 1024);
	string usedMatStr = string("") + &buf[7];

	//curIndex = tempStr.find("\n", 0);
	//tempStr = tempStr.substr(curIndex + 1);

	//msgBoxCstring((char *)(usedMatStr.c_str()));
	//msgBoxCstring((char *)(tempStr.c_str()));

	//-------------------------------------------------------------------------
	// get smooth shading 
	//-------------------------------------------------------------------------
	//curIndex = tempStr.find("\n", 0);
	//string smoothShadingStr = tempStr.substr(0, curIndex + 1);
	//smoothShadingStr = smoothShadingStr.substr(2);
	fin.getline(buf, 1024);
	string smoothShadingStr = string("") + &buf[2];

	//curIndex = tempStr.find("\n", 0);
	//tempStr = tempStr.substr(curIndex + 1);

	//msgBoxCstring((char *)(smoothShadingStr.c_str()));
	//msgBoxCstring((char *)(tempStr.c_str()));

	//for (int i = 0; i < 24; i++) {
	//	vertices.at(atoi(vertexIndices.c_str()) - 1).texCoord.x = uvVertices.at(atoi(vertexTextureIndices.c_str()) - 1).texCoord.x;
	//}

	//-------------------------------------------------------------------------
	// get faces indices
	//-------------------------------------------------------------------------
	//for (int i = 0; i < numFaces; i++) {
	int numFaces = 0;
	while (true) {
		//curIndex = tempStr.find("\n", 0);
		//string faceIndicesStr = tempStr.substr(0, curIndex + 1);
		fin.getline(buf, 1024);
		//string faceIndicesStr = string("|") + buf + string("|");
		string faceIndicesStr = string("") + buf;

		if (fin.eof() || faceIndicesStr.find("f ", 0) == std::string::npos) {
			//msgBoxCstring((char *)(tempStr.c_str()));
			//char cStrBuf1[2048];
			sprintf(cStrBuf1, "ObjModel::loadObjFile():  break at %d: faceIndicesStr: %s", numFaces, faceIndicesStr);
			//msgBoxCstring(cStrBuf1);
			break;
		}

		numFaces++;

		regex r("f\\s+(\\d+\\s+|\\d+\/\\d+\\s+|\\d+\/\\d+\/\\d+\\s+|\\d+\/\/\\d+\\s+){2,}(\\d+\/\\d+\/\\d+|\\d+\/\/\\d+|\\d+\/\\d+|\\d+)$");

		smatch m;
		regex_search(faceIndicesStr, m, r);

		if (m[0].matched) {
			regex words_regex("\\s+(\\d+\/\/\\d+|\\d+\/\\d+\/\\d+|\\d+\/\\d+|\\d+)");
			auto words_begin =
				sregex_iterator(faceIndicesStr.begin(), faceIndicesStr.end(), words_regex);
			auto words_end = sregex_iterator();

			//cout << "Found "
			//	<< distance(words_begin, words_end)
			//	<< " words:\n";

			int indices[4];
			indices[3] = -1;

			int count = 0;

			for (sregex_iterator j = words_begin; j != words_end; ++j) {

				//cout << "------------------------------------------" << endl;
				smatch match = *j;
				string match_str = match.str();
				//				cout << match_str << '\n';
								//msgBoxCstring((char *)match_str.c_str());

				curIndex = match_str.find("/", 0);
				string vertexIndices = match_str.substr(0, curIndex); // x coord

				int prevIndex = curIndex + 1;

				curIndex = match_str.find("/", curIndex + 1);

				string vertexTextureIndices = match_str.substr(prevIndex, curIndex - prevIndex); // y coord

				string vertexNormalIndices = match_str.substr(curIndex + 1); // z coord

				int curVertexIndex = atoi(vertexIndices.c_str()) - 1;
				int curUvIndex = atoi(vertexTextureIndices.c_str()) - 1;
				//char cStrBuf1[2048];
				//sprintf(cStrBuf1, "%d", visited[curUvIndex]);
				//msgBoxCstring(cStrBuf1);

				if (visited[curUvIndex] == -1) {
					Vertex curFinalVertex(vertices.at(curVertexIndex).pos.x, vertices.at(curVertexIndex).pos.y, vertices.at(curVertexIndex).pos.z, uvVertices.at(curUvIndex).texCoord.x, uvVertices.at(curUvIndex).texCoord.y);
					curFinalVertex.boneIndices[0] = 255; // If the object is a non-skinned mesh (a regular mesh) the first bone index is set to 
														 // 255 to tell the vertex shader to bypass the gpu skinning and use the 'old' vertex shader
														 // algorithm. This must be done in order for the regular meshes to appear on the screen.
														 // **Note that this is only temporary and this needs to be changed after creating a seperate
														 //   PSO and shaders for the skinned meshes.

					finalVertices.push_back(curFinalVertex);
					visited[curUvIndex] = finalVertices.size() - 1;

					//char cStrBuf1[2048];
					//sprintf(cStrBuf1, "(%f, %f, %f, %f, %f)", curFinalVertex.pos.x, curFinalVertex.pos.y, curFinalVertex.pos.z, curFinalVertex.texCoord.x, curFinalVertex.texCoord.y);
					//msgBoxCstring(cStrBuf1);
				}
				indices[count] = visited[curUvIndex];

				//if (vertices.at(atoi(vertexIndices.c_str()) - 1).texCoord.x == -1.0f) {
					//vertices.at(curVertexIndex).texCoord.x = uvVertices.at(curUvIndex).texCoord.x;
				//}
				//if (vertices.at(atoi(vertexIndices.c_str()) - 1).texCoord.y == -1.0f) {
					//vertices.at(curVertexIndex).texCoord.y = uvVertices.at(curUvIndex).texCoord.y;
				//}

				string temp = vertexIndices + " | " + vertexTextureIndices + " | " + vertexNormalIndices;

				//indices[count] = atoi(vertexIndices.c_str());
				count++;

				//msgBoxCstring((char *)temp.c_str());

				//for (ssub_match v : match) {
				//	string curToken = v.str();
				//	cout << curToken << endl;
				//}
			}

			quadFaces.push_back(QuadFace(indices));
			//char cStrBuf1[2048];
			//sprintf(cStrBuf1, "(%d, %d, %d, %d)", indices[0], indices[1], indices[2], indices[3]);
			//msgBoxCstring(cStrBuf1);


			//char cStrBuf1[2048];
			//sprintf(cStrBuf1, "(%d, %d, %d, %d)", quadFaces[quadFaces.size()-1].indices[0], quadFaces[quadFaces.size() - 1].indices[1], quadFaces[quadFaces.size() - 1].indices[2], quadFaces[quadFaces.size() - 1].indices[3]);
			//msgBoxCstring(cStrBuf1);
		}

		//curIndex = tempStr.find("\n", 0);
		//tempStr = tempStr.substr(curIndex + 1);
	}

	//char cStrBuf1[2048];
	sprintf(cStrBuf1, "ObjModel::loadObjFile(): numFaces: %d", numFaces);
	//msgBoxCstring(cStrBuf1);
	sprintf(cStrBuf1, "ObjModel::loadObjFile(): finalVertices.size(): %d", finalVertices.size());
	//msgBoxCstring(cStrBuf1);


	return true;
}

void Mesh::Render() {
	//char cStrBuf1[1024];
	//sprintf(cStrBuf1, "Mesh::Render()");
	//msgBoxCstring(cStrBuf1);
	//================================================================================
	//================================================================================
	// Updating vertex info and uploading to vertex buffer in GPU
	//================================================================================
	//================================================================================
	//finalVertices[0].pos.x += 0.0001;

	Vertex *vList = &(finalVertices[0]);
	int vBufferSize = sizeof(Vertex) * finalVertices.size();

	// (20181111): 여기에서 아래를 꺼두고, Init()에서 한번만 하도록 해야만, memory가
	// 계속 증가되는 에러가 발생하지 않는다.
	//ID3D12Resource* vBufferUploadHeap; // move to global to reuse in Render()

	// create upload heap
	// upload heaps are used to upload data to the GPU. CPU can write to it, GPU can read from it
	// We will upload the vertex buffer using this heap to the default heap
	//HRESULT hr = gd3dApp->device->CreateCommittedResource(
	//	&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // upload heap
	//	D3D12_HEAP_FLAG_NONE, // no flags
	//	&CD3DX12_RESOURCE_DESC::Buffer(vBufferSize), // resource description for a buffer
	//	D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
	//	nullptr,
	//	IID_PPV_ARGS(&vBufferUploadHeap));
	//if (FAILED(hr))
	//{
	//	gd3dApp->Running = false;
	//	return;
	//}
	//vBufferUploadHeap->SetName(L"Vertex Buffer Upload Resource Heap");

	D3D12_SUBRESOURCE_DATA vertexData = {};
	vertexData.pData = reinterpret_cast<BYTE*>(vList); // pointer to our vertex array
	vertexData.RowPitch = vBufferSize; // size of all our triangle vertex data
	vertexData.SlicePitch = vBufferSize; // also the size of our triangle vertex data

										 // we are now creating a command with the command list to copy the data from
										 // the upload heap to the default heap
	UpdateSubresources(gd3dApp->commandList, vertexBuffer, vBufferUploadHeap, 0, 0, 1, &vertexData);
	//================================================================================

	ID3D12DescriptorHeap* descriptorHeaps[] = { mainDescriptorHeap };
	gd3dApp->commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	// set the descriptor table to the descriptor heap (parameter 1, as constant buffer root descriptor is parameter index 0)
	gd3dApp->commandList->SetGraphicsRootDescriptorTable(1, mainDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

	// set cube1's constant buffer
	//gd3dApp->commandList->SetGraphicsRootConstantBufferView(0, constantBufferUploadHeaps[frameIndex]->GetGPUVirtualAddress() + ConstantBufferPerObjectAlignedSize * objIndex);
	//gd3dApp->commandList->RSSetViewports(1, &viewport); // set the viewports
	//gd3dApp->commandList->RSSetScissorRects(1, &scissorRect); // set the scissor rects
	gd3dApp->commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // set the primitive topology

	//commandList->IASetVertexBuffers(0, 1, &vertexBufferViews[objIndex]); // set the vertex buffer (using the vertex buffer view)
	//commandList->IASetIndexBuffer(&indexBufferViews[objIndex]);
	gd3dApp->commandList->IASetVertexBuffers(0, 1, &vertexBufferView); // set the vertex buffer (using the vertex buffer view)
	gd3dApp->commandList->IASetIndexBuffer(&indexBufferView);

	// draw first cube
	gd3dApp->commandList->DrawIndexedInstanced(numIndices, 1, 0, 0, 0);
}

bool Mesh::loadMtlFile(string mtlFilePathWithFilename) {

	char cStrBuf1[2048];
	//sprintf(cStrBuf1, "ObjModel::loadMtlFile(): mtlFilePathWithFilename = %s", mtlFilePathWithFilename.c_str());
	//msgBoxCstring(cStrBuf1);

	ifstream fin;
	fin.open(mtlFilePathWithFilename.c_str());
	if (fin.fail()) {
		MessageBox(NULL, L"Unable to load material file", L"Error", MB_OK | MB_ICONERROR);
		exit(1);
	}

	char buf[1024];

	//================================================================================
	//================================================================================
	// Mtl File Importer
	//================================================================================
	//================================================================================

	//-------------------------------------------------------------------------
	// cut off first 3 lines
	//-------------------------------------------------------------------------

	fin.getline(buf, 1024);
	fin.getline(buf, 1024);
	fin.getline(buf, 1024);

	//-------------------------------------------------------------------------
	// get material name
	//-------------------------------------------------------------------------
	fin.getline(buf, 1024);
	string matStr = string("") + &buf[7];

	//sprintf(cStrBuf1, "matStr: %s", matStr);
	//msgBoxCstring(cStrBuf1);

	//-------------------------------------------------------------------------
	// get specular exponent 
	//-------------------------------------------------------------------------
	fin.getline(buf, 1024);
	string specularExpStr = string("") + buf;
	specularExpStr = specularExpStr.substr(3); // cut off "Ns "
	float specularExp = atof(specularExpStr.c_str());

	//sprintf(cStrBuf1, "specularExp: %f", specularExp);
	//msgBoxCstring(cStrBuf1);

	//-------------------------------------------------------------------------
	// get ambient / diffuse / specular reflectivity 
	//-------------------------------------------------------------------------
	float r[3];
	float g[3];
	float b[3];

	for (int i = 0; i < 3; i++) {
		fin.getline(buf, 1024);
		string tempStr = string("") + buf;

		tempStr = tempStr.substr(3); // cut off tag

		int curIndex = tempStr.find(" ", 0);
		string tempRStr = tempStr.substr(0, curIndex + 1); // red ambient
		r[i] = atof(tempRStr.c_str());

		int prevIndex = curIndex;
		curIndex = tempStr.find(" ", curIndex + 1);

		string tempGStr = tempStr.substr(prevIndex, curIndex + 1); // blue ambient
		g[i] = atof(tempGStr.c_str());

		string tempBStr = tempStr.substr(curIndex + 1); // green ambient
		b[i] = atof(tempBStr.c_str());
	}
	// set ambient
	float ambientR = r[0];
	float ambientG = g[0];
	float ambientB = b[0];

	// set diffuse
	float diffuseR = r[1];
	float diffuseG = g[1];
	float diffuseB = b[1];

	// set specular reflectivity
	float specRefR = r[2];
	float specRefG = g[2];
	float specRefB = b[2];

	//sprintf(cStrBuf1, "ambient: (%f, %f, %f)", ambientR, ambientG, ambientB);
	//msgBoxCstring(cStrBuf1);
	//sprintf(cStrBuf1, "diffuse: (%f, %f, %f)", diffuseR, diffuseG, diffuseB);
	//msgBoxCstring(cStrBuf1);
	//sprintf(cStrBuf1, "specular reflectivity: (%f, %f, %f)", specRefR, specRefG, specRefB);
	//msgBoxCstring(cStrBuf1);

	//-------------------------------------------------------------------------
	// emissive coefficient 
	//-------------------------------------------------------------------------
	fin.getline(buf, 1024);
	// set emissive coefficient
	float emisCoR;
	float emisCoG;
	float emisCoB;
	string tempStr = string("") + buf;

	if (tempStr.find("Ke ", 0) != std::string::npos) {
		tempStr = tempStr.substr(3); // cut off tag

		int curIndex = tempStr.find(" ", 0);
		string tempRStr = tempStr.substr(0, curIndex + 1); // red ambient
		emisCoR = atof(tempRStr.c_str());

		int prevIndex = curIndex;
		curIndex = tempStr.find(" ", curIndex + 1);

		string tempGStr = tempStr.substr(prevIndex, curIndex + 1); // blue ambient
		emisCoG = atof(tempGStr.c_str());

		string tempBStr = tempStr.substr(curIndex + 1); // green ambient
		emisCoB = atof(tempBStr.c_str());

		fin.getline(buf, 1024);

		//sprintf(cStrBuf1, "emissive coefficient: (%f, %f, %f)", emisCoR, emisCoG, emisCoG);
		//msgBoxCstring(cStrBuf1);
	}
	//-------------------------------------------------------------------------
	// get index of refraction
	//-------------------------------------------------------------------------
	if (string(buf).find("Ni ", 0) != std::string::npos) {
		string indexOfRefractionStr = string(buf);
		indexOfRefractionStr = indexOfRefractionStr.substr(3); // cut off "Ni "
		float indexOfRefraction = atof(indexOfRefractionStr.c_str());
		fin.getline(buf, 1024);

		//sprintf(cStrBuf1, "index of refraction: %f", indexOfRefraction);
		//msgBoxCstring(cStrBuf1);
	}

	//-------------------------------------------------------------------------
	// get dissolve
	//-------------------------------------------------------------------------
	string dissolveStr = string("") + buf;
	dissolveStr = dissolveStr.substr(2); // cut off "d "
	float dissolve = atof(dissolveStr.c_str());

	//sprintf(cStrBuf1, "dissolve: %f", dissolve);
	//msgBoxCstring(cStrBuf1);

	//-------------------------------------------------------------------------
	// get illumination model
	//-------------------------------------------------------------------------
	fin.getline(buf, 1024);
	string illumModelStr = string("") + buf;
	illumModelStr = illumModelStr.substr(6); // cut off "illum "
	int illumModel = atoi(illumModelStr.c_str());

	//sprintf(cStrBuf1, "illumination model: %d", illumModel);
	//msgBoxCstring(cStrBuf1);

	//-------------------------------------------------------------------------
	// get image name
	//-------------------------------------------------------------------------
	fin.getline(buf, 1024);
	string imageNameStr = string("") + buf;
	imageNameStr = imageNameStr.substr(7); // cut off "map_kd "

	//sprintf(cStrBuf1, "image name: %s", imageNameStr.c_str());
	//msgBoxCstring(cStrBuf1);
}