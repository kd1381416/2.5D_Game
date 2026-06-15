#pragma once

class BaseObject :public KdGameObject
{
public:
	BaseObject(){}
	virtual ~BaseObject(){}

	void Init()override;
	void Update()override;
	void DrawLit()override;

private:

protected:
};