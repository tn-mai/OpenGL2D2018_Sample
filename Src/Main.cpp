/**
* @file Main.cpp
*/
#include "GLFWEW.h"
#include "Texture.h"
#include "Sprite.h"
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
Sprite sprBackground; // 背景用スプライト.
Sprite sprPlayer;     // 自機用スプライト.

glm::vec3 playerVelocity; // 自機の移動速度.

Actor enemyList[128]; // 敵のリスト.
Actor playerBulletList[128]; // 自機の弾のリスト.
float enemyGenerationTimer; // 次の敵が出現するまでの時間(単位:秒).

void processInput(GLFWEW::WindowRef);
void update(GLFWEW::WindowRef);
void render(GLFWEW::WindowRef);
bool detectCollision(const Rect*, const Rect*);

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

	random.seed(std::random_device()()); // 乱数エンジンの初期化.

	//スプライトに画像を設定.
	sprBackground = Sprite("Res/UnknownPlanet.png");
	sprPlayer = Sprite("Res/Objects.png", glm::vec3(0, 0, 0), Rect(0, 0, 64, 32));

	// 敵の配列を初期化.
	for (Actor* i = std::begin(enemyList); i != std::end(enemyList); ++i) {
		i->health = 0;
	}
	// 自機の弾の配列を初期化.
	for (Actor* i = std::begin(playerBulletList); i != std::end(playerBulletList); ++i) {
		i->health = 0;
	}

	enemyGenerationTimer = 2;

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
		// 空いている弾の構造体を検索.
		Actor* bullet = nullptr;
		for (Actor* i = std::begin(playerBulletList);
			i != std::end(playerBulletList); ++i) {
			if (i->health <= 0) {
				bullet = i;
				break;
			}
		}
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
			namespace TA = TweenAnimation;
			TA::SequencePtr seq = TA::Sequence::Create(4);
			seq->Add(TA::MoveBy::Create(1, glm::vec3(-100, 0, 0), TA::EasingType::Linear));
			seq->Add(TA::MoveBy::Create(1, glm::vec3(0, 100, 0), TA::EasingType::Linear));
			seq->Add(TA::MoveBy::Create(1, glm::vec3(-100, 0, 0), TA::EasingType::Linear));
			seq->Add(TA::MoveBy::Create(1, glm::vec3(0, -100, 0), TA::EasingType::Linear));
			enemy->spr.Tweener(TA::Animate::Create(seq));
			enemy->collisionShape = Rect(-16, -16, 32, 32);
			enemy->health = 3;
			// 次の敵が出現するまでの時間を設定する.
			const std::uniform_real_distribution<float> time_distribution(0.5f, 4.0f);
			enemyGenerationTimer = time_distribution(random);
		}
	}
	// 敵の更新.
	for (Actor* i = std::begin(enemyList); i != std::end(enemyList); ++i) {
		if (i->health > 0) {
			i->spr.Update(deltaTime);
			if (i->spr.Tweener()->IsFinished()) {
				i->health = 0;
			}
		}
	}
	// 自機の弾の更新.
	for (Actor* i = std::begin(playerBulletList); i != std::end(playerBulletList); ++i) {
		if (i->health > 0) {
			i->spr.Update(deltaTime);
			if (i->spr.Tweener()->IsFinished()) {
				i->health = 0;
			}
		}
	}

	// 自機の弾と敵の衝突判定.
	for (Actor* bullet = std::begin(playerBulletList); bullet != std::end(playerBulletList); ++bullet) {
		if (bullet->health <= 0) {
			continue;
		}
		Rect shotRect = bullet->collisionShape;
		shotRect.origin += glm::vec2(bullet->spr.Position());
		for (Actor* enemy = std::begin(enemyList); enemy != std::end(enemyList); ++enemy) {
			if (enemy->health <= 0) {
				continue;
			}
			Rect enemyRect = enemy->collisionShape;
			enemyRect.origin += glm::vec2(enemy->spr.Position());
			if (detectCollision(&shotRect, &enemyRect)) {
				const int tmp = bullet->health;
				bullet->health -= enemy->health;
				enemy->health -= tmp;
				break;
			}
		}
	}
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
	for (const Actor* i = std::begin(enemyList); i != std::end(enemyList); ++i) {
		if (i->health > 0) {
			renderer.AddVertices(i->spr);
		}
	}
	for (const Actor* i = std::begin(playerBulletList); i != std::end(playerBulletList); ++i) {
		if (i->health > 0) {
			renderer.AddVertices(i->spr);
		}
	}
	renderer.EndUpdate();
	renderer.Draw(glm::vec2(windowWidth, windowHeight));
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
