/**
* @file Main.cpp
*/
#include "GLFWEW.h"
#include "Texture.h"
#include "Sprite.h"
#include "Font.h"
#include "TiledMap.h"
#include <random>

const char windowTitle[] = "OpenGL2D 2018"; // タイトルバーに表示される文章.
const int windowWidth = 800; // ウィンドウの描画領域の幅.
const int windowHeight = 600; // ウィンドウの描画領域の高さ.

std::mt19937 random; // 乱数を発生させる変数(乱数エンジン).

/**
* ゲームキャラクター構造体.
*/
struct Actor
{
	Sprite spr; // 画像表示用スプライト.
	Rect collisionShape; // 衝突判定の位置と大きさ.
	int health; // 耐久力(0以下なら破壊されている).
};

SpriteRenderer renderer; // スプライトを描画するオブジェクト.
FontRenderer fontRenderer; // フォント描画用変数.
Sprite sprBackground; // 背景用スプライト.
Sprite sprPlayer;     // 自機用スプライト.

glm::vec3 playerVelocity; // 自機の移動速度.

Actor enemyList[128]; // 敵のリスト.
Actor playerBulletList[128]; // 自機の弾のリスト.
Actor effectList[128]; // 爆発などの特殊効果用スプライトのリスト.
float enemyGenerationTimer; // 次の敵が出現するまでの時間(単位:秒).
int score; // プレイヤーの得点.

// 敵の出現を制御するためのデータ.
TiledMap enemyMap;
float mapCurrentPosX;
float mapProcessedX;

// 敵のアニメーション.
const FrameAnimation::KeyFrame enemyKeyFrames[] = {
	{ 0.000f, glm::vec2(480, 0), glm::vec2(32, 32) },
	{ 0.125f, glm::vec2(480, 96), glm::vec2(32, 32) },
	{ 0.250f, glm::vec2(480, 64), glm::vec2(32, 32) },
	{ 0.375f, glm::vec2(480, 32), glm::vec2(32, 32) },
	{ 0.500f, glm::vec2(480, 0), glm::vec2(32, 32) },
};
FrameAnimation::TimelinePtr tlEnemy;

// 自機のアニメーション.
const FrameAnimation::KeyFrame playerKeyFrames[] = {
	{ 0.000f, glm::vec2(0, 0), glm::vec2(64, 32) },
	{ 0.125f, glm::vec2(0, 32), glm::vec2(64, 32) },
	{ 0.250f, glm::vec2(0, 64), glm::vec2(64, 32) },
	{ 0.375f, glm::vec2(0, 0), glm::vec2(64, 32) },
};
FrameAnimation::TimelinePtr tlPlayer;

/*
* プロトタイプ宣言.
*/
void processInput(GLFWEW::WindowRef);
void update(GLFWEW::WindowRef);
void render(GLFWEW::WindowRef);
bool detectCollision(const Rect*, const Rect*);
void initializeActorList(Actor*, Actor*);
void updateActorList(Actor*, Actor*, float);
void renderActorList(const Actor*, const Actor*);
Actor* findAvailableActor(Actor*, Actor*);
using CollisionHandlerType = void(*)(Actor*, Actor*); // 衝突処理関数の型.
void detectCollision(Actor*, Actor*, Actor*, Actor*, CollisionHandlerType);
void playerBulletAndEnemyContactHandler(Actor*, Actor*);

/**
* プログラムのエントリーポイント.
*/
int main()
{
	// アプリケーションの初期化.
	GLFWEW::WindowRef window = GLFWEW::Window::Instance();
	if (!window.Initialize(windowWidth, windowHeight, windowTitle)) {
		return 1;
	}
	if (!Texture::Initialize()) {
		return 1;
	}
	if (!renderer.Initialize(1024)) {
		return 1;
	}
	if (!fontRenderer.Initialize(1024, glm::vec2(windowWidth, windowHeight))) {
		return 1;
	}
	if (!fontRenderer.LoadFromFile("Res/Font/makinas_scrap.fnt")) {
		return 1;
	}

	random.seed(std::random_device()()); // 乱数エンジンの初期化.

	// アニメーション・タイムラインの作成.
	tlEnemy = FrameAnimation::Timeline::Create(enemyKeyFrames);
	tlPlayer = FrameAnimation::Timeline::Create(playerKeyFrames);

	//スプライトに画像を設定.
	sprBackground = Sprite("Res/UnknownPlanet.png");
	sprPlayer = Sprite("Res/Objects.png", glm::vec3(0, 0, 0), Rect(0, 0, 64, 32));
	sprPlayer.Animator(FrameAnimation::Animate::Create(tlPlayer));

	initializeActorList(std::begin(enemyList), std::end(enemyList));
	initializeActorList(std::begin(playerBulletList), std::end(playerBulletList));
	initializeActorList(std::begin(effectList), std::end(effectList));

	enemyGenerationTimer = 2;
	score = 0;

	// 敵配置マップを読み込む.
	enemyMap.Load("Res/EnemyMap.json");
	mapCurrentPosX = windowWidth;
	mapProcessedX = windowWidth;

	// ゲームループ.
	while (!window.ShouldClose()) {
		processInput(window);
		update(window);
		render(window);
	}

	Texture::Finalize();
	return 0;
}

