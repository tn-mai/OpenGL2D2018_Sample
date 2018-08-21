/**
* @file TitleScene.cpp
*/
#include "TitleScene.h"

/**
* タイトル画面用の構造体の初期設定を行う.
*
* @param scene     タイトル画面用構造体のポインタ.
*
* @retval true  初期化成功.
* @retval false 初期化失敗.
*/
bool initialize(TitleScene* scene)
{
	scene->bg = Sprite("Res/UnknownPlanet.png");
	scene->logo = Sprite("Res/Title.png", glm::vec3(0, 100, 0));
	scene->mode = scene->modeStart;
	scene->timer = 0.5f; // 入力を受け付けない期間(秒).
	return true;
}

/**
* タイトル画面の終了処理を行う.
*
* @param scene  タイトル画面用構造体のポインタ.
*/
void finalize(TitleScene* scene)
{
	scene->bg = Sprite();
	scene->logo = Sprite();
}

/**
* タイトル画面のプレイヤー入力を処理する.
*
* @param window ゲームを管理するウィンドウ.
* @param scene  タイトル画面用構造体のポインタ.
*/
void processInput(GLFWEW::WindowRef window, TitleScene* scene)
{
	window.Update();
	// 入力受付モードになるまでなにもしない.
	if (scene->mode != scene->modeTitle) {
		return;
	}
	// AまたはSTARTボタンが押されたら、ゲーム開始待ちモードに移る.
	const GamePad gamepad = window.GetGamePad();
	if (gamepad.buttonDown & (GamePad::A | GamePad::START)) {
		scene->mode = scene->modeNextState;
		scene->timer = 2.0f;
		Audio::Engine::Instance().Prepare("Res/Audio/Start.xwm")->Play();
	}
}

/**
* タイトル画面を更新する.
*
* @param window ゲームを管理するウィンドウ.
* @param scene  タイトル画面用構造体のポインタ.
*/
void update(GLFWEW::WindowRef window, TitleScene* scene)
{
	const float deltaTime = window.DeltaTime();

	scene->bg.Update(deltaTime);
	scene->logo.Update(deltaTime);

	// タイマーが0以下になるまでカウントダウン.
	if (scene->timer > 0) {
		scene->timer -= deltaTime;
		return;
	}

	if (scene->mode == scene->modeStart) {
		scene->mode = scene->modeTitle;
	} else if (scene->mode == scene->modeNextState) {
		finalize(scene); // タイトル画面の後始末.
		gamestate = gamestateMain;

		// ゲームの初期設定を行う.

		//スプライトに画像を設定.
		sprBackground = Sprite("Res/UnknownPlanet.png");
		sprPlayer.spr = Sprite("Res/Objects.png", glm::vec3(0, 0, 0), Rect(0, 0, 64, 32));
		sprPlayer.spr.Animator(FrameAnimation::Animate::Create(tlPlayer));
		sprPlayer.collisionShape = Rect(-24, -8, 48, 16);
		sprPlayer.health = 1;

		initializeActorList(std::begin(enemyList), std::end(enemyList));
		initializeActorList(std::begin(playerBulletList), std::end(playerBulletList));
		initializeActorList(std::begin(effectList), std::end(effectList));

		enemyGenerationTimer = 2;
		score = 0;
		timer = 0;

		// 敵配置マップを読み込む.
		enemyMap.Load("Res/EnemyMap.json");
		mapCurrentPosX = windowWidth;
		mapProcessedX = windowWidth;

		// 音声を準備する.
		Audio::EngineRef audio = Audio::Engine::Instance();
		seBlast = audio.Prepare("Res/Audio/Blast.xwm");
		sePlayerShot = audio.Prepare("Res/Audio/PlayerShot.xwm");
		bgm = audio.Prepare("Res/Audio/Neolith.xwm");
		// BGMをループ再生する.
		bgm->Play(Audio::Flag_Loop);
	}
}

/**
* タイトル画面を描画する.
*
* @param window ゲームを管理するウィンドウ.
* @param scene  タイトル画面用構造体のポインタ.
*/
void render(GLFWEW::WindowRef window, TitleScene* scene)
{
	renderer.BeginUpdate();
	renderer.AddVertices(scene->bg);
	renderer.AddVertices(scene->logo);
	renderer.EndUpdate();
	renderer.Draw(glm::vec2(windowWidth, windowHeight));

	fontRenderer.BeginUpdate();
	if (scene->mode == scene->modeTitle) {
		fontRenderer.AddString(glm::vec2(-80, -100), "START");
	} else if (scene->mode == scene->modeNextState) {
		// ゲーム開始待ちのときは文字を点滅させる.
		if ((int)(scene->timer * 10) % 2) {
			fontRenderer.AddString(glm::vec2(-80, -100), "START");
		}
	}
	fontRenderer.EndUpdate();
	fontRenderer.Draw();

	window.SwapBuffers();
}
