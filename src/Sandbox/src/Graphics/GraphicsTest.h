#pragma once
#include "GameInstance.h"
#include "Rendering/Device.h"

class TestState : public Starshine::GameState
{
public:
	TestState() {};

public:
	bool Initialize();
	bool LoadContent();
	void UnloadContent();
	void Destroy();
	void Update(f64 deltaTime_milliseconds);
	void Draw(f64 deltaTime_milliseconds);

	std::string_view GetStateName() const;

private:
	f64 elapsedTime{};

	Starshine::Rendering::Device* GFXDevice{};
	std::unique_ptr<Starshine::Rendering::VertexBuffer> vertexBuffer{};
	std::unique_ptr<Starshine::Rendering::IndexBuffer> indexBuffer{};
	std::unique_ptr<Starshine::Rendering::VertexDesc> vertexDesc{};
	std::unique_ptr<Starshine::Rendering::Shader> testShader{};
	std::unique_ptr<Starshine::Rendering::Texture> testTexture{};

	mat4 transformMatrix{};
};
