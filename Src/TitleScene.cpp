/**
* @file TitleScene.cpp
*/
#include "TitleScene.h"
#include "MainScene.h"
#include "GameData.h"
#include "Audio.h"

/**
* �^�C�g����ʗp�̍\���̂̏����ݒ���s��.
*
* @param scene     �^�C�g����ʗp�\���̂̃|�C���^.
*
* @retval true  ����������.
* @retval false ���������s.
*/
bool initialize(TitleScene* scene)
{
	scene->bg = Sprite("Res/UnknownPlanet.png");
	scene->logo = Sprite("Res/Title.png", glm::vec3(0, 100, 0));
	scene->mode = scene->modeStart;
	scene->timer = 0.5f; // ���͂��󂯕t���Ȃ�����(�b).
	return true;
}

/**
* �^�C�g����ʂ̏I���������s��.
*
* @param scene  �^�C�g����ʗp�\���̂̃|�C���^.
*/
void finalize(TitleScene* scene)
{
	scene->bg = Sprite();
	scene->logo = Sprite();
}

/**
* �^�C�g����ʂ̃v���C���[���͂���������.
*
* @param window �Q�[�����Ǘ�����E�B���h�E.
* @param scene  �^�C�g����ʗp�\���̂̃|�C���^.
*/
void processInput(GLFWEW::WindowRef window, TitleScene* scene)
{
	window.Update();
	// ���͎�t���[�h�ɂȂ�܂łȂɂ����Ȃ�.
	if (scene->mode != scene->modeTitle) {
		return;
	}
	// A�܂���START�{�^���������ꂽ��A�Q�[���J�n�҂����[�h�Ɉڂ�.
	const GamePad gamepad = window.GetGamePad();
	if (gamepad.buttonDown & (GamePad::A | GamePad::START)) {
		scene->mode = scene->modeNextState;
		scene->timer = 2.0f;
		Audio::Engine::Instance().Prepare("Res/Audio/Start.xwm")->Play();
	}
}

/**
* �^�C�g����ʂ��X�V����.
*
* @param window �Q�[�����Ǘ�����E�B���h�E.
* @param scene  �^�C�g����ʗp�\���̂̃|�C���^.
*/
void update(GLFWEW::WindowRef window, TitleScene* scene)
{
	const float deltaTime = window.DeltaTime();

	scene->bg.Update(deltaTime);
	scene->logo.Update(deltaTime);

	// �^�C�}�[��0�ȉ��ɂȂ�܂ŃJ�E���g�_�E��.
	if (scene->timer > 0) {
		scene->timer -= deltaTime;
		return;
	}

	if (scene->mode == scene->modeStart) {
		scene->mode = scene->modeTitle;
	} else if (scene->mode == scene->modeNextState) {
		finalize(scene); // �^�C�g����ʂ̌�n��.
		gamestate = gamestateMain;
		initialize(&mainScene);
	}
}

/**
* �^�C�g����ʂ�`�悷��.
*
* @param window �Q�[�����Ǘ�����E�B���h�E.
* @param scene  �^�C�g����ʗp�\���̂̃|�C���^.
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
		// �Q�[���J�n�҂��̂Ƃ��͕�����_�ł�����.
		if ((int)(scene->timer * 10) % 2) {
			fontRenderer.AddString(glm::vec2(-80, -100), "START");
		}
	}
	fontRenderer.EndUpdate();
	fontRenderer.Draw();

	window.SwapBuffers();
}
