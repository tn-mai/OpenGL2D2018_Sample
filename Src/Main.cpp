/**
* @file Main.cpp
*/
#include "MainScene.h"
#include "TitleScene.h"
#include "GameOverScene.h"
#include "GameData.h"
#include "Actor.h"
#include "GLFWEW.h"
#include "Texture.h"
#include "Sprite.h"
#include "Font.h"
#include "TiledMap.h"
#include "Audio.h"
#include <random>
#include <glm/gtc/matrix_transform.hpp>

const char windowTitle[] = "OpenGL2D 2018"; // タイトルバーに表示される文章.

std::mt19937 random; // 乱数を発生させる変数(乱数エンジン).

SpriteRenderer renderer; // スプライトを描画するオブジェクト.
FontRenderer fontRenderer; // フォント描画用変数.
Sprite sprBackground; // 背景用スプライト.
Actor sprPlayer;     // 自機用スプライト.

glm::vec3 playerVelocity; // 自機の移動速度.

Actor enemyList[128]; // 敵のリスト.
Actor playerBulletList[128]; // 自機の弾のリスト.
Actor playerLaserList[3]; // 自機のレーザーのリスト.
Actor effectList[128]; // 爆発などの特殊効果用スプライトのリスト.
Actor itemList[64]; // アイテム用スプライトのリスト.
float enemyGenerationTimer; // 次の敵が出現するまでの時間(単位:秒).
int score; // プレイヤーの得点.
float timer; // シーン切り替えで使用するタイマー.
const int weaponLevelMin = 0; // 自機の武器強化の最低段階.
const int weaponLevelMax = 3; // 自機の武器強化の最高段階.
int weaponLevel; // 自機の武器強化段階.

// 音声制御用変数.
Audio::SoundPtr bgm;
Audio::SoundPtr sePlayerShot;
Audio::SoundPtr seBlast;
Audio::SoundPtr sePowerUp;

// ゲームの状態.
int gamestate; // 実行中の場面ID.

TitleScene titleScene;
MainScene mainScene;
GameOverScene gameOverScene;

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

// 爆発アニメーション.
const FrameAnimation::KeyFrame blastKeyFrames[] = {
	{ 0 / 60.0f, glm::vec2(416, 0), glm::vec2(32, 32) },
	{ 5 / 60.0f, glm::vec2(416, 32), glm::vec2(32, 32) },
	{ 10 / 60.0f, glm::vec2(416, 64), glm::vec2(32, 32) },
	{ 15 / 60.0f, glm::vec2(416, 96), glm::vec2(32, 32) },
	{ 20 / 60.0f, glm::vec2(416, 96), glm::vec2(32, 32) },
};
FrameAnimation::TimelinePtr tlBlast;

/*
* プロトタイプ宣言.
*/
void processInput(GLFWEW::WindowRef);
void update(GLFWEW::WindowRef);
void render(GLFWEW::WindowRef);
void playerBulletAndEnemyContactHandler(Actor*, Actor*);
void playerAndEnemyContactHandler(Actor*, Actor*);
void playerAndItemContactHandler(Actor*, Actor*);

/**
* メイン画面用の構造体の初期設定を行う.
*
* @param scene     メイン画面用構造体のポインタ.
*
* @retval true  初期化成功.
* @retval false 初期化失敗.
*/
bool initialize(MainScene* scene)
{
	// ゲームの初期設定を行う.

	//スプライトに画像を設定.
	sprBackground = Sprite("Res/UnknownPlanet.png");
	sprPlayer.spr = Sprite("Res/Objects.png", glm::vec3(0, 0, 0), Rect(0, 0, 64, 32));
	sprPlayer.spr.Animator(FrameAnimation::Animate::Create(tlPlayer));
	sprPlayer.collisionShape = Rect(-24, -8, 48, 16);
	sprPlayer.health = 1;

	initializeActorList(std::begin(enemyList), std::end(enemyList));
	initializeActorList(std::begin(playerBulletList), std::end(playerBulletList));
	initializeActorList(std::begin(playerLaserList), std::end(playerLaserList));
	initializeActorList(std::begin(effectList), std::end(effectList));
	initializeActorList(std::begin(itemList), std::end(itemList));

	enemyGenerationTimer = 2;
	score = 0;
	weaponLevel = weaponLevelMin;
	timer = 0;

	// 敵配置マップを読み込む.
	enemyMap.Load("Res/EnemyMap.json");
	mapCurrentPosX = windowWidth;
	mapProcessedX = windowWidth;

	// 音声を準備する.
	Audio::EngineRef audio = Audio::Engine::Instance();
	seBlast = audio.Prepare("Res/Audio/Blast.xwm");
	sePlayerShot = audio.Prepare("Res/Audio/PlayerShot.xwm");
	sePowerUp = audio.Prepare("Res/Audio/GetItem.xwm");
	bgm = audio.Prepare("Res/Audio/Neolith.xwm");
	// BGMをループ再生する.
	bgm->Play(Audio::Flag_Loop);

	return true;
}

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
	// 音声再生システムの初期化.
	Audio::EngineRef audio = Audio::Engine::Instance();
	if (!audio.Initialize()) {
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
	tlBlast = FrameAnimation::Timeline::Create(blastKeyFrames);

	// タイトル画面を初期化する.
	gamestate = gamestateTitle;
	initialize(&titleScene);

	// ゲームループ.
	while (!window.ShouldClose()) {
		processInput(window);
		update(window);
		render(window);
		audio.Update();
	}

	Texture::Finalize();
	audio.Destroy();
	return 0;
}

