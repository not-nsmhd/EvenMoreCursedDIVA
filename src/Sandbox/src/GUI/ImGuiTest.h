#pragma once
#include "GameInstance.h"
#include "Rendering/Device.h"

class ImGuiTest : public Starshine::GameState
{
public:
	ImGuiTest();
	~ImGuiTest();

public:
	bool Initialize();
	bool LoadContent();
	void UnloadContent();
	void Destroy();
	void Update(Starshine::GameTime& gameTime);
	void Draw(Starshine::GameTime& gameTime);

	std::string_view GetStateName() const;

private:
	struct Impl;
	std::unique_ptr<Impl> impl{};
};
