/**
* @file Actor.cpp
*/
#include "Actor.h"
#include "GameData.h"

/**
* 2つの長方形の衝突状態を調べる.
*
* @param lhs 長方形その1.
* @param rhs 長方形その2.
*
* @retval true  衝突している.
* @retval false 衝突していない.
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
* Actorの配列を初期化する.
*
* @param  first  初期化する配列の先頭.
* @param  last    初期化する配列の終端.
*/
void initializeActorList(Actor* first, Actor* last)
{
	for (Actor* i = first; i != last; ++i) {
		i->health = 0;
	}
}

/**
* Actorの配列を更新する.
*
* @param  first      更新する配列の先頭.
* @param  last       更新する配列の終端.
* @param  deltaTime 前回の更新からの経過時間.
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
* Actorの配列を描画する.
*
* @param  first      更新する配列の先頭.
* @param  last       更新する配列の終端.
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
* 利用可能なのActorを取得する.
*
* @param first 検索対象の先頭要素のポインタ.
* @param last  検索対象の終端要素のポインタ.
*
* @return 利用可能なActorのポインタ.
*         利用可能なActorが見つからなければnullptr.
*
* [first, last)の範囲から、利用可能な(healthが0以下の)Actorを検索する.
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
* 衝突を検出する.
*
* @param firstA    衝突させる配列Aの先頭ポインタ.
* @param lastA     衝突させる配列Aの終端ポインタ.
* @param firstB    衝突させる配列Bの先頭ポインタ.
* @param lastB     衝突させる配列Bの終端ポインタ.
* @param handler   A-B間で衝突が検出されたときに実行する関数.
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
