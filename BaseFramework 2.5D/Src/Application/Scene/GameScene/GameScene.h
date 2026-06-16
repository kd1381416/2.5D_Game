#pragma once

#include"../BaseScene/BaseScene.h"

class GameScene : public BaseScene
{
public :

	GameScene()  { Init(); }
	~GameScene() {}

private:

	void Event() override;
	void Init()  override;

	//プレイヤーの情報を持つ
	std::shared_ptr<KdGameObject>	m_Player;
};
