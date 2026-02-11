#pragma once
#include "GameInstance.h"
#include "Rendering/Device.h"

class SpriteRendererTest : public Starshine::GameState
{
public:
	SpriteRendererTest();
	~SpriteRendererTest();

public:
	bool Initialize();
	bool LoadContent();
	void UnloadContent();
	void Destroy();
	void Update(f64 deltaTime_milliseconds);
	void Draw(f64 deltaTime_milliseconds);

	std::string_view GetStateName() const;

private:
	struct Impl;
	std::unique_ptr<Impl> impl{};
};
