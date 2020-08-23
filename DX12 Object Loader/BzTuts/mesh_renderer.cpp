#include "mesh_renderer.h"
#include "renderable.h"

MeshRenderer::MeshRenderer() : Component() {
}

MeshRenderer::~MeshRenderer() {
}

void MeshRenderer::Render() {
	renderable->Render();
}
