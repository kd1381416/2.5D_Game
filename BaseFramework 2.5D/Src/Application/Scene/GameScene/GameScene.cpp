#include "GameScene.h"
#include"../SceneManager.h"

#include"../../Object/Ground/Ground.h"	//地面

void GameScene::Event()
{
	if (GetAsyncKeyState('T') & 0x8000)
	{
		SceneManager::Instance().SetNextScene
		(
			SceneManager::SceneType::Title
		);
	}
//===================================================================
// 行列
//===================================================================
	//===カメラ===
	// 回転
	Math::Matrix _rotatX = Math::Matrix::CreateRotationX(DirectX::XMConvertToRadians(30));
	//移動
	Math::Matrix _trans = Math::Matrix::CreateTranslation(0, 5, -5);
	//合成
	m_camera->SetCameraMatrix(_rotatX * _trans);
}

void GameScene::Init()
{
//===================================================================
// オブジェクト追加
//===================================================================
	//===カメラ===
	m_camera = std::make_unique<KdCamera>();

	//===地面===
	//ポインタ作成
	std::shared_ptr<Ground>	_ground;
	//実体化
	_ground = std::make_shared<Ground>();
	//初期化
	_ground->Init();
	//オブジェクトリストに追加
	m_objList.push_back(_ground);
}
