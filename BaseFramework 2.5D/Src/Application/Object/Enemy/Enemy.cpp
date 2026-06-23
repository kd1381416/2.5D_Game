#include "Enemy.h"

#include"../../Scene/SceneManager.h"

void Enemy::Init()
{
//===================================================================
// デバック用
//===================================================================
	m_pDebugWire = std::make_unique<KdDebugWireFrame>();

//===================================================================
//画像 
//===================================================================
	//実体化
	m_Polygon = std::make_shared<KdSquarePolygon>();
	//画像読み込み
	m_Polygon->SetMaterial("Asset/Textures/Dragon/Dragon.png");
	//画像分割
	m_Polygon->SetSplit(17, 6);
	//原点変更
	m_Polygon->SetPivot(KdSquarePolygon::PivotType::Center_Bottom);
	m_Polygon->SetScale(2);

	m_HpBar = std::make_shared<KdTexture>();
	m_HpBar->Load("Asset/Textures/System/EnemyHp.png");
	

//===================================================================
//初期設定 
//===================================================================
	//座標
	m_Pos = { 0,1.5,0 };
	//移動速度
	m_Speed = 0.05f;
	//画像
	m_Polygon->SetUVRect(0);
	m_Dir = 1;
	//アニメーション
	m_Anime = { 0, 8, 0, 0.2 };
	//体力
	m_Hp = 800;
	//攻撃力
	m_AttackNum = 10;

//===================================================================
//コライダー(当たり判定情報)の初期化(登録) 
//===================================================================
	m_pCollider = std::make_unique<KdCollider>();	// 1 生成
	m_pCollider->RegisterCollisionShape				// 2 判定リストに登録
	(
		"Enemy",				// 登録名
		{0,1,0},
		0.6f,
		KdCollider::TypeDamage	// 判定種類
	);
}

void Enemy::PreUpdate()
{
//===================================================================
// 次の行動を決める
//===================================================================
	if (m_MoveFlg)
	{
		int	i = KdRandom::GetInt(0, EnemyMove::Size);

		switch (i)
		{
		case EnemyMove::Idle:
			m_NowMove = EnemyMove::Idle;
			m_Anime = { 0,8,0,0.2 };
			m_MoveTime = 50;
			m_MoveIterval = 100;
			break;
		case EnemyMove::Move:
			m_NowMove = EnemyMove::Move;
			m_Anime = { 17,24,0,0.2 };
			m_MoveTime = 50;
			m_MoveIterval = 100;
			break;
		case EnemyMove::Attack1:
			m_NowMove = EnemyMove::Attack1;
			m_Anime = { 34,47,0,0.2 };
			m_MoveIterval = 100;
			m_MoveTime = 100;
			break;
		case EnemyMove::Attack2:
			m_NowMove = EnemyMove::Attack2;
			m_Anime = { 51,67,0,0.2 };
			m_MoveTime = 100;
			m_MoveIterval = 100;
		}

		m_MoveFlg = false;

	}

	if (m_DeathFlg)
	{
		m_NowMove = EnemyMove::Death;
		m_Anime.start = 85;
		m_Anime.end = 91;
	}
}

void Enemy::Update()
{
//===================================================================
//行動
//===================================================================
	if(m_MoveTime > 0)
	{
		switch (m_NowMove)
		{
		case EnemyMove::Idle:
			break;
		case EnemyMove::Move:
			EMove();
			break;
		case EnemyMove::Attack1:
			EAttack1();
			break;
		case EnemyMove::Attack2:
			EAttack2();
			break;
		default:
			break;
		}

		--m_MoveTime;
	}
	else
	{
		--m_MoveIterval;

		if (m_MoveIterval <= 0)
		{
			m_MoveFlg = true;	
		}
	}

//===================================================================
//アニメーション 
//===================================================================
	m_Polygon->SetUVRect(m_Anime.start + m_Anime.count);

	m_Anime.count += m_Anime.speed;

	if (m_Anime.start + m_Anime.count >= m_Anime.end) 
	{
		if (m_NowMove == EnemyMove::Attack1) { m_MoveFlg = true; }
		m_Anime.count = 0; 
	}

//===================================================================
//プレイヤーの向き
//===================================================================
	if(m_NowMove != EnemyMove::Attack1 && m_NowMove != EnemyMove::Attack2)
	{
		Math::Vector3 _targetdir = Math::Vector3::Zero;
		_targetdir = m_Target.lock()->GetPos() - m_Pos;
		//方向転換
		if (_targetdir.x > 0) { m_Dir = -1; }
		if (_targetdir.x < 0) { m_Dir =  1; }
	}

//===================================================================
//重力 
//===================================================================
	m_Pos.y -= m_Gravity;
	m_Gravity += 0.005f;

	--m_MinusHpInterval;
}

