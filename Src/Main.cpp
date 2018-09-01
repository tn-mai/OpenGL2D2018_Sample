/**
* @file Main.cpp
*/
#include "MainScene.h"
#include "TitleScene.h"
#include "GameOverScene.h"
#include "GameData.h"
#include "Actor.h"
#include "GLFWEW.h"
#include "Texture.h"
#include "Sprite.h"
#include "Font.h"
#include "TiledMap.h"
#include "Audio.h"
#include <random>
#include <glm/gtc/matrix_transform.hpp>

const char windowTitle[] = "OpenGL2D 2018"; // �^�C�g���o�[�ɕ\������镶��.

std::mt19937 random; // �����𔭐�������ϐ�(�����G���W��).

SpriteRenderer renderer; // �X�v���C�g��`�悷��I�u�W�F�N�g.
FontRenderer fontRenderer; // �t�H���g�`��p�ϐ�.
Sprite sprBackground; // �w�i�p�X�v���C�g.
Actor sprPlayer;     // ���@�p�X�v���C�g.

glm::vec3 playerVelocity; // ���@�̈ړ����x.

Actor enemyList[128]; // �G�̃��X�g.
Actor playerBulletList[128]; // ���@�̒e�̃��X�g.
Actor playerLaserList[3]; // ���@�̃��[�U�[�̃��X�g.
Actor effectList[128]; // �����Ȃǂ̓�����ʗp�X�v���C�g�̃��X�g.
Actor itemList[64]; // �A�C�e���p�X�v���C�g�̃��X�g.
float enemyGenerationTimer; // ���̓G���o������܂ł̎���(�P��:�b).
int score; // �v���C���[�̓��_.
float timer; // �V�[���؂�ւ��Ŏg�p����^�C�}�[.
const int weaponLevelMin = 0; // ���@�̕��틭���̍Œ�i�K.
const int weaponLevelMax = 3; // ���@�̕��틭���̍ō��i�K.
int weaponLevel; // ���@�̕��틭���i�K.

// ��������p�ϐ�.
Audio::SoundPtr bgm;
Audio::SoundPtr sePlayerShot;
Audio::SoundPtr seBlast;
Audio::SoundPtr sePowerUp;

// �Q�[���̏��.
int gamestate; // ���s���̏��ID.

TitleScene titleScene;
MainScene mainScene;
GameOverScene gameOverScene;

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

// �����A�j���[�V����.
const FrameAnimation::KeyFrame blastKeyFrames[] = {
	{ 0 / 60.0f, glm::vec2(416, 0), glm::vec2(32, 32) },
	{ 5 / 60.0f, glm::vec2(416, 32), glm::vec2(32, 32) },
	{ 10 / 60.0f, glm::vec2(416, 64), glm::vec2(32, 32) },
	{ 15 / 60.0f, glm::vec2(416, 96), glm::vec2(32, 32) },
	{ 20 / 60.0f, glm::vec2(416, 96), glm::vec2(32, 32) },
};
FrameAnimation::TimelinePtr tlBlast;

/*
* �v���g�^�C�v�錾.
*/
void processInput(GLFWEW::WindowRef);
void update(GLFWEW::WindowRef);
void render(GLFWEW::WindowRef);
void playerBulletAndEnemyContactHandler(Actor*, Actor*);
void playerAndEnemyContactHandler(Actor*, Actor*);
void playerAndItemContactHandler(Actor*, Actor*);

/**
* ���C����ʗp�̍\���̂̏����ݒ���s��.
*
* @param scene     ���C����ʗp�\���̂̃|�C���^.
*
* @retval true  ����������.
* @retval false ���������s.
*/
bool initialize(MainScene* scene)
{
	// �Q�[���̏����ݒ���s��.

	//�X�v���C�g�ɉ摜��ݒ�.
	sprBackground = Sprite("Res/UnknownPlanet.png");
	sprPlayer.spr = Sprite("Res/Objects.png", glm::vec3(0, 0, 0), Rect(0, 0, 64, 32));
	sprPlayer.spr.Animator(FrameAnimation::Animate::Create(tlPlayer));
	sprPlayer.collisionShape = Rect(-24, -8, 48, 16);
	sprPlayer.health = 1;

	initializeActorList(std::begin(enemyList), std::end(enemyList));
	initializeActorList(std::begin(playerBulletList), std::end(playerBulletList));
	initializeActorList(std::begin(playerLaserList), std::end(playerLaserList));
	initializeActorList(std::begin(effectList), std::end(effectList));
	initializeActorList(std::begin(itemList), std::end(itemList));

	enemyGenerationTimer = 2;
	score = 0;
	weaponLevel = weaponLevelMin;
	timer = 0;

	// �G�z�u�}�b�v��ǂݍ���.
	enemyMap.Load("Res/EnemyMap.json");
	mapCurrentPosX = windowWidth;
	mapProcessedX = windowWidth;

	// ��������������.
	Audio::EngineRef audio = Audio::Engine::Instance();
	seBlast = audio.Prepare("Res/Audio/Blast.xwm");
	sePlayerShot = audio.Prepare("Res/Audio/PlayerShot.xwm");
	sePowerUp = audio.Prepare("Res/Audio/GetItem.xwm");
	bgm = audio.Prepare("Res/Audio/Neolith.xwm");
	// BGM�����[�v�Đ�����.
	bgm->Play(Audio::Flag_Loop);

	return true;
}

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
	// �����Đ��V�X�e���̏�����.
	Audio::EngineRef audio = Audio::Engine::Instance();
	if (!audio.Initialize()) {
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
	tlBlast = FrameAnimation::Timeline::Create(blastKeyFrames);

	// �^�C�g����ʂ�����������.
	gamestate = gamestateTitle;
	initialize(&titleScene);

	// �Q�[�����[�v.
	while (!window.ShouldClose()) {
		processInput(window);
		update(window);
		render(window);
		audio.Update();
	}

	Texture::Finalize();
	audio.Destroy();
	return 0;
}