/**
* プレイヤーの入力を処理する.
*
* @param window ゲームを管理するウィンドウ.
*/
void processInput(GLFWEW::WindowRef window)
{
	window.Update();

	// 自機の速度を設定する.
	const GamePad gamepad = window.GetGamePad();
	if (gamepad.buttons & GamePad::DPAD_UP) {
		playerVelocity.y = 1;
	} else if (gamepad.buttons & GamePad::DPAD_DOWN) {
		playerVelocity.y = -1;
	} else {
		playerVelocity.y = 0;
	}
	if (gamepad.buttons & GamePad::DPAD_RIGHT) {
		playerVelocity.x = 1;
	} else if (gamepad.buttons & GamePad::DPAD_LEFT) {
		playerVelocity.x = -1;
	} else {
		playerVelocity.x = 0;
	}
	if (playerVelocity.x || playerVelocity.y) {
		playerVelocity = glm::normalize(playerVelocity) * 400.0f;
	}

	// 弾の発射.
	if (gamepad.buttonDown & GamePad::A) {
		Actor* bullet = findAvailableActor(std::begin(playerBulletList), std::end(playerBulletList));
		// 空いている構造体が見つかったら、それを使って弾を発射する.
		if (bullet != nullptr) {
			bullet->spr = Sprite("Res/Objects.png", sprPlayer.Position(), Rect(64, 0, 32, 16));
			bullet->spr.Tweener(TweenAnimation::Animate::Create(
				TweenAnimation::MoveBy::Create(1, glm::vec3(1200, 0, 0),
				TweenAnimation::EasingType::Linear)));
			bullet->collisionShape = Rect(-8, -4, 16, 8);
			bullet->health = 4;
		}
	}
}

