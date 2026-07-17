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
	void Update(Starshine::GameTime& gameTime);
	void Draw(Starshine::GameTime& gameTime);

	i64 GetStateID() const;

private:
	struct Impl;
	std::unique_ptr<Impl> impl{};
};