/**
* �v���C���[�̓��͂���������.
*
* @param window �Q�[�����Ǘ�����E�B���h�E.
*/
void processInput(GLFWEW::WindowRef window)
{
	if (gamestate == gamestateTitle) {
		processInput(window, &titleScene);
		return;
	} else if (gamestate == gamestateGameOver) {
		processInput(window, &gameOverScene);
		return;
	}

	window.Update();

	if (sprPlayer.health <= 0) {
		playerVelocity = glm::vec3(0, 0, 0);
	} else {
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
			for (int i = 0; i < weaponLevel * 2 + 1; ++i) {
				Actor* bullet = findAvailableActor(std::begin(playerBulletList), std::end(playerBulletList));
				// �󂢂Ă���\���̂�����������A������g���Ēe�𔭎˂���.
				if (bullet != nullptr) {
					bullet->spr = Sprite("Res/Objects.png", sprPlayer.spr.Position(), Rect(64, 0, 32, 16));
					const float angles[] = { 0.0f, -7.5f, 7.5f, -15.0f, 15.0f, -22.5f, 22.5f };
					const glm::vec3 v = glm::rotate(glm::mat4(), glm::radians(angles[i]), glm::vec3(0, 0, 1)) * glm::vec4(1200, 0, 0, 1);
					bullet->spr.Tweener(TweenAnimation::Animate::Create(
						TweenAnimation::MoveBy::Create(1, v,
						TweenAnimation::EasingType::Linear)));
					bullet->spr.Rotation(glm::radians(angles[i]));
					bullet->collisionShape = Rect(-8, -4, 16, 8);
					bullet->health = 4;
					sePlayerShot->Play(); // �e�̔��ˉ����Đ�.
				}
			}
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

	if (gamestate == gamestateTitle) {
		update(window, &titleScene);
		return;
	} else if (gamestate == gamestateMain) {
		// ���@���j�󂳂�Ă�����Q�[���I�[�o�[��ʂ�\������.
		if (sprPlayer.health <= 0) {
			if (timer > 0) {
				timer -= deltaTime;
			} else {
				bgm->Stop(); // BGM���~����.
				gamestate = gamestateGameOver;
				initialize(&gameOverScene);
				return;
			}
		}
	} else if (gamestate == gamestateGameOver) {
		update(window, &gameOverScene);
		return;
	}

	// ���@�̈ړ�.
	if (sprPlayer.health > 0) {
		if (playerVelocity.x || playerVelocity.y) {
			glm::vec3 newPos = sprPlayer.spr.Position() + playerVelocity * deltaTime;
			// ���@�̈ړ��͈͂���ʓ��ɐ�������.
			const Rect playerRect = sprPlayer.spr.Rectangle();
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
			sprPlayer.spr.Position(newPos);
		}
		sprPlayer.spr.Update(deltaTime);
	}
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
			const int powerUpItemId = 230; // �p���[�A�b�v�A�C�e����ID.
			const int tileId = tiledMapLayer.At(mapY, mapX);
			if (tileId == enemyId) {
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
			} else if (tileId == powerUpItemId) {
				Actor* item = findAvailableActor(std::begin(itemList), std::end(itemList));
				if (item != nullptr) {
					const float y = windowHeight * 0.5f - static_cast<float>(mapY * tileSize.x);
					item->spr = Sprite("Res/Objects.png", glm::vec3(0.5f * windowWidth, y, 0), Rect(160, 32, 32, 32));
					namespace TA = TweenAnimation;
					item->spr.Tweener(TA::Animate::Create(TA::MoveBy::Create(16, glm::vec3(-1000, 0, 0))));
					item->collisionShape = Rect(-16, -16, 32, 32);
					item->health = 1;
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
	updateActorList(std::begin(playerLaserList), std::end(playerLaserList), deltaTime);
	updateActorList(std::begin(effectList), std::end(effectList), deltaTime);
	updateActorList(std::begin(itemList), std::end(itemList), deltaTime);

	// ���@�̒e�ƓG�̏Փ˔���.
	detectCollision(
		std::begin(playerBulletList), std::end(playerBulletList),
		std::begin(enemyList), std::end(enemyList),
		playerBulletAndEnemyContactHandler);
	// ���@�ƓG�̏Փ˔���.
	detectCollision(
		&sprPlayer, &sprPlayer + 1,
		std::begin(enemyList), std::end(enemyList),
		playerAndEnemyContactHandler);
	// ���@�ƃA�C�e���̏Փ˔���.
	detectCollision(
		&sprPlayer, &sprPlayer + 1,
		std::begin(itemList), std::end(itemList),
		playerAndItemContactHandler);
}

/**
* �Q�[���̏�Ԃ�`�悷��.
*
* @param window �Q�[�����Ǘ�����E�B���h�E.
*/
void render(GLFWEW::WindowRef window)
{
	if (gamestate == gamestateTitle) {
		render(window, &titleScene);
		return;
	} else if (gamestate == gamestateGameOver) {
		render(window, &gameOverScene);
		return;
	}

	renderer.BeginUpdate();
	renderer.AddVertices(sprBackground);
	if (sprPlayer.health > 0) {
		renderer.AddVertices(sprPlayer.spr);
	}
	renderActorList(std::begin(enemyList), std::end(enemyList));
	renderActorList(std::begin(playerBulletList), std::end(playerBulletList));
	renderActorList(std::begin(playerLaserList), std::end(playerLaserList));
	renderActorList(std::begin(effectList), std::end(effectList));
	renderActorList(std::begin(itemList), std::end(itemList));
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
	if (enemy->health <= 0) {
		score += 100;
		// ������\������.
		Actor* blast = findAvailableActor(std::begin(effectList), std::end(effectList));
		if (blast != nullptr) {
			blast->spr = Sprite("Res/Objects.png", enemy->spr.Position());
			blast->spr.Animator(FrameAnimation::Animate::Create(tlBlast));
			namespace TA = TweenAnimation;
			blast->spr.Tweener(TA::Animate::Create(TA::Rotation::Create(20 / 60.0f, 1.5f)));
			blast->health = 1;
			seBlast->Play();// ���������Đ�.
		}
	}
}

/**
* ���@�ƓG�̏Փ˂���������.
*
* @param player ���@�̃|�C���^.
* @param enemy  �G�̃|�C���^.
*/
void playerAndEnemyContactHandler(Actor* player, Actor* enemy)
{
	const int tmp = player->health;
	player->health -= enemy->health;
	enemy->health -= tmp;
	if (enemy->health <= 0) {
		score += 100;
		Actor* blast = findAvailableActor(std::begin(effectList), std::end(effectList));
		if (blast != nullptr) {
			blast->spr = Sprite("Res/Objects.png", enemy->spr.Position());
			blast->spr.Animator(FrameAnimation::Animate::Create(tlBlast));
			namespace TA = TweenAnimation;
			blast->spr.Tweener(TA::Animate::Create(TA::Rotation::Create(20 / 60.0f, 1.5f)));
			blast->health = 1;
		}
	}
	if (player->health <= 0) {
		timer = 2;
		Actor* blast = findAvailableActor(
			std::begin(effectList), std::end(effectList));
		if (blast != nullptr) {
			blast->spr = Sprite("Res/Objects.png", player->spr.Position());
			blast->spr.Animator(FrameAnimation::Animate::Create(tlBlast));
			namespace TA = TweenAnimation;
			blast->spr.Tweener(TA::Animate::Create(TA::Rotation::Create(20 / 60.0f, 1.5f)));
			blast->spr.Scale(glm::vec2(2, 2)); // ���@�̔����͏����傫�߂ɂ���.
			blast->health = 1;
			seBlast->Play();// ���������Đ�.
		}
	}
}

/**
* ���@�ƃA�C�e���̏Փ˂���������.
*
* @param player ���@�̃|�C���^.
* @param item   �A�C�e���̃|�C���^.
*/
void playerAndItemContactHandler(Actor* player, Actor* item)
{
	item->health = 0;
	sePowerUp->Play();

	// ���@�̕������������.
	++weaponLevel;
	if (weaponLevel > weaponLevelMax) {
		weaponLevel = weaponLevelMax;
		score += 1000;
	}
}