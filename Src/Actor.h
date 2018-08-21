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