/**
* ゲームの状態を更新する.
*
* @param window ゲームを管理するウィンドウ.
*/
void update(GLFWEW::WindowRef window)
{
	const float deltaTime = window.DeltaTime(); // 前回の更新からの経過時間(秒).

	// 自機の移動.
	if (playerVelocity.x || playerVelocity.y) {
		glm::vec3 newPos = sprPlayer.Position() + playerVelocity * deltaTime;
		// 自機の移動範囲を画面内に制限する.
		const Rect playerRect = sprPlayer.Rectangle();
		if (newPos.x < -0.5f * (windowWidth - playerRect.size.x)) {
			newPos.x = -0.5f * (windowWidth - playerRect.size.x);
		} else if (newPos.x > 0.5f * (windowWidth - playerRect.size.x)) {
			newPos.x = 0.5f * (windowWidth - playerRect.size.x);
		}
		if (newPos.y < -0.5f * (windowHeight - playerRect.size.y)) {
			newPos.y = -0.5f * (windowHeight - playerRect.size.y);
		} else if (newPos.y > 0.5f * (windowHeight - playerRect.size.y)) {
			newPos.y = 0.5f * (windowHeight - playerRect.size.y);
		}
		sprPlayer.Position(newPos);
	}
	sprPlayer.Update(deltaTime);

	// 敵の出現.
#if 1
	const TiledMap::Layer& tiledMapLayer = enemyMap.GetLayer(0);
	const glm::vec2 tileSize = enemyMap.GetTileSet(tiledMapLayer.tilesetNo).size;
	// 敵配置マップ参照位置の更新.
	const float enemyMapScrollSpeed = 100; // 更新速度.
	mapCurrentPosX += enemyMapScrollSpeed * deltaTime;
	if (mapCurrentPosX >= tiledMapLayer.size.x * tileSize.x) {
		// 終端を超えたら先頭にループ.
		mapCurrentPosX = 0;
		mapProcessedX = 0;
	}
	// 次の列に到達したらデータを読む.
	if (mapCurrentPosX - mapProcessedX >= tileSize.x) {
		mapProcessedX += tileSize.x;
		const int mapX = static_cast<int>(mapProcessedX / tileSize.x);
		for (int mapY = 0; mapY < tiledMapLayer.size.y; ++mapY) {
			const int enemyId = 256; // 敵とみなすタイルID.
			if (tiledMapLayer.At(mapY, mapX) == enemyId) {
				Actor* enemy = findAvailableActor(std::begin(enemyList), std::end(enemyList));
				// 空いている構造体が見つかったら、それを使って敵を出現させる.
				if (enemy != nullptr) {
					const float y = windowHeight * 0.5f - static_cast<float>(mapY * tileSize.x);
					enemy->spr = Sprite("Res/Objects.png", glm::vec3(0.5f * windowWidth, y, 0), Rect(480, 0, 32, 32));
					enemy->spr.Animator(FrameAnimation::Animate::Create(tlEnemy));
					namespace TA = TweenAnimation;
					TA::SequencePtr seq = TA::Sequence::Create(4);
					seq->Add(TA::MoveBy::Create(1, glm::vec3(0, 100, 0), TA::EasingType::EaseInOut, TA::Target::Y));
					seq->Add(TA::MoveBy::Create(1, glm::vec3(0, -100, 0), TA::EasingType::EaseInOut, TA::Target::Y));
					TA::ParallelizePtr par = TA::Parallelize::Create(1);
					par->Add(seq);
					par->Add(TA::MoveBy::Create(8, glm::vec3(-1000, 0, 0), TA::EasingType::Linear, TA::Target::X));
					enemy->spr.Tweener(TA::Animate::Create(par));
					enemy->collisionShape = Rect(-16, -16, 32, 32);
					enemy->health = 2;
				}
			}
		}
	}
#else
	// 出現までの時間が0以下になったら敵を出現させる.
	enemyGenerationTimer -= deltaTime;
	if (enemyGenerationTimer <= 0) {
		// 空いている(破壊されている)敵構造体を検索.
		Actor* enemy = nullptr;
		for (Actor* i = std::begin(enemyList); i != std::end(enemyList); ++i) {
			if (i->health <= 0) {
				enemy = i;
				break;
			}
		}
		// 空いている構造体が見つかったら、それを使って敵を出現させる.
		if (enemy != nullptr) {
			const std::uniform_real_distribution<float> y_distribution(
				-0.5f * windowHeight, 0.5f * windowHeight);
			enemy->spr = Sprite("Res/Objects.png",
				glm::vec3(0.5f * windowWidth, y_distribution(random), 0),
				Rect(480, 0, 32, 32));
			// フレームアニメーションの設定.
			enemy->spr.Animator(FrameAnimation::Animate::Create(tlEnemy));
			// トウィーニングの設定.
			namespace TA = TweenAnimation;
			TA::SequencePtr seq = TA::Sequence::Create(1);
			seq->Add(TA::MoveBy::Create(1, glm::vec3(-400, 0, 0), TA::EasingType::EaseOut, TA::Target::X));
			seq->Add(TA::MoveBy::Create(1, glm::vec3(400, 0, 0), TA::EasingType::EaseIn, TA::Target::X));
			TA::ParallelizePtr par = TA::Parallelize::Create(1);
			par->Add(seq);
			par->Add(TA::MoveBy::Create(2, glm::vec3(0, -300, 0), TA::EasingType::Linear, TA::Target::Y));
			enemy->spr.Tweener(TA::Animate::Create(par));
			enemy->collisionShape = Rect(-16, -16, 32, 32);
			enemy->health = 3;
			// 次の敵が出現するまでの時間を設定する.
			const std::uniform_real_distribution<float> time_distribution(0.5f, 4.0f);
			enemyGenerationTimer = time_distribution(random);
		}
	}
#endif
	// Actorの更新.
	updateActorList(std::begin(enemyList), std::end(enemyList), deltaTime);
	updateActorList(std::begin(playerBulletList), std::end(playerBulletList), deltaTime);
	updateActorList(std::begin(effectList), std::end(effectList), deltaTime);

	// 自機の弾と敵の衝突判定.
	detectCollision(
		std::begin(playerBulletList), std::end(playerBulletList),
		std::begin(enemyList), std::end(enemyList),
		playerBulletAndEnemyContactHandler);
}

