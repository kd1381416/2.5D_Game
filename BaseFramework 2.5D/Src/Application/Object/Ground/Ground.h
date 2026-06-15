#pragma once

class Ground : public KdGameObject
{
public:
	Ground(){}
	~Ground()override{}

	void Init()		override;
	void Update()	override;
	void DrawLit()	override;

private:

	//モデル
	std::shared_ptr<KdModelData>	m_Model;	
	//座標
	Math::Vector3	m_Pos = Math::Vector3::Zero;
	//スケール
	float			m_Scale;		
};