#pragma once
#include <object/baseObject2d/BaseObject2d.h>

class Router : public BaseObject2d
{
public:
	Router();
	~Router() override;

	void Initialize() override;
	void Update() override;
	void Draw() override;
};
