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
	m_Polygon->SetMaterial("Asset/Textures/EliteOrc/EliteOrc.png");
	//画像分割
	m_Polygon->SetSplit(11, 7);
	//原点変更
	m_Polygon->SetPivot(KdSquarePolygon::PivotType::Center_Bottom);

//===================================================================
//初期設定 
//===================================================================
	//座標
	m_Pos = { 0,1,0 };
	//移動速度
	m_Speed = 0.05f;
	//画像
	m_Polygon->SetUVRect(0);
	m_Dir = 1;
	//アニメーション
	m_Anime = { 0, 6, 0, 0.2 };

//===================================================================
//コライダー(当たり判定情報)の初期化(登録) 
//===================================================================
	m_pCollider = std::make_unique<KdCollider>();	// 1 生成
	m_pCollider->RegisterCollisionShape				// 2 判定リストに登録
	(
		"Enemy",				// 登録名
		m_Polygon,				// モデルデータ
		KdCollider::TypeDamage	// 判定種類
	);
}

void Enemy::PreUpdate()
{
//===================================================================
// 次の行動を決める
//===================================================================
	if (!m_MoveFlg)
	{
		int	i = KdRandom::GetInt(0, 100);
		switch (i)
		{
		case :
				break;
		default:
			break;
		}
	}
}

void Enemy::Update()
{
//===================================================================
//アニメーション 
//===================================================================
	m_Polygon->SetUVRect(m_Anime.start + m_Anime.count);

	m_Anime.count += m_Anime.speed;

	if (m_Anime.start + m_Anime.count > m_Anime.end) { m_Anime.count = 0; }

//===================================================================
// 追尾処理
//===================================================================
	Math::Vector3	_targetdir = Math::Vector3::Zero;

	_targetdir = m_Target.lock()->GetPos() - m_Pos;

	_targetdir.y = 0;

	float _stopDistance = 1.0f;

	float _distance = _targetdir.Length();

	if (_distance > _stopDistance)
	{
		_targetdir.Normalize();

		float moveSpeed = m_Speed;

		// 停止距離付近で減速
		if (_distance < _stopDistance + 0.3f)
		{
			moveSpeed *= (_distance - _stopDistance) / 0.3f;
		}

		m_Pos += _targetdir * moveSpeed;
	}
	
	//方向転換
	if (_targetdir.x > 0) { m_Dir =  1; }
	if (_targetdir.x < 0) { m_Dir = -1; }

//===================================================================
//重力 
//===================================================================
	m_Pos.y -= m_Gravity;
	m_Gravity += 0.005f;

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
	//ちょっと上からの位置にする
	_ray.m_pos.y += 0.45f;
	//段差の許容範囲
	//float _eneblestephigh = 0.2f;
	//_ray.m_pos.y += _eneblestephigh;
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
		m_Pos = _hitpos += Math::Vector3(0, -0.45, 0);
		m_Gravity = 0;
	}


//===================================================================
//デバック処理
//===================================================================
	//レイ判定
	m_pDebugWire->AddDebugLine(_ray.m_pos, _ray.m_dir, _ray.m_range);

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

void Enemy::OnHit()
{
	m_isExpired = true;
}
