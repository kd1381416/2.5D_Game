#pragma once

class Player :public KdGameObject
{
public:
	Player() {};
	~Player()override {};

	void Init()		override;
	void Update()	override;
	void DrawLit()	override;

private:

	//画像
	std::shared_ptr<KdPolygon>	m_Polugon;
	//座標
	Math::Vector3	m_Pos = Math::Vector3::Zero;
};