/**
* ゲームの状態を描画する.
*
* @param window ゲームを管理するウィンドウ.
*/
void render(GLFWEW::WindowRef window)
{
	renderer.BeginUpdate();
	renderer.AddVertices(sprBackground);
	renderer.AddVertices(sprPlayer);
	renderActorList(std::begin(enemyList), std::end(enemyList));
	renderActorList(std::begin(playerBulletList), std::end(playerBulletList));
	renderActorList(std::begin(effectList), std::end(effectList));
	renderer.EndUpdate();
	renderer.Draw(glm::vec2(windowWidth, windowHeight));

	// 文字列を表示する.
	fontRenderer.BeginUpdate();
	char str[9];
	snprintf(str, sizeof(str), "%08d", score);
	fontRenderer.AddString(glm::vec2(-100, 300), str);
	fontRenderer.EndUpdate();
	fontRenderer.Draw();

	window.SwapBuffers();
}

/**
* 2つの長方形の衝突状態を調べる.
*
* @param lhs 長方形その1.
* @param rhs 長方形その2.
*
* @retval true  衝突している.
* @retval false 衝突していない.
*/
bool detectCollision(const Rect* lhs, const Rect* rhs)
{
	return
		lhs->origin.x < rhs->origin.x + rhs->size.x &&
		lhs->origin.x + lhs->size.x > rhs->origin.x &&
		lhs->origin.y < rhs->origin.y + rhs->size.y &&
		lhs->origin.y + lhs->size.y > rhs->origin.y;
}

/**
* Actorの配列を初期化する.
*
* @param  first  初期化する配列の先頭.
* @param  last    初期化する配列の終端.
*/
void initializeActorList(Actor* first, Actor* last)
{
	for (Actor* i = first; i != last; ++i) {
		i->health = 0;
	}
}

/**
* Actorの配列を更新する.
*
* @param  first      更新する配列の先頭.
* @param  last       更新する配列の終端.
* @param  deltaTime 前回の更新からの経過時間.
*/
void updateActorList(Actor* first, Actor* last, float deltaTime)
{
	for (Actor* i = first; i != last; ++i) {
		if (i->health > 0) {
			i->spr.Update(deltaTime);
			if (i->spr.Tweener()->IsFinished()) {
				i->health = 0;
			}
		}
	}
}

/**
* Actorの配列を描画する.
*
* @param  first      更新する配列の先頭.
* @param  last       更新する配列の終端.
*/
void renderActorList(const Actor* first, const Actor* last)
{
	for (const Actor* i = first; i != last; ++i) {
		if (i->health > 0) {
			renderer.AddVertices(i->spr);
		}
	}
}

/**
* 利用可能なのActorを取得する.
*
* @param first 検索対象の先頭要素のポインタ.
* @param last  検索対象の終端要素のポインタ.
*
* @return 利用可能なActorのポインタ.
*         利用可能なActorが見つからなければnullptr.
*
* [first, last)の範囲から、利用可能な(healthが0以下の)Actorを検索する.
*/
Actor* findAvailableActor(Actor* first, Actor* last)
{
	Actor* result = nullptr;
	for (Actor* i = first;
		i != last; ++i) {
		if (i->health <= 0) {
			result = i;
			break;
		}
	}
	return result;
}

/**
* 衝突を検出する.
*
* @param firstA    衝突させる配列Aの先頭ポインタ.
* @param lastA     衝突させる配列Aの終端ポインタ.
* @param firstB    衝突させる配列Bの先頭ポインタ.
* @param lastB     衝突させる配列Bの終端ポインタ.
* @param handler   A-B間で衝突が検出されたときに実行する関数.
*/
void detectCollision(Actor* firstA, Actor* lastA, Actor* firstB, Actor* lastB, CollisionHandlerType handler)
{
	for (Actor* a = firstA; a != lastA; ++a) {
		if (a->health <= 0) {
			continue;
		}
		Rect rectA = a->collisionShape;
		rectA.origin += glm::vec2(a->spr.Position());
		for (Actor* b = firstB; b != lastB; ++b) {
			if (b->health <= 0) {
				continue;
			}
			Rect rectB = b->collisionShape;
			rectB.origin += glm::vec2(b->spr.Position());
			if (detectCollision(&rectA, &rectB)) {
				handler(a, b);
				if (a->health <= 0) {
					break;
				}
			}
		}
	}
}

/**
* 自機の弾と敵の衝突を処理する.
*
* @param bullet 自機の弾のポインタ.
* @param enemy  敵のポインタ.
*/
void playerBulletAndEnemyContactHandler(Actor* bullet, Actor* enemy)
{
	const int tmp = bullet->health;
	bullet->health -= enemy->health;
	enemy->health -= tmp;
	score += 100; // 敵を破壊したら得点を増やす.
}
