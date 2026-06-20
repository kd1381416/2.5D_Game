#include "GameScene.h"
#include"../SceneManager.h"

#include"../../Object/Ground/Ground.h"	//地面
#include"../../Object/Player/Player.h"	//プレイヤー
#include"../../Object//Enemy/Enemy.h"	//エネミー

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
	Math::Matrix _rotatX = Math::Matrix::CreateRotationX(DirectX::XMConvertToRadians(20));
	//移動
	Math::Matrix _trans = Math::Matrix::CreateTranslation(m_Player->GetPos() + Math::Vector3(0, 1, -1.5));
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

	//===プレイヤー===
	//ポインタ作成
	std::shared_ptr<Player>	_player;
	//実体化
	_player = std::make_shared<Player>();
	//初期化
	_player->Init();
	//GameSceneにプレイヤーの情報を持たせる
	m_Player = _player;
	//オブジェクトリストに追加
	m_objList.push_back(_player);

	//===エネミー===
	//ポインタ作成
	std::shared_ptr<Enemy>	_enemy;
	//実体化
	_enemy = std::make_shared<Enemy>();
	//初期化
	_enemy->Init();
	//ターゲットを設定
	_enemy->SetTarget(m_Player);
	//オブジェクトリストに追加
	m_objList.push_back(_enemy);
}