/**
* プレイヤーの入力を処理する.
*
* @param window ゲームを管理するウィンドウ.
*/
void processInput(GLFWEW::WindowRef window)
{
	if (gamestate == gamestateTitle) {
		processInput(window, &titleScene);
		return;
	} else if (gamestate == gamestateGameOver) {
		processInput(window, &gameOverScene);
		return;
	}

	window.Update();

	if (sprPlayer.health <= 0) {
		playerVelocity = glm::vec3(0, 0, 0);
	} else {
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
			for (int i = 0; i < weaponLevel * 2 + 1; ++i) {
				Actor* bullet = findAvailableActor(std::begin(playerBulletList), std::end(playerBulletList));
				// 空いている構造体が見つかったら、それを使って弾を発射する.
				if (bullet != nullptr) {
					bullet->spr = Sprite("Res/Objects.png", sprPlayer.spr.Position(), Rect(64, 0, 32, 16));
					const float angles[] = { 0.0f, -7.5f, 7.5f, -15.0f, 15.0f, -22.5f, 22.5f };
					const glm::vec3 v = glm::rotate(glm::mat4(), glm::radians(angles[i]), glm::vec3(0, 0, 1)) * glm::vec4(1200, 0, 0, 1);
					bullet->spr.Tweener(TweenAnimation::Animate::Create(
						TweenAnimation::MoveBy::Create(1, v,
						TweenAnimation::EasingType::Linear)));
					bullet->spr.Rotation(glm::radians(angles[i]));
					bullet->collisionShape = Rect(-8, -4, 16, 8);
					bullet->health = 4;
					sePlayerShot->Play(); // 弾の発射音を再生.
				}
			}
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

	if (gamestate == gamestateTitle) {
		update(window, &titleScene);
		return;
	} else if (gamestate == gamestateMain) {
		// 自機が破壊されていたらゲームオーバー画面を表示する.
		if (sprPlayer.health <= 0) {
			if (timer > 0) {
				timer -= deltaTime;
			} else {
				bgm->Stop(); // BGMを停止する.
				gamestate = gamestateGameOver;
				initialize(&gameOverScene);
				return;
			}
		}
	} else if (gamestate == gamestateGameOver) {
		update(window, &gameOverScene);
		return;
	}

	// 自機の移動.
	if (sprPlayer.health > 0) {
		if (playerVelocity.x || playerVelocity.y) {
			glm::vec3 newPos = sprPlayer.spr.Position() + playerVelocity * deltaTime;
			// 自機の移動範囲を画面内に制限する.
			const Rect playerRect = sprPlayer.spr.Rectangle();
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
			sprPlayer.spr.Position(newPos);
		}
		sprPlayer.spr.Update(deltaTime);
	}
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
			const int powerUpItemId = 230; // パワーアップアイテムのID.
			const int tileId = tiledMapLayer.At(mapY, mapX);
			if (tileId == enemyId) {
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
			} else if (tileId == powerUpItemId) {
				Actor* item = findAvailableActor(std::begin(itemList), std::end(itemList));
				if (item != nullptr) {
					const float y = windowHeight * 0.5f - static_cast<float>(mapY * tileSize.x);
					item->spr = Sprite("Res/Objects.png", glm::vec3(0.5f * windowWidth, y, 0), Rect(160, 32, 32, 32));
					namespace TA = TweenAnimation;
					item->spr.Tweener(TA::Animate::Create(TA::MoveBy::Create(16, glm::vec3(-1000, 0, 0))));
					item->collisionShape = Rect(-16, -16, 32, 32);
					item->health = 1;
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
	updateActorList(std::begin(playerLaserList), std::end(playerLaserList), deltaTime);
	updateActorList(std::begin(effectList), std::end(effectList), deltaTime);
	updateActorList(std::begin(itemList), std::end(itemList), deltaTime);

	// 自機の弾と敵の衝突判定.
	detectCollision(
		std::begin(playerBulletList), std::end(playerBulletList),
		std::begin(enemyList), std::end(enemyList),
		playerBulletAndEnemyContactHandler);
	// 自機と敵の衝突判定.
	detectCollision(
		&sprPlayer, &sprPlayer + 1,
		std::begin(enemyList), std::end(enemyList),
		playerAndEnemyContactHandler);
	// 自機とアイテムの衝突判定.
	detectCollision(
		&sprPlayer, &sprPlayer + 1,
		std::begin(itemList), std::end(itemList),
		playerAndItemContactHandler);
}

/**
* ゲームの状態を描画する.
*
* @param window ゲームを管理するウィンドウ.
*/
void render(GLFWEW::WindowRef window)
{
	if (gamestate == gamestateTitle) {
		render(window, &titleScene);
		return;
	} else if (gamestate == gamestateGameOver) {
		render(window, &gameOverScene);
		return;
	}

	renderer.BeginUpdate();
	renderer.AddVertices(sprBackground);
	if (sprPlayer.health > 0) {
		renderer.AddVertices(sprPlayer.spr);
	}
	renderActorList(std::begin(enemyList), std::end(enemyList));
	renderActorList(std::begin(playerBulletList), std::end(playerBulletList));
	renderActorList(std::begin(playerLaserList), std::end(playerLaserList));
	renderActorList(std::begin(effectList), std::end(effectList));
	renderActorList(std::begin(itemList), std::end(itemList));
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
	if (enemy->health <= 0) {
		score += 100;
		// 爆発を表示する.
		Actor* blast = findAvailableActor(std::begin(effectList), std::end(effectList));
		if (blast != nullptr) {
			blast->spr = Sprite("Res/Objects.png", enemy->spr.Position());
			blast->spr.Animator(FrameAnimation::Animate::Create(tlBlast));
			namespace TA = TweenAnimation;
			blast->spr.Tweener(TA::Animate::Create(TA::Rotation::Create(20 / 60.0f, 1.5f)));
			blast->health = 1;
			seBlast->Play();// 爆発音を再生.
		}
	}
}

/**
* 自機と敵の衝突を処理する.
*
* @param player 自機のポインタ.
* @param enemy  敵のポインタ.
*/
void playerAndEnemyContactHandler(Actor* player, Actor* enemy)
{
	const int tmp = player->health;
	player->health -= enemy->health;
	enemy->health -= tmp;
	if (enemy->health <= 0) {
		score += 100;
		Actor* blast = findAvailableActor(std::begin(effectList), std::end(effectList));
		if (blast != nullptr) {
			blast->spr = Sprite("Res/Objects.png", enemy->spr.Position());
			blast->spr.Animator(FrameAnimation::Animate::Create(tlBlast));
			namespace TA = TweenAnimation;
			blast->spr.Tweener(TA::Animate::Create(TA::Rotation::Create(20 / 60.0f, 1.5f)));
			blast->health = 1;
		}
	}
	if (player->health <= 0) {
		timer = 2;
		Actor* blast = findAvailableActor(
			std::begin(effectList), std::end(effectList));
		if (blast != nullptr) {
			blast->spr = Sprite("Res/Objects.png", player->spr.Position());
			blast->spr.Animator(FrameAnimation::Animate::Create(tlBlast));
			namespace TA = TweenAnimation;
			blast->spr.Tweener(TA::Animate::Create(TA::Rotation::Create(20 / 60.0f, 1.5f)));
			blast->spr.Scale(glm::vec2(2, 2)); // 自機の爆発は少し大きめにする.
			blast->health = 1;
			seBlast->Play();// 爆発音を再生.
		}
	}
}

/**
* 自機とアイテムの衝突を処理する.
*
* @param player 自機のポインタ.
* @param item   アイテムのポインタ.
*/
void playerAndItemContactHandler(Actor* player, Actor* item)
{
	item->health = 0;
	sePowerUp->Play();

	// 自機の武器を強化する.
	++weaponLevel;
	if (weaponLevel > weaponLevelMax) {
		weaponLevel = weaponLevelMax;
		score += 1000;
	}
}