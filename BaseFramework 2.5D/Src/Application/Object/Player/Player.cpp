#include "Player.h"

#include"../../Scene/SceneManager.h"

void Player::Init()
{
//===================================================================
// デバック用
//===================================================================
	m_pDebugWire = std::make_unique<KdDebugWireFrame>();

//===================================================================
// 画像
//===================================================================
	for (int i = 0; i < PlayerDir::Size; i++)
	{
		//実体化
		m_Polygon[i] = std::make_shared<KdSquarePolygon>();
		//画像読み込み
		switch (i)
		{
		case PlayerDir::Up:
			m_Polygon[i]->SetMaterial("Asset/Textures/Player/Up/Up.png");
			break;
		case PlayerDir::Down:
			m_Polygon[i]->SetMaterial("Asset/Textures/Player/Down/Down.png");
			break;
		case PlayerDir::Right:
			m_Polygon[i]->SetMaterial("Asset/Textures/Player/Right/Right.png");
			break;
		case PlayerDir::Left:
			m_Polygon[i]->SetMaterial("Asset/Textures/Player/Left/Left.png");
			break;
		default:
			break;
		}
		//画像分割
		m_Polygon[i]->SetSplit(8, 4);
		//初期画像
		m_Polygon[i]->SetUVRect(1);
		//原点を変更
		m_Polygon[i]->SetPivot(KdSquarePolygon::PivotType::Center_Bottom);
	}

	m_HpTex = std::make_shared<KdTexture>();
	m_HpTex->Load("Asset/Textures/System/PlayerHp.png");

//===================================================================
// 初期設定
//===================================================================
	m_Pos = { -2,1,0 };	//初期座標
	m_Speed = 0.06f;	//移動速度
	m_Gravity = 0;		//重力
	m_NowDir = PlayerDir::Right;
	m_Anime = { 0,8,0,0.2 };
	m_Hp = 100;
	m_AttackNum = 50;
}

void Player::Update()
{
//===================================================================
// デバック用
//===================================================================
	if (GetAsyncKeyState('R') & 0x8000) { Init(); }

//===================================================================
// 初期化
//===================================================================
	//プレイヤーが何もしていなかったらIdle状態にする
	if(!m_AttackFlg)
	{
		m_Anime.start = 0;
		m_Anime.speed = 0.2;
	}

//===================================================================
// 移動処理
//===================================================================
	Math::Vector3 _movedir = Math::Vector3::Zero;

	if(!m_AttackFlg)
	{
		if (GetAsyncKeyState('W') & 0x8000)
		{
			_movedir += {0, 0, 1 };
			m_Anime.start = 8;
			m_NowDir = PlayerDir::Up;
		}
		if (GetAsyncKeyState('S') & 0x8000)
		{
			_movedir += {0, 0,-1 }; 
			m_Anime.start = 8;
			m_NowDir = PlayerDir::Down;
		}
		if (GetAsyncKeyState('A') & 0x8000)
		{
			_movedir += {-1, 0, 0 }; 
			m_Anime.start = 8;
			m_NowDir = PlayerDir::Left;
		}
		if (GetAsyncKeyState('D') & 0x8000) 
		{
			_movedir += {1, 0, 0 };
			m_Anime.start = 8;
			m_NowDir = PlayerDir::Right;
		}

		_movedir.Normalize();
		m_Pos += _movedir * m_Speed;
	}

//===================================================================
// 攻撃
//===================================================================
	if (GetAsyncKeyState(VK_LBUTTON) & 0x8000)
	{
		if (!m_AttackFlg)
		{
			m_AttackFlg = true;

			if (m_AttackInterval <= 10 && m_Attack2Flg)
			{
				m_Anime.start = 24;
				m_Anime.count = 0;
				m_Anime.speed = 0.4;
				m_Attack2Flg = false;
			}
			else
			{
				m_Anime.start = 16;
				m_Anime.count = 0;
				m_Anime.speed = 0.4;
				m_Attack2Flg = true;
			}
		}
	}

	//当たり判定
	if (m_AttackFlg)
	{
		if (m_Anime.count > 1 && m_Anime.count < 3)
		{
			AttackHit();
		}
	}

	++m_AttackInterval;
	if (m_AttackInterval >= 200) { m_Attack2Flg = false; }

//===================================================================
//アニメーション
//===================================================================	
	m_Polygon[m_NowDir]->SetUVRect(m_Anime.start + (int)m_Anime.count);

	m_Anime.count += m_Anime.speed;

	if (m_Anime.count > m_Anime.end	) 
	{
		m_Anime.count = 0; 
		if (m_AttackFlg) 
		{ 
			m_AttackFlg = false;
			m_AttackInterval = 0;
		}
	}

//===================================================================
//重力 
//===================================================================
	m_Pos.y -= m_Gravity;
	m_Gravity += 0.005f;

}

void Player::PostUpdate()
{
//===================================================================
//当たり判定(レイ(光線)判定)
//===================================================================
	//当たる側(加害者側(レイを出す側))
	KdCollider::RayInfo _ray;
	//レイの発射位置を測定
	_ray.m_pos = m_Pos;
	//段差の許容範囲
	float _eneblestephigh = 0.3f;
	_ray.m_pos.y += _eneblestephigh;
	//レイの発射位置を設定
	_ray.m_dir = { 0,-1,0 };
	//レイの長さを設定
	_ray.m_range = m_Gravity;
	//当たり判定を行いたいタイプを設定
	_ray.m_type = KdCollider::TypeGround;

	//_rayに当たったオブジェクト情報を格納するリスト
	std::list<KdCollider::CollisionResult> _retraylist;
	//当たり判定(全オブジェクト)
	for (auto& obj : SceneManager::Instance().GetObjList())
	{
		//全オブジェクトに対してレイ判定をする関数を呼び出す
		obj->Intersects(_ray, &_retraylist);
	}

	//_retraylistから一番近いオブジェクトを探す
	float _maxoverlap = 0;
	Math::Vector3 _hitpos;
	bool _hit = false;

	//レイを遮断しオーバーした長さが一番長いものを探す
	for (auto& ret : _retraylist)
	{
		if (_maxoverlap < ret.m_overlapDistance)
		{
			//更新
			_maxoverlap = ret.m_overlapDistance;
			_hitpos = ret.m_hitPos;
			_hit = true;
		}
	}

	if (_hit)
	{
		//当たっていたらその座標をプレイヤー座標にセット
		m_Pos = _hitpos += Math::Vector3(0, -(_eneblestephigh), 0);
		m_Gravity = 0;
	}

//===================================================================
//当たり判定(球(スフィア)判定)
//===================================================================
	//球判定用の変数を用意
	KdCollider::SphereInfo _sphere;
	//球の中心座標を設定
	_sphere.m_sphere.Center = m_Pos;
	_sphere.m_sphere.Center.y += 0.5f;
	//球の半径設定
	_sphere.m_sphere.Radius = 0.2f;
	//当たり判定をしたいTypeを設定
	_sphere.m_type = KdCollider::TypeGround;
	//球に当たったオブジェクトの情報を格納するリスト
	std::list<KdCollider::CollisionResult>	_resultspherelist;
	//当たり判定(全オブジェクト)
	for (auto& obj : SceneManager::Instance().GetObjList())
	{
		//全オブジェクトに対して球判定をする関数を呼び出す
		obj->Intersects(_sphere, &_resultspherelist);
	}
	//球に当たったリストから一番近いオブジェクトを探す
	_maxoverlap = 0;			//←使いまわし
	_hit = false;				//←使いまわし
	Math::Vector3	_hitdir;	//当たった方向

	for (auto& _result : _resultspherelist)
	{
		//球にめり込んだ長さが一番長いものを探す
		if (_maxoverlap < _result.m_overlapDistance)
		{
			//更新
			_maxoverlap = _result.m_overlapDistance;
			_hitdir = _result.m_hitDir;
			_hit = true;
		}
	}

	if (_hit)
	{
		//Z方向への押し戻しを無効にする
		_hitdir.z = 0;
		//※方向ベクトルは絶対に長さ「1」
		//正規化(長さは１)
		_hitdir.Normalize();

		//押し戻し処理
		m_Pos += _hitdir * _maxoverlap;
	}

//===================================================================
//デバック処理
//===================================================================
	//レイ判定
	m_pDebugWire->AddDebugLine(_ray.m_pos, _ray.m_dir, _ray.m_range);
	//球判定
	m_pDebugWire->AddDebugSphere(_sphere.m_sphere.Center, _sphere.m_sphere.Radius);

//===================================================================
//行列 
//===================================================================
	//拡縮
	Math::Matrix _scale = Math::Matrix::CreateScale(1.0f);
	//移動
	Math::Matrix _trans = Math::Matrix::CreateTranslation(m_Pos);
	//合成
	m_mWorld = _scale * _trans;
}

void Player::DrawLit()
{
	KdShaderManager::Instance().m_StandardShader.DrawPolygon(*m_Polygon[m_NowDir], m_mWorld);
}

void Player::DrawSprite()
{
	Math::Rectangle	rec = { 0,0,128,32 };
	Math::Color	color = { 1,1,1,0.5 };
	KdShaderManager::Instance().m_spriteShader.DrawTex(m_HpTex, -600, -300, m_Hp * 3, 50, &rec, &color, Math::Vector2{ 0.0,0.5 });
}

void Player::GenerateDepthMapFromLight()
{
	KdShaderManager::Instance().m_StandardShader.DrawPolygon(*m_Polygon[m_NowDir], m_mWorld);
}

void Player::AttackHit()
{
//===================================================================
//当たり判定(球(スフィア)判定)
//===================================================================
	// 球判定用の変数作成
	KdCollider::SphereInfo sphereInfo;

	// 球の中心位置を設定
	Math::Vector3 _attackdir;
	switch (m_NowDir)
	{
	case Player::Up:
		_attackdir = { 0.0f,0.5f,0.3f };
		break;
	case Player::Down:
		_attackdir = { 0.0f,0.5f,-0.3f };
		break;
	case Player::Right:
		_attackdir = { 0.3f,0.5f, 0.0f };
		break;
	case Player::Left:
		_attackdir = { -0.3f,0.5f,0.0f };
		break;
	default:
		break;
	}
	
	sphereInfo.m_sphere.Center = GetPos() + _attackdir;

	// 球の半径を設定
	sphereInfo.m_sphere.Radius = 0.15f;

	// 当たり判定をしたいタイプを設定
	sphereInfo.m_type = KdCollider::TypeDamage;

	// デバッグ用
	m_pDebugWire->AddDebugSphere
	(
		sphereInfo.m_sphere.Center,
		sphereInfo.m_sphere.Radius,
		kRedColor
	);

	// 球情報と当たり判定
	bool hit = false;

	for (auto& obj : SceneManager::Instance().GetObjList())
	{
		// 戻り値 … 当たっていたら true
		hit = obj->Intersects(sphereInfo, nullptr);

		// 攻撃が当たった場合
		if (hit)
		{
			// Hit時の相手オブジェクトの処理
			obj->OnHit();

			// Hit時の自分の処理
			//OnHit();

			// ループから抜ける
			break;
		}
	}
}
