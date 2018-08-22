#pragma once
/**
* @file GameData.h
*/
#include "Sprite.h"
#include "Font.h"

const int windowWidth = 800; // �E�B���h�E�̕`��̈�̕�.
const int windowHeight = 600; // �E�B���h�E�̕`��̈�̍���.

extern SpriteRenderer renderer; // �X�v���C�g�`��p�ϐ�.
extern FontRenderer fontRenderer; // �t�H���g�`��p�ϐ�.

// �Q�[���̏��.
const int gamestateTitle = 0; // �^�C�g����ʂ̏��ID.
const int gamestateMain = 1; // �Q�[����ʂ̏��ID.
const int gamestateGameOver = 2; // �Q�[���I�[�o�[��ʂ̏��ID.
extern int gamestate; // ���s���̏��ID.

// ��s�錾.
struct TitleScene;
struct MainScene;
struct GameOverScene;

extern TitleScene titleScene;
extern MainScene mainScene;
extern GameOverScene gameOverScene;
