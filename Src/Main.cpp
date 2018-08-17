/**
* @file Main.cpp
*/
#include "GLFWEW.h"
#include "Texture.h"
#include "Sprite.h"
#include "Font.h"
#include "TiledMap.h"
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
FontRenderer fontRenderer; // �t�H���g�`��p�ϐ�.
Sprite sprBackground; // �w�i�p�X�v���C�g.
Sprite sprPlayer;     // ���@�p�X�v���C�g.

glm::vec3 playerVelocity; // ���@�̈ړ����x.

Actor enemyList[128]; // �G�̃��X�g.
Actor playerBulletList[128]; // ���@�̒e�̃��X�g.
Actor effectList[128]; // �����Ȃǂ̓�����ʗp�X�v���C�g�̃��X�g.
float enemyGenerationTimer; // ���̓G���o������܂ł̎���(�P��:�b).
int score; // �v���C���[�̓��_.

// �G�̏o���𐧌䂷�邽�߂̃f�[�^.
TiledMap enemyMap;
float mapCurrentPosX;
float mapProcessedX;

// �G�̃A�j���[�V����.
const FrameAnimation::KeyFrame enemyKeyFrames[] = {
	{ 0.000f, glm::vec2(480, 0), glm::vec2(32, 32) },
	{ 0.125f, glm::vec2(480, 96), glm::vec2(32, 32) },
	{ 0.250f, glm::vec2(480, 64), glm::vec2(32, 32) },
	{ 0.375f, glm::vec2(480, 32), glm::vec2(32, 32) },
	{ 0.500f, glm::vec2(480, 0), glm::vec2(32, 32) },
};
FrameAnimation::TimelinePtr tlEnemy;

// ���@�̃A�j���[�V����.
const FrameAnimation::KeyFrame playerKeyFrames[] = {
	{ 0.000f, glm::vec2(0, 0), glm::vec2(64, 32) },
	{ 0.125f, glm::vec2(0, 32), glm::vec2(64, 32) },
	{ 0.250f, glm::vec2(0, 64), glm::vec2(64, 32) },
	{ 0.375f, glm::vec2(0, 0), glm::vec2(64, 32) },
};
FrameAnimation::TimelinePtr tlPlayer;

/*
* �v���g�^�C�v�錾.
*/
void processInput(GLFWEW::WindowRef);
void update(GLFWEW::WindowRef);
void render(GLFWEW::WindowRef);
bool detectCollision(const Rect*, const Rect*);
void initializeActorList(Actor*, Actor*);
void updateActorList(Actor*, Actor*, float);
void renderActorList(const Actor*, const Actor*);
Actor* findAvailableActor(Actor*, Actor*);
using CollisionHandlerType = void(*)(Actor*, Actor*); // �Փˏ����֐��̌^.
void detectCollision(Actor*, Actor*, Actor*, Actor*, CollisionHandlerType);
void playerBulletAndEnemyContactHandler(Actor*, Actor*);

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
	if (!fontRenderer.Initialize(1024, glm::vec2(windowWidth, windowHeight))) {
		return 1;
	}
	if (!fontRenderer.LoadFromFile("Res/Font/makinas_scrap.fnt")) {
		return 1;
	}

	random.seed(std::random_device()()); // �����G���W���̏�����.

	// �A�j���[�V�����E�^�C�����C���̍쐬.
	tlEnemy = FrameAnimation::Timeline::Create(enemyKeyFrames);
	tlPlayer = FrameAnimation::Timeline::Create(playerKeyFrames);

	//�X�v���C�g�ɉ摜��ݒ�.
	sprBackground = Sprite("Res/UnknownPlanet.png");
	sprPlayer = Sprite("Res/Objects.png", glm::vec3(0, 0, 0), Rect(0, 0, 64, 32));
	sprPlayer.Animator(FrameAnimation::Animate::Create(tlPlayer));

	initializeActorList(std::begin(enemyList), std::end(enemyList));
	initializeActorList(std::begin(playerBulletList), std::end(playerBulletList));
	initializeActorList(std::begin(effectList), std::end(effectList));

	enemyGenerationTimer = 2;
	score = 0;

	// �G�z�u�}�b�v��ǂݍ���.
	enemyMap.Load("Res/EnemyMap.json");
	mapCurrentPosX = windowWidth;
	mapProcessedX = windowWidth;

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
		Actor* bullet = findAvailableActor(std::begin(playerBulletList), std::end(playerBulletList));
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

	// �G�̏o��.
