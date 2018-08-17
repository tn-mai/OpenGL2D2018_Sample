/**
* @file Main.cpp
*/
#include "GLFWEW.h"
#include "Texture.h"
#include "Sprite.h"
#include <random>

const char windowTitle[] = "OpenGL2D 2018"; // �^�C�g���o�[�ɕ\������镶��.
const int windowWidth = 800; // �E�B���h�E�̕`��̈�̕�.
const int windowHeight = 600; // �E�B���h�E�̕`��̈�̍���.

std::mt19937 random; // �����𔭐�������ϐ�(�����G���W��).

/**
* �Q�[���L�����N�^�[�\����.
*/
struct Actor
{
	Sprite spr; // �摜�\���p�X�v���C�g.
	Rect collisionShape; // �Փ˔���̈ʒu�Ƒ傫��.
	int health; // �ϋv��(0�ȉ��Ȃ�j�󂳂�Ă���).
};

SpriteRenderer renderer; // �X�v���C�g��`�悷��I�u�W�F�N�g.
Sprite sprBackground; // �w�i�p�X�v���C�g.
Sprite sprPlayer;     // ���@�p�X�v���C�g.

glm::vec3 playerVelocity; // ���@�̈ړ����x.

Actor enemyList[128]; // �G�̃��X�g.
Actor playerBulletList[128]; // ���@�̒e�̃��X�g.
float enemyGenerationTimer; // ���̓G���o������܂ł̎���(�P��:�b).

void processInput(GLFWEW::WindowRef);
void update(GLFWEW::WindowRef);
void render(GLFWEW::WindowRef);
bool detectCollision(const Rect*, const Rect*);

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

	random.seed(std::random_device()()); // �����G���W���̏�����.

	//�X�v���C�g�ɉ摜��ݒ�.
	sprBackground = Sprite("Res/UnknownPlanet.png");
	sprPlayer = Sprite("Res/Objects.png", glm::vec3(0, 0, 0), Rect(0, 0, 64, 32));

	// �G�̔z���������.
	for (Actor* i = std::begin(enemyList); i != std::end(enemyList); ++i) {
		i->health = 0;
	}
	// ���@�̒e�̔z���������.
	for (Actor* i = std::begin(playerBulletList); i != std::end(playerBulletList); ++i) {
		i->health = 0;
	}

	enemyGenerationTimer = 2;

	// �Q�[�����[�v.
	while (!window.ShouldClose()) {
		processInput(window);
		update(window);
		render(window);
	}

	Texture::Finalize();
	return 0;
}

/**
* �v���C���[�̓��͂���������.
*
* @param window �Q�[�����Ǘ�����E�B���h�E.
*/
void processInput(GLFWEW::WindowRef window)
{
	window.Update();

	// ���@�̑��x��ݒ肷��.
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

	// �e�̔���.
	if (gamepad.buttonDown & GamePad::A) {
		// �󂢂Ă���e�̍\���̂�����.
		Actor* bullet = nullptr;
		for (Actor* i = std::begin(playerBulletList);
			i != std::end(playerBulletList); ++i) {
			if (i->health <= 0) {
				bullet = i;
				break;
			}
		}
		// �󂢂Ă���\���̂�����������A������g���Ēe�𔭎˂���.
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
* �Q�[���̏�Ԃ��X�V����.
*
* @param window �Q�[�����Ǘ�����E�B���h�E.
*/
void update(GLFWEW::WindowRef window)
{
	const float deltaTime = window.DeltaTime(); // �O��̍X�V����̌o�ߎ���(�b).

	// ���@�̈ړ�.
	if (playerVelocity.x || playerVelocity.y) {
		glm::vec3 newPos = sprPlayer.Position() + playerVelocity * deltaTime;
		// ���@�̈ړ��͈͂���ʓ��ɐ�������.
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

	// �o���܂ł̎��Ԃ�0�ȉ��ɂȂ�����G���o��������.
	enemyGenerationTimer -= deltaTime;
	if (enemyGenerationTimer <= 0) {
		// �󂢂Ă���(�j�󂳂�Ă���)�G�\���̂�����.
		Actor* enemy = nullptr;
		for (Actor* i = std::begin(enemyList); i != std::end(enemyList); ++i) {
			if (i->health <= 0) {
				enemy = i;
				break;
			}
		}
		// �󂢂Ă���\���̂�����������A������g���ēG���o��������.
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
			// ���̓G���o������܂ł̎��Ԃ�ݒ肷��.
			const std::uniform_real_distribution<float> time_distribution(0.5f, 4.0f);
			enemyGenerationTimer = time_distribution(random);
		}
	}
	// �G�̍X�V.
	for (Actor* i = std::begin(enemyList); i != std::end(enemyList); ++i) {
		if (i->health > 0) {
			i->spr.Update(deltaTime);
			if (i->spr.Tweener()->IsFinished()) {
				i->health = 0;
			}
		}
	}
	// ���@�̒e�̍X�V.
	for (Actor* i = std::begin(playerBulletList); i != std::end(playerBulletList); ++i) {
		if (i->health > 0) {
			i->spr.Update(deltaTime);
			if (i->spr.Tweener()->IsFinished()) {
				i->health = 0;
			}
		}
	}

	// ���@�̒e�ƓG�̏Փ˔���.
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
* �Q�[���̏�Ԃ�`�悷��.
*
* @param window �Q�[�����Ǘ�����E�B���h�E.
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
* 2�̒����`�̏Փˏ�Ԃ𒲂ׂ�.
*
* @param lhs �����`����1.
* @param rhs �����`����2.
*
* @retval true  �Փ˂��Ă���.
* @retval false �Փ˂��Ă��Ȃ�.
*/
bool detectCollision(const Rect* lhs, const Rect* rhs)
{
	return
		lhs->origin.x < rhs->origin.x + rhs->size.x &&
		lhs->origin.x + lhs->size.x > rhs->origin.x &&
		lhs->origin.y < rhs->origin.y + rhs->size.y &&
		lhs->origin.y + lhs->size.y > rhs->origin.y;
}
