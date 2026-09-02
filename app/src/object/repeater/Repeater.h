#pragma once
#include <object/baseObject2d/BaseObject2d.h>

class Repeater : public BaseObject2d
{
public:
	Repeater();
	~Repeater() override;

	void Initialize() override;
	void Update() override;
	void Draw() override;
};
