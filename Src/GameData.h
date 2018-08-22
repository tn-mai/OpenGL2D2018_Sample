#pragma once
/**
* @file GameData.h
*/
#include "Sprite.h"
#include "Font.h"

const int windowWidth = 800; // ウィンドウの描画領域の幅.
const int windowHeight = 600; // ウィンドウの描画領域の高さ.

extern SpriteRenderer renderer; // スプライト描画用変数.
extern FontRenderer fontRenderer; // フォント描画用変数.

// ゲームの状態.
const int gamestateTitle = 0; // タイトル画面の場面ID.
const int gamestateMain = 1; // ゲーム画面の場面ID.
const int gamestateGameOver = 2; // ゲームオーバー画面の場面ID.
extern int gamestate; // 実行中の場面ID.

// 先行宣言.
struct TitleScene;
struct MainScene;
struct GameOverScene;

extern TitleScene titleScene;
extern MainScene mainScene;
extern GameOverScene gameOverScene;
