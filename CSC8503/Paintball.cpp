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
	if (object->GetName() == "Floor")
	{
		this->GetTransform().SetPosition(Vector3(1000, 1000, 1000));
		std::cout << "Paintball hit floor";
		//add paint splat logic here
		this->SetActive(false);
	}
}

bool NCL::CSC8503::Paintball::isRed()
{
	return red;
}

void NCL::CSC8503::Paintball::toggleRed()
{
	red = !red;
}
