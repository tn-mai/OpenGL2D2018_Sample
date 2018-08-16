/**
* @file Main.cpp
*/
#include "GLFWEW.h"
#include "Texture.h"
#include "Sprite.h"

const char windowTitle[] = "OpenGL2D 2018"; // タイトルバーに表示される文章.
const int windowWidth = 800; // ウィンドウの描画領域の幅.
const int windowHeight = 600; // ウィンドウの描画領域の高さ.

SpriteRenderer renderer; // スプライトを描画するオブジェクト.
Sprite sprBackground; // 背景用スプライト.
Sprite sprPlayer;     // 自機用スプライト.

void processInput(GLFWEW::WindowRef);
void update(GLFWEW::WindowRef);
void render(GLFWEW::WindowRef);

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

	//スプライトに画像を設定.
	sprBackground = Sprite("Res/UnknownPlanet.png");
	sprPlayer = Sprite("Res/Objects.png", glm::vec3(0, 0, 0), Rect(0, 0, 64, 32));

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
}

/**
* ゲームの状態を更新する.
*
* @param window ゲームを管理するウィンドウ.
*/
void update(GLFWEW::WindowRef window)
{
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
	renderer.EndUpdate();
	renderer.Draw(glm::vec2(windowWidth, windowHeight));
	window.SwapBuffers();
}
