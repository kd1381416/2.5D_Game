#pragma once

class Player :public KdGameObject
{
public:
	// アニメーション情報
	struct AnimationInfo
	{
		int   start;	// 開始コマ
		int   end;		// 終了コマ
		float count;	// 現在のカウント数
		float speed;	// アニメーションの速度
	};

	enum PlayerDir
	{
		Up,
		Down,
		Right,
		Left,
		Size
	};

	Player() {};
	~Player()override {};

	void Init()			override;
	void Update()		override;
	void PostUpdate()	override;
	void DrawLit()		override;

private:

	//向いている向き
	PlayerDir	m_NowDir;
	//画像
	std::shared_ptr<KdSquarePolygon>	m_Polygon[PlayerDir::Size];
	//座標
	Math::Vector3	m_Pos = Math::Vector3::Zero;
	//移動速度
	float	m_Speed = 0;
	//重力
	float	m_Gravity = 0;
	//アニメーション用
	AnimationInfo	m_Anime;
	//攻撃
	bool	m_AttackFlg = false;
	//攻撃二段目
	bool	m_Attack2Flg = false;
	int		m_AttackInterval;
};