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
	//実体化
	m_Polygon = std::make_shared<KdSquarePolygon>();
	//画像読み込み
	m_Polygon->SetMaterial("Asset/Textures/Player/Up/Up.png");
	//画像分割
	m_Polygon->SetSplit(8, 4);
	//初期画像
	m_Polygon->SetUVRect(1);
	//原点を変更
	m_Polygon->SetPivot(KdSquarePolygon::PivotType::Center_Bottom);

//===================================================================
// 初期設定
//===================================================================
	m_Pos = { 0,1,0 };	//初期座標
	m_Speed = 0.06f;	//移動速度
	m_Gravity = 0;		//重力

	m_Anime = { 0,8,0,0.2 };
}

void Player::Update()
{
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
		}
		if (GetAsyncKeyState('S') & 0x8000) { _movedir += {	0, 0, -1 }; }
		if (GetAsyncKeyState('A') & 0x8000) { _movedir += {-1, 0, 0 }; }
		if (GetAsyncKeyState('D') & 0x8000) { _movedir += {	1, 0, 0 }; }

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
				//m_AttackInterval = 0;
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

	++m_AttackInterval;
	if (m_AttackInterval >= 200) { m_Attack2Flg = false; }

//===================================================================
//アニメーション
//===================================================================	
	m_Polygon->SetUVRect(m_Anime.start + (int)m_Anime.count);

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
	//ちょっと上からの位置にする
	_ray.m_pos.y += 0.1f;
	//段差の許容範囲
	//float _eneblestephigh = 0.2f;
	//_ray.m_pos.y += _eneblestephigh;
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
		m_Pos = _hitpos += Math::Vector3(0, -0.1f, 0);
		m_Gravity = 0;
	}


//===================================================================
//デバック処理
//===================================================================
	//レイ判定
	m_pDebugWire->AddDebugLine(_ray.m_pos, _ray.m_dir, _ray.m_range);

//===================================================================
//行列 
//===================================================================
	//拡縮
	Math::Matrix _scale = Math::Matrix::CreateScale(1);
	//移動
	Math::Matrix _trans = Math::Matrix::CreateTranslation(m_Pos);
	//合成
	m_mWorld = _scale * _trans;
}

void Player::DrawLit()
{
	KdShaderManager::Instance().m_StandardShader.DrawPolygon(*m_Polygon, m_mWorld);
}