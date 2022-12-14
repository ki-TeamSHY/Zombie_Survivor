#pragma once
#include "Weapon.h"

class Player;

class Pistol : public Weapon
{
public:
	Pistol(Player* player);
	virtual ~Pistol();

	void Init();
	virtual void Reset() override;
	virtual void Update(float dt) override;
	virtual void Draw(RenderWindow& window) override;

	virtual void Fire();
};