#if 1
	const TiledMap::Layer& tiledMapLayer = enemyMap.GetLayer(0);
	const glm::vec2 tileSize = enemyMap.GetTileSet(tiledMapLayer.tilesetNo).size;
	// �G�z�u�}�b�v�Q�ƈʒu�̍X�V.
	const float enemyMapScrollSpeed = 100; // �X�V���x.
	mapCurrentPosX += enemyMapScrollSpeed * deltaTime;
	if (mapCurrentPosX >= tiledMapLayer.size.x * tileSize.x) {
		// �I�[�𒴂�����擪�Ƀ��[�v.
		mapCurrentPosX = 0;
		mapProcessedX = 0;
	}
	// ���̗�ɓ��B������f�[�^��ǂ�.
	if (mapCurrentPosX - mapProcessedX >= tileSize.x) {
		mapProcessedX += tileSize.x;
		const int mapX = static_cast<int>(mapProcessedX / tileSize.x);
		for (int mapY = 0; mapY < tiledMapLayer.size.y; ++mapY) {
			const int enemyId = 256; // �G�Ƃ݂Ȃ��^�C��ID.
			if (tiledMapLayer.At(mapY, mapX) == enemyId) {
				Actor* enemy = findAvailableActor(std::begin(enemyList), std::end(enemyList));
				// �󂢂Ă���\���̂�����������A������g���ēG���o��������.
				if (enemy != nullptr) {
					const float y = windowHeight * 0.5f - static_cast<float>(mapY * tileSize.x);
					enemy->spr = Sprite("Res/Objects.png", glm::vec3(0.5f * windowWidth, y, 0), Rect(480, 0, 32, 32));
					enemy->spr.Animator(FrameAnimation::Animate::Create(tlEnemy));
					namespace TA = TweenAnimation;
					TA::SequencePtr seq = TA::Sequence::Create(4);
					seq->Add(TA::MoveBy::Create(1, glm::vec3(0, 100, 0), TA::EasingType::EaseInOut, TA::Target::Y));
					seq->Add(TA::MoveBy::Create(1, glm::vec3(0, -100, 0), TA::EasingType::EaseInOut, TA::Target::Y));
					TA::ParallelizePtr par = TA::Parallelize::Create(1);
					par->Add(seq);
					par->Add(TA::MoveBy::Create(8, glm::vec3(-1000, 0, 0), TA::EasingType::Linear, TA::Target::X));
					enemy->spr.Tweener(TA::Animate::Create(par));
					enemy->collisionShape = Rect(-16, -16, 32, 32);
					enemy->health = 2;
				}
			}
		}
	}
#else
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
			// �t���[���A�j���[�V�����̐ݒ�.
			enemy->spr.Animator(FrameAnimation::Animate::Create(tlEnemy));
			// �g�E�B�[�j���O�̐ݒ�.
			namespace TA = TweenAnimation;
			TA::SequencePtr seq = TA::Sequence::Create(1);
			seq->Add(TA::MoveBy::Create(1, glm::vec3(-400, 0, 0), TA::EasingType::EaseOut, TA::Target::X));
			seq->Add(TA::MoveBy::Create(1, glm::vec3(400, 0, 0), TA::EasingType::EaseIn, TA::Target::X));
			TA::ParallelizePtr par = TA::Parallelize::Create(1);
			par->Add(seq);
			par->Add(TA::MoveBy::Create(2, glm::vec3(0, -300, 0), TA::EasingType::Linear, TA::Target::Y));
			enemy->spr.Tweener(TA::Animate::Create(par));
			enemy->collisionShape = Rect(-16, -16, 32, 32);
			enemy->health = 3;
			// ���̓G���o������܂ł̎��Ԃ�ݒ肷��.
			const std::uniform_real_distribution<float> time_distribution(0.5f, 4.0f);
			enemyGenerationTimer = time_distribution(random);
		}
	}
