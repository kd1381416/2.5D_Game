#pragma once

class Enemy :public KdGameObject
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

	enum EnemyMove
	{
		Idle,
		Move,
		Attack1,
		Size,
	};

	Enemy() {};
	~Enemy()override {};

	void Init()			override;
	void PreUpdate()	override;
	void Update()		override;
	void PostUpdate()	override;
	void DrawLit()		override;
	void OnHit()		override;

	void GenerateDepthMapFromLight()	override;

	void LMove();
	void LAttack1();

	void SetTarget(std::shared_ptr<KdGameObject> _target) { m_Target = _target; }

private:

	//座標
	Math::Vector3	m_Pos = Math::Vector3::Zero;
	//移動速度
	float	m_Speed = 0;
	//重力
	float	m_Gravity = 0;
	//向き
	int		m_Dir = 0;
	//画像
	std::shared_ptr<KdSquarePolygon>	m_Polygon = nullptr;
	//アニメーション
	AnimationInfo	m_Anime = { 0,0,0,0 };
	//追尾ターゲット
	std::weak_ptr<KdGameObject>		m_Target;
	//ターゲットの位置
	//Math::Vector3	m_TargetPos = Math::Vector3::Zero;
	//今行動中かどうか
	bool	m_MoveFlg = false;
	//行動のリキャストタイム
	int		m_MoveIterval = 0;
	//今の行動
	EnemyMove	m_NowMove = EnemyMove::Idle;
};