void Enemy::PostUpdate()
{
//===================================================================
//当たり判定(レイ(光線)判定)
//===================================================================
	//当たる側(加害者側(レイを出す側))
	KdCollider::RayInfo _ray;
	//レイの発射位置を測定
	_ray.m_pos = m_Pos;
	//段差の許容範囲
	float _eneblestephigh = 0.35f;
	_ray.m_pos.y += _eneblestephigh;
	//レイの発射方向を設定
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
		m_Pos = _hitpos -= Math::Vector3(0, _eneblestephigh, 0);
		m_Gravity = 0;
	}


//===================================================================
//デバック処理
//===================================================================
	//レイ判定
	m_pDebugWire->AddDebugLine(_ray.m_pos, _ray.m_dir, _ray.m_range);
	m_pDebugWire->AddDebugSphere(m_Pos + Math::Vector3{ 0,1,0 }, 0.6f, kRedColor);

//===================================================================
//行列作成 
//===================================================================
	//移動
	Math::Matrix _trans = Math::Matrix::CreateTranslation(m_Pos);
	//拡縮
	Math::Matrix _scale = Math::Matrix::CreateScale(m_Dir, 1, 1);
	//合成
	m_mWorld = _scale * _trans;

}

void Enemy::DrawLit()
{
//===================================================================
// 描画
//===================================================================
	KdShaderManager::Instance().m_StandardShader.DrawPolygon(*m_Polygon, m_mWorld);
}

void Enemy::DrawSprite()
{
	Math::Rectangle	rec = { 0,0,128,32 };
	Math::Color	color = { 1,1,1,0.5 };
	KdShaderManager::Instance().m_spriteShader.DrawTex(m_HpBar, -400, 300, m_Hp, 50, &rec, &color, Math::Vector2{ 0.0,0.5 });
}

void Enemy::OnHit()
{
	if(m_MinusHpInterval<=0)
	{
		m_Hp -= 100;
		m_MinusHpInterval=100;
	}
	
	if (m_Hp <= 0)
	{
		m_DeathFlg = true;
	}
}

void Enemy::GenerateDepthMapFromLight()
{
	KdShaderManager::Instance().m_StandardShader.DrawPolygon(*m_Polygon, m_mWorld);
}

void Enemy::EMove()
{
//===================================================================
// 追尾処理
//===================================================================
	if (m_NowMove != EnemyMove::Move || m_MoveFlg)return;

	Math::Vector3 _targetdir = Math::Vector3::Zero;

	_targetdir = m_Target.lock()->GetPos() - m_Pos;

	_targetdir.y = 0;

	float _stopDistance = 1.0f;

	float _distance = _targetdir.Length();

	if (_distance > _stopDistance)
	{
		_targetdir.Normalize();

		float moveSpeed = m_Speed;

		// 停止距離付近で減速
		if (_distance < _stopDistance + 0.1f)
		{
			moveSpeed *= (_distance - _stopDistance) / 0.1f;
		}

		m_Pos += _targetdir * moveSpeed;
	}
}

void Enemy::EAttack1()
{
	if (m_NowMove != EnemyMove::Attack1 || m_MoveFlg)return;

//===================================================================
//当たり判定(球(スフィア)判定)
//===================================================================
	if(m_Anime.start+m_Anime.count >= 42 && m_Anime.start+m_Anime.count <= 45)
	{
		//球判定用の変数を用意
		KdCollider::SphereInfo _sphere;
		//球の中心座標を設定
		_sphere.m_sphere.Center = m_Pos;
		_sphere.m_sphere.Center.y += 0.8f;
		switch (m_Dir)
		{
		case 1:
			_sphere.m_sphere.Center.x += -0.6f;
			break;
		case-1:
			_sphere.m_sphere.Center.x += 0.6f;
			break;
		}
		//球の半径設定
		_sphere.m_sphere.Radius = 0.3f;
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
		float _maxoverlap = 0;			//←使いまわし
		bool  _hit = false;				//←使いまわし
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
		}

		//球判定
		m_pDebugWire->AddDebugSphere(_sphere.m_sphere.Center, _sphere.m_sphere.Radius);
	}

}

void Enemy::EAttack2()
{
	if (m_NowMove != EnemyMove::Attack2 || m_MoveFlg)return;

	if (m_Anime.start + m_Anime.count >= 59 && m_Anime.start + m_Anime.count <= 64)
	{
		//球判定用の変数を用意
		KdCollider::SphereInfo _sphere;
		//球の中心座標を設定
		_sphere.m_sphere.Center = m_Pos;
		_sphere.m_sphere.Center.y += 0.5f;
		switch (m_Dir)
		{
		case 1:
			_sphere.m_sphere.Center.x += -0.7f;
			break;
		case-1:
			_sphere.m_sphere.Center.x += 0.7f;
			break;
		}
		//球の半径設定
		_sphere.m_sphere.Radius = 0.4f;
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
		float _maxoverlap = 0;			//←使いまわし
		bool  _hit = false;				//←使いまわし
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
		}

		//球判定
		m_pDebugWire->AddDebugSphere(_sphere.m_sphere.Center, _sphere.m_sphere.Radius);

	}
}