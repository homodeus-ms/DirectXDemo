#pragma once
#include "Component.h"

class Shader;
class Model;
class Material;

class ModelRenderer : public Component
{
	using Super = Component;
public:
	ModelRenderer(shared_ptr<Shader> shader);
	virtual ~ModelRenderer();

	void SetModel(shared_ptr<Model> model);
	void SetRenderPass(uint8 pass) { _renderPass = pass; }

	void RenderInstancing(shared_ptr<class InstancingBuffer>& buffer);

	InstanceID GetInstanceID();

private:
	shared_ptr<Shader> _shader;
	uint8 _renderPass = 0;
	shared_ptr < Model > _model;
};