#endif
	// Actor�̍X�V.
	updateActorList(std::begin(enemyList), std::end(enemyList), deltaTime);
	updateActorList(std::begin(playerBulletList), std::end(playerBulletList), deltaTime);
	updateActorList(std::begin(effectList), std::end(effectList), deltaTime);

	// ���@�̒e�ƓG�̏Փ˔���.
	detectCollision(
		std::begin(playerBulletList), std::end(playerBulletList),
		std::begin(enemyList), std::end(enemyList),
		playerBulletAndEnemyContactHandler);
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
	renderActorList(std::begin(enemyList), std::end(enemyList));
	renderActorList(std::begin(playerBulletList), std::end(playerBulletList));
	renderActorList(std::begin(effectList), std::end(effectList));
	renderer.EndUpdate();
	renderer.Draw(glm::vec2(windowWidth, windowHeight));

	// �������\������.
	fontRenderer.BeginUpdate();
	char str[9];
	snprintf(str, sizeof(str), "%08d", score);
	fontRenderer.AddString(glm::vec2(-100, 300), str);
	fontRenderer.EndUpdate();
	fontRenderer.Draw();

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

/**
* Actor�̔z�������������.
*
* @param  first  ����������z��̐擪.
* @param  last    ����������z��̏I�[.
*/
void initializeActorList(Actor* first, Actor* last)
{
	for (Actor* i = first; i != last; ++i) {
		i->health = 0;
	}
}

/**
* Actor�̔z����X�V����.
*
* @param  first      �X�V����z��̐擪.
* @param  last       �X�V����z��̏I�[.
* @param  deltaTime �O��̍X�V����̌o�ߎ���.
*/
void updateActorList(Actor* first, Actor* last, float deltaTime)
{
	for (Actor* i = first; i != last; ++i) {
		if (i->health > 0) {
			i->spr.Update(deltaTime);
			if (i->spr.Tweener()->IsFinished()) {
				i->health = 0;
			}
		}
	}
}

/**
* Actor�̔z���`�悷��.
*
* @param  first      �X�V����z��̐擪.
* @param  last       �X�V����z��̏I�[.
*/
void renderActorList(const Actor* first, const Actor* last)
{
	for (const Actor* i = first; i != last; ++i) {
		if (i->health > 0) {
			renderer.AddVertices(i->spr);
		}
	}
}

/**
* ���p�\�Ȃ�Actor���擾����.
*
* @param first �����Ώۂ̐擪�v�f�̃|�C���^.
* @param last  �����Ώۂ̏I�[�v�f�̃|�C���^.
*
* @return ���p�\��Actor�̃|�C���^.
*         ���p�\��Actor��������Ȃ����nullptr.
*
* [first, last)�͈̔͂���A���p�\��(health��0�ȉ���)Actor����������.
*/
Actor* findAvailableActor(Actor* first, Actor* last)
{
	Actor* result = nullptr;
	for (Actor* i = first;
		i != last; ++i) {
		if (i->health <= 0) {
			result = i;
			break;
		}
	}
	return result;
}

/**
* �Փ˂����o����.
*
* @param firstA    �Փ˂�����z��A�̐擪�|�C���^.
* @param lastA     �Փ˂�����z��A�̏I�[�|�C���^.
* @param firstB    �Փ˂�����z��B�̐擪�|�C���^.
* @param lastB     �Փ˂�����z��B�̏I�[�|�C���^.
* @param handler   A-B�ԂŏՓ˂����o���ꂽ�Ƃ��Ɏ��s����֐�.
*/
void detectCollision(Actor* firstA, Actor* lastA, Actor* firstB, Actor* lastB, CollisionHandlerType handler)
{
	for (Actor* a = firstA; a != lastA; ++a) {
		if (a->health <= 0) {
			continue;
		}
		Rect rectA = a->collisionShape;
		rectA.origin += glm::vec2(a->spr.Position());
		for (Actor* b = firstB; b != lastB; ++b) {
			if (b->health <= 0) {
				continue;
			}
			Rect rectB = b->collisionShape;
			rectB.origin += glm::vec2(b->spr.Position());
			if (detectCollision(&rectA, &rectB)) {
				handler(a, b);
				if (a->health <= 0) {
					break;
				}
			}
		}
	}
}

/**
* ���@�̒e�ƓG�̏Փ˂���������.
*
* @param bullet ���@�̒e�̃|�C���^.
* @param enemy  �G�̃|�C���^.
*/
void playerBulletAndEnemyContactHandler(Actor* bullet, Actor* enemy)
{
	const int tmp = bullet->health;
	bullet->health -= enemy->health;
	enemy->health -= tmp;
	score += 100; // �G��j�󂵂��瓾�_�𑝂₷.
}
