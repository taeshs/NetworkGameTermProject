#pragma once
#include "Global.h"
#include <math.h>

class Player
{
public:
	Player();
	Player(float, float);
	~Player();

	void move(int x, int y, float deltaTime);
	bool shoot(int stX, int stY, int dstX, int dstY, float deltaTime);

	void setPos(float x, float y);

	float getX() { return posX; }
	float getY() { return posY; }



	int getMp() { return mp; }

	void addMp() { if (mp < maxMp) { mp++; } }
	void subMp(int val);

	int getBulletCost() { return nowBullet.bType; }

	void setBullet(int val) {
		if (val == 1)nowBullet = bullet1;
		if (val == 2)nowBullet = bullet2;
		if (val == 3)nowBullet = bullet3;
	}

	Bullet bullet1 = {2, 10, 200, 10};	// size spd dmg
	Bullet bullet2 = {3, 20, 200, 20};
	Bullet bullet3 = {4, 10, 400, 20};

	int maxBulletCnt = 10;
	Bullet bullets[10];
	int bulletCount = 0;

	int maxHp;
	int maxMp;

	HBITMAP bitmap;

private:
	float playerNo = 0;
	float posX;
	float posY;

	int hp;
	int mp;

	float velocity = 100.f;

	Bullet nowBullet = bullet1;


	//BoundingBox bb;
};