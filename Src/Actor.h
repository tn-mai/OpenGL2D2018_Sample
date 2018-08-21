#pragma once
/**
* @file Actor.h
*
* Actor構造体およびActor構造体に関連する関数を宣言するヘッダファイル.
*/
#include "Sprite.h"

/**
* ゲームキャラクター構造体.
*/
struct Actor
{
	Sprite spr; // 画像表示用スプライト.
	Rect collisionShape; // 衝突判定の位置と大きさ.
	int health; // 耐久力(0以下なら破壊されている).
};
bool detectCollision(const Rect*, const Rect*);
void initializeActorList(Actor*, Actor*);
void updateActorList(Actor*, Actor*, float);
void renderActorList(const Actor*, const Actor*);
Actor* findAvailableActor(Actor*, Actor*);
using CollisionHandlerType = void(*)(Actor*, Actor*); // 衝突処理関数の型.
void detectCollision(Actor*, Actor*, Actor*, Actor*, CollisionHandlerType);
