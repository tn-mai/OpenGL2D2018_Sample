/**
* @file Actor.cpp
*/
#include "Actor.h"
#include "GameData.h"

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
			if (i->spr.Tweener() && i->spr.Tweener()->IsFinished()) {
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
