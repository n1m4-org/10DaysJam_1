#pragma once
#include <object/baseObject2d/BaseObject2d.h>

class AlumiWall : public BaseObject2d
{
public:
	AlumiWall();
	~AlumiWall() override;

	void Initialize() override;
	void Update() override;
	void Draw() override;
};
