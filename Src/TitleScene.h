#pragma once
/**
* @file TitleScene.h
*/
#include "GLFWEW.h"
#include "Sprite.h"

/**
* �^�C�g����ʗp�̍\����.
*
* �^�C�g����ʂŎg�p����ϐ��Ȃǂ��i�[����.
*/
struct TitleScene
{
	Sprite bg;
	Sprite logo;
	const int modeStart = 0; // �^�C�g���\���҂����[�h.
	const int modeTitle = 1; // ���͎�t���[�h.
	const int modeNextState = 2; // �Q�[���J�n�҂����[�h.
	int mode; // ���s���̃��[�h.
	float timer; // ���[�h�؂�ւ��Ŏg�p����^�C�}�[.
};
bool initialize(TitleScene*);
void finalize(TitleScene*);
void processInput(GLFWEW::WindowRef, TitleScene*);
void update(GLFWEW::WindowRef, TitleScene*);
void render(GLFWEW::WindowRef, TitleScene*);