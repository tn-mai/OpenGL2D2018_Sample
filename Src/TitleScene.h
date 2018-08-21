#pragma once
/**
* @file TitleScene.h
*/
#include "GLFWEW.h"
#include "Sprite.h"

/**
* タイトル画面用の構造体.
*
* タイトル画面で使用する変数などを格納する.
*/
struct TitleScene
{
	Sprite bg;
	Sprite logo;
	const int modeStart = 0; // タイトル表示待ちモード.
	const int modeTitle = 1; // 入力受付モード.
	const int modeNextState = 2; // ゲーム開始待ちモード.
	int mode; // 実行中のモード.
	float timer; // モード切り替えで使用するタイマー.
};
bool initialize(TitleScene*);
void finalize(TitleScene*);
void processInput(GLFWEW::WindowRef, TitleScene*);
void update(GLFWEW::WindowRef, TitleScene*);
void render(GLFWEW::WindowRef, TitleScene*);