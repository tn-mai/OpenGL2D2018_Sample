/**
* @file Main.cpp
*/
#include "GLFWEW.h"
#include "Texture.h"
#include "Sprite.h"

const char windowTitle[] = "OpenGL2D 2018"; // �^�C�g���o�[�ɕ\������镶��.
const int windowWidth = 800; // �E�B���h�E�̕`��̈�̕�.
const int windowHeight = 600; // �E�B���h�E�̕`��̈�̍���.

SpriteRenderer renderer; // �X�v���C�g��`�悷��I�u�W�F�N�g.

/**
* �v���O�����̃G���g���[�|�C���g.
*/
int main()
{
	// �A�v���P�[�V�����̏�����.
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

	Texture::Finalize();
	return 0;
}