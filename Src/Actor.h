#pragma once
/**
* @file Actor.h
*
* Actor�\���̂����Actor�\���̂Ɋ֘A����֐���錾����w�b�_�t�@�C��.
*/
#include "Sprite.h"

/**
* �Q�[���L�����N�^�[�\����.
*/
struct Actor
{
	Sprite spr; // �摜�\���p�X�v���C�g.
	Rect collisionShape; // �Փ˔���̈ʒu�Ƒ傫��.
	int health; // �ϋv��(0�ȉ��Ȃ�j�󂳂�Ă���).
};