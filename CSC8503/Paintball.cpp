#include "Paintball.h"

NCL::CSC8503::Paintball::Paintball()
{
}

NCL::CSC8503::Paintball::~Paintball()
{
}

void NCL::CSC8503::Paintball::Update(float dt)
{
}

void NCL::CSC8503::Paintball::OnCollisionBegin(GameObject* object)
{
	// if object wall/slash floor paint
	// if object enemy player do damage
	delete this;
}

bool NCL::CSC8503::Paintball::isRed()
{
	return red;
}

void NCL::CSC8503::Paintball::toggleRed()
{
	red = !red;
}
