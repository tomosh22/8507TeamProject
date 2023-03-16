#include "PhysicsSystem.h"
#include "PhysicsObject.h"
#include "GameObject.h"
#include "CollisionDetection.h"
#include "Quaternion.h"

#include "Constraint.h"

#include "Debug.h"
#include "Window.h"
#include <functional>
#include<algorithm>
#include"Projectile.h"
#include"NetworkedGame.h"

using namespace NCL;
using namespace CSC8503;

PhysicsSystem::PhysicsSystem(GameWorld& g) : gameWorld(g)	{
	applyGravity	= true;
	useBroadPhase	= false;	
	dTOffset		= 0.0f;
	globalDamping	= 0.995f;
	SetGravity(Vector3(0.0f, -9.8f, 0.0f));
}

PhysicsSystem::~PhysicsSystem()	{
}

void PhysicsSystem::SetGravity(const Vector3& g) {
	gravity = g;
}

/*

If the 'game' is ever reset, the PhysicsSystem must be
'cleared' to remove any old collisions that might still
be hanging around in the collision list. If your engine
is expanded to allow objects to be removed from the world,
you'll need to iterate through this collisions list to remove
any collisions they are in.

*/
void PhysicsSystem::Clear() {
	allCollisions.clear();

}

/*

This is the core of the physics engine update

*/

bool useSimpleContainer = false;

int constraintIterationCount = 10;

//This is the fixed timestep we'd LIKE to have
const int   idealHZ = 120;
const float idealDT = 1.0f / idealHZ;

/*
This is the fixed update we actually have...
If physics takes too long it starts to kill the framerate, it'll drop the 
iteration count down until the FPS stabilises, even if that ends up
being at a low rate. 
*/
int realHZ		= idealHZ;
float realDT	= idealDT;

void PhysicsSystem::Update(float dt) {	
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::B)) {
		useBroadPhase = !useBroadPhase;
		std::cout << "Setting broadphase to " << useBroadPhase << std::endl;
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::N)) {
		useSimpleContainer = !useSimpleContainer;
		std::cout << "Setting broad container to " << useSimpleContainer << std::endl;
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::I)) {
		constraintIterationCount--;
		std::cout << "Setting constraint iterations to " << constraintIterationCount << std::endl;
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::O)) {
		constraintIterationCount++;
		std::cout << "Setting constraint iterations to " << constraintIterationCount << std::endl;
	}

	dTOffset += dt; //We accumulate time delta here - there might be remainders from previous frame!

	GameTimer t;
	t.GetTimeDeltaSeconds();

	if (useBroadPhase) {
		UpdateObjectAABBs();
	}
	int iteratorCount = 0;
	while(dTOffset > realDT) {
		IntegrateAccel(realDT); //Update accelerations from external forces
		if (useBroadPhase) {
			BroadPhase();
			NarrowPhase();
		}
		else {
			BasicCollisionDetection();
			/*std::cout << BasicCollisionDetection().contactPosition << std::endl;
			std::cout << BasicCollisionDetection().paintRadius << std::endl;*/
		}

		//This is our simple iterative solver - 
		//we just run things multiple times, slowly moving things forward
		//and then rechecking that the constraints have been met		
		float constraintDt = realDT /  (float)constraintIterationCount;
		for (int i = 0; i < constraintIterationCount; ++i) {
			UpdateConstraints(constraintDt);	
		}
		IntegrateVelocity(realDT); //update positions from new velocity changes

		dTOffset -= realDT;
		iteratorCount++;
	}

	ClearForces();	//Once we've finished with the forces, reset them to zero

	UpdateCollisionList(); //Remove any old collisions

	t.Tick();
	float updateTime = t.GetTimeDeltaSeconds();

	//Uh oh, physics is taking too long...
	if (updateTime > realDT) {
		realHZ /= 2;
		realDT *= 2;
		std::cout << "Dropping iteration count due to long physics time...(now " << realHZ << ")\n";
	}
	else if(dt*2 < realDT) { //we have plenty of room to increase iteration count!
		int temp = realHZ;
		realHZ *= 2;
		realDT /= 2;

		if (realHZ > idealHZ) {
			realHZ = idealHZ;
			realDT = idealDT;
		}
		if (temp != realHZ) {
			std::cout << "Raising iteration count due to short physics time...(now " << realHZ << ")\n";
		}
	}
}

/*
Later on we're going to need to keep track of collisions
across multiple frames, so we store them in a set.

The first time they are added, we tell the objects they are colliding.
The frame they are to be removed, we tell them they're no longer colliding.

From this simple mechanism, we we build up gameplay interactions inside the
OnCollisionBegin / OnCollisionEnd functions (removing health when hit by a 
rocket launcher, gaining a point when the player hits the gold coin, and so on).
*/
void PhysicsSystem::UpdateCollisionList() {
	for (std::set<CollisionDetection::CollisionInfo>::iterator i = allCollisions.begin(); i != allCollisions.end(); ) {
		if ((*i).framesLeft == numCollisionFrames) {
		
			
			/*if (i->a->GetName() == "Bullet" && i->b->isPaintable)
				NetworkedGame::GetInstance()->DispatchComputeShaderForEachTriangle(i->b,i->a->GetTransform().GetPosition(), 10,  );

			if (i->b->GetName() == "Bullet" && i->a->isPaintable)
				NetworkedGame::GetInstance()->DispatchComputeShaderForEachTriangle(i->a, i->b->GetTransform().GetPosition(), 10, Team::team2);
		*/
			i->a->OnCollisionBegin(i->b);
			i->b->OnCollisionBegin(i->a);
		}

		CollisionDetection::CollisionInfo& in = const_cast<CollisionDetection::CollisionInfo&>(*i);
		in.framesLeft--;

		if ((*i).framesLeft < 0) {
			i->a->OnCollisionEnd(i->b);
			i->b->OnCollisionEnd(i->a);
			i = allCollisions.erase(i);
		}
		else {
			++i;
		}
	}
}

//void PhysicsSystem::UpdateObjectAABBs() {
//	gameWorld.OperateOnContents(
//		[](GameObject* g) {
//			g->UpdateBroadphaseAABB();
//		}
//	);
//}

void PhysicsSystem::UpdateObjectAABBs() {
	std::vector <GameObject*>::const_iterator first;
	std::vector <GameObject*>::const_iterator last;
	gameWorld.GetObjectIterators(first, last);
	for (auto i = first; i != last; ++i) {
		(*i)->UpdateBroadphaseAABB();

	}

}

/*

This is how we'll be doing collision detection in tutorial 4.
We step thorugh every pair of objects once (the inner for loop offset 
ensures this), and determine whether they collide, and if so, add them
to the collision set for later processing. The set will guarantee that
a particular pair will only be added once, so objects colliding for
multiple frames won't flood the set with duplicates.
*/
PhysicsSystem::collisionData PhysicsSystem::BasicCollisionDetection() {
	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;
	gameWorld.GetObjectIterators(first, last);

	for (auto i = first; i != last; i++) {
		if ((*i)->GetPhysicsObject() == nullptr||!(*i)->IsActive()) {
			continue;
		}
		for (auto j = i + 1; j != last; ++j) {
			if ((*j)->GetPhysicsObject() == nullptr||!(*j)->IsActive() || ((*i)->GetLayerMask()==Bullet&& (*j)->GetLayerMask() == Bullet)) {
				continue;
			}
			CollisionDetection::CollisionInfo info;
			if (CollisionDetection::ObjectIntersection(*i, *j, info)) {
				ImpulseResolveCollision(*info.a, *info.b, info.point);
				info.framesLeft = numCollisionFrames;
				allCollisions.insert(info);
				if ((info.a)->GetBoundingVolume()->type == VolumeType::Sphere) {  //means it is a bullet type
					return collisionData{ (info.a)->collisionInfo(),(info.a)->GetTransform().GetPosition() };
				}
				if ((info.b)->GetBoundingVolume()->type == VolumeType::Sphere) {  //means it is a bullet type
					return collisionData{ (info.b)->collisionInfo(),(info.b)->GetTransform().GetPosition() };
				}
			}
		}
	}
	return collisionData{ 0,{} };
}

/*

In tutorial 5, we start determining the correct response to a collision,
so that objects separate back out. 

*/
void PhysicsSystem::ImpulseResolveCollision(GameObject& a, GameObject& b, CollisionDetection::ContactPoint& p) const {
	PhysicsObject* physA = a.GetPhysicsObject();
	PhysicsObject* physB = b.GetPhysicsObject();
	Vector3 relativeA = p.localA;
	Vector3 relativeB = p.localB;

	Vector3 angVelocityA =
		Vector3::Cross(physA->GetAngularVelocity(), relativeA);
	Vector3 angVelocityB =
		Vector3::Cross(physB->GetAngularVelocity(), relativeB);

	Vector3 fullVelocityA = physA->GetLinearVelocity() + angVelocityA;
	Vector3 fullVelocityB = physB->GetLinearVelocity() + angVelocityB;

	Vector3 contactVelocity = fullVelocityB - fullVelocityA;
	p.normal.Normalise();
	//try layer id
	if ((physA->getLayerId() == physB->getLayerId()) && physA->getLayerId() == 2) {
		return;
	}
	// try layer id
	//std::cout << contactVelocity << std::endl; // To stop gravity jittering 
	if (applyGravity && (contactVelocity.Length() < 0.3f) && (p.normal == PhysicsObject::gravityDirection || -p.normal == PhysicsObject::gravityDirection || ((Vector3::Dot(p.normal, PhysicsObject::gravityDirection)) <= 0.99999) || ((Vector3::Dot((-p.normal), PhysicsObject::gravityDirection)) <= 0.99999))) {
		ImpulseResolveStop(a, b, p);
		physA->setFloorContactTrue();
		physB->setFloorContactTrue();
	}
	else
	{
		/*typedef std::numeric_limits< float > dbl;
		std::cout.precision(dbl::max_digits10);
		std::cout << p.normal << std::endl;*/
		ImpulseResolveContinuedResponse(a, b, p);
		if (physA->GetFloorContact()) {
			physA->setFloorContactFalse();
		}
		if (physB->GetFloorContact()) {
			physB->setFloorContactFalse();
		}

	}

}


void PhysicsSystem::ImpulseResolveStop(GameObject& a, GameObject& b, CollisionDetection::ContactPoint& p) const {
	PhysicsObject* physA = a.GetPhysicsObject();
	PhysicsObject* physB = b.GetPhysicsObject();

	Transform& transformA = a.GetTransform();
	Transform& transformB = b.GetTransform();

	

	//testing gost alpha 33333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333
	if (a.GetIsAlpha()) {
		transformB.SetPosition(transformB.GetPosition() + (p.normal * p.penetration * (physB->GetInverseMass())));
		return;
	}
	if (b.GetIsAlpha()) {
		transformA.SetPosition(transformA.GetPosition() + (p.normal * p.penetration * (physA->GetInverseMass())));
		return;
	}
	//testing gost alpha 33333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333

	float totalMass = physA->GetInverseMass() + physB->GetInverseMass();

	if (totalMass == 0) {
		return; //no collision to resolve 
	}
	//seperating the objects out using projection
	transformA.SetPosition(transformA.GetPosition() - (p.normal * p.penetration * (physA->GetInverseMass() / totalMass)));

	transformB.SetPosition(transformB.GetPosition() + (p.normal * p.penetration * (physB->GetInverseMass() / totalMass)));


}


void PhysicsSystem::ImpulseResolveContinuedResponse(GameObject& a, GameObject& b, CollisionDetection::ContactPoint& p) const {
	PhysicsObject* physA = a.GetPhysicsObject();
	PhysicsObject* physB = b.GetPhysicsObject();

	Transform& transformA = a.GetTransform();
	Transform& transformB = b.GetTransform();


	float totalMass = physA->GetInverseMass() + physB->GetInverseMass();

	if (totalMass == 0) {
		return; //no collision to resolve 
	}
	//seperating the objects out using projection
	transformA.SetPosition(transformA.GetPosition() - (p.normal * p.penetration * (physA->GetInverseMass() / totalMass)));

	transformB.SetPosition(transformB.GetPosition() + (p.normal * p.penetration * (physB->GetInverseMass() / totalMass)));
	// dead stop dead end
	Vector3 relativeA = p.localA;
	Vector3 relativeB = p.localB;

	Vector3 angVelocityA =
		Vector3::Cross(physA->GetAngularVelocity(), relativeA);

	Vector3 angVelocityB =
		Vector3::Cross(physB->GetAngularVelocity(), relativeB);

	Vector3 fullVelocityA = physA->GetLinearVelocity() + angVelocityA;
	Vector3 fullVelocityB = physB->GetLinearVelocity() + angVelocityB;

	Vector3 contactVelocity = fullVelocityB - fullVelocityA;

	float impulseForce = Vector3::Dot(contactVelocity, p.normal);

	//inertia effect
	Vector3 inertiaA = Vector3::Cross(physA->GetInertiaTensor() * Vector3::Cross(relativeA, p.normal), relativeA);
	Vector3 inertiaB = Vector3::Cross(physB->GetInertiaTensor() * Vector3::Cross(relativeB, p.normal), relativeB);

	float angularEffect = Vector3::Dot(inertiaA + inertiaB, p.normal);

	float cRestitution = physA->GetCoeficient() + physB->GetCoeficient(); // energy lost

	float j = (-(1.0f + cRestitution) * impulseForce) / (totalMass + angularEffect);
	float FIF = 0.0f;
	float& FIFR = FIF;

	//put in friction for linear movement 
	//if (transformB.GetGoatID() == 7) {
	//	Vector3 floorColisionNormal = { 0,-1,0 };
	//	//Vector3 distanceFromCollision = (transformB.GetPosition() - transformA.GetPosition());
	//	Vector3 distanceFromCollision = { 5.5,5.5,5.5 };
	//	float coefficeintOfRestitution = 0.2f;
	//	float coefficientOfFriction = 0.009f;
	//	float inversePlayerMass = physB->GetInverseMass();
	//	Vector3 distNormDist = Vector3::Cross(distanceFromCollision, Vector3::Cross(distanceFromCollision, floorColisionNormal));
	//	Vector3 frictionTangent = physB->GetLinearVelocity() - (floorColisionNormal * (Vector3::Dot(physB->GetLinearVelocity(), floorColisionNormal)));
	//	float topOfImpulse = -(Vector3::Dot(physB->GetLinearVelocity() * coefficientOfFriction, frictionTangent));
	//	float tenserByVectorNormal = Vector3::Dot((physB->GetInertiaTensor()) * distNormDist, floorColisionNormal);
	//	float bottomOfImpulse = inversePlayerMass + tenserByVectorNormal;
	//	float impulseForce = topOfImpulse / bottomOfImpulse;
	//	FIFR = std::min(impulseForce, (physB->GetForce().Length()));

	//}

	// put in friction for linear movement
	//Get absorbtion amount
	float aForceScaler = 1 - (a.getImpactAbsorbtionAmount());
	float bForceScaler = 1 - (b.getImpactAbsorbtionAmount());
	//get absorbtion amount
	//+ (p.normal * transformA.getFrictionImpulse())
	Vector3 fullImpulse = p.normal * j;
	//Vector3 frictionVector = ((physB->GetLinearVelocity()).Normalised()) * -FIF;
	FIFR = 0;



	physA->ApplyLinearImpulse((-fullImpulse) * aForceScaler);
	physB->ApplyLinearImpulse((fullImpulse)*bForceScaler);
	//physB->ApplyLinearImpulse(frictionVector);


	physA->ApplyAngularImpulse(Vector3::Cross(relativeA, -fullImpulse));
	physB->ApplyAngularImpulse(Vector3::Cross(relativeB, -fullImpulse));
}


/*

Later, we replace the BasicCollisionDetection method with a broadphase
and a narrowphase collision detection method. In the broad phase, we
split the world up using an acceleration structure, so that we can only
compare the collisions that we absolutely need to. 

*/
void PhysicsSystem::BroadPhase() {
	broadphaseCollisions.clear();
	QuadTree<GameObject*> tree(Vector2(1024, 1024), 7, 6);

	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;
	gameWorld.GetObjectIterators(first, last);
	for (auto i = first; i != last; ++i) {
		Vector3 halfSizes;
		if (!(*i)->GetBroadphaseAABB(halfSizes)) {
			continue;
		}
		Vector3 pos = (*i)->GetTransform().GetPosition();
		tree.Insert(*i, pos, halfSizes);
		tree.OperateOnContents([&](std::list<QuadTreeEntry<GameObject*>>& data) {
			CollisionDetection::CollisionInfo info;
			for (auto i = data.begin(); i != data.end(); ++i) {
				for (auto j = std::next(i); j != data.end(); ++j) {
					//is this pair of items allready in the collision set
					// if the same pair is in another quadtree node together ect

					info.a = min((*i).object, (*j).object);
					info.b = max((*i).object, (*j).object);
					broadphaseCollisions.insert(info);

				}
			}
			}
		);
	}
}

/*

The broadphase will now only give us likely collisions, so we can now go through them,
and work out if they are truly colliding, and if so, add them into the main collision list
*/
void PhysicsSystem::NarrowPhase() {
	for (std::set<CollisionDetection::CollisionInfo>::iterator i = broadphaseCollisions.begin(); i != broadphaseCollisions.end(); ++i) {
		CollisionDetection::CollisionInfo info = *i;
		if (CollisionDetection::ObjectIntersection(info.a, info.b, info)) {
			info.framesLeft = numCollisionFrames;
			ImpulseResolveCollision(*info.a, *info.b, info.point);
			allCollisions.insert(info);
		}

	}
}

/*
Integration of acceleration and velocity is split up, so that we can
move objects multiple times during the course of a PhysicsUpdate,
without worrying about repeated forces accumulating etc. 

This function will update both linear and angular acceleration,
based on any forces that have been accumulated in the objects during
the course of the previous game frame.
*/
void PhysicsSystem::IntegrateAccel(float dt) {
	std::vector<GameObject*> ::const_iterator first;
	std::vector<GameObject*> ::const_iterator last;
	gameWorld.GetObjectIterators(first, last);

	for (auto i = first; i != last; ++i) {
		PhysicsObject* object = (*i)->GetPhysicsObject();
		if (object == nullptr) {
			continue;
		}
		float inverseMass = object->GetInverseMass();
		Matrix3 objectInertiaTensor = object->GetInertiaTensor();// already inverse 
		Vector3 linearVel = object->GetLinearVelocity();
		Vector3 force = object->GetForce();
		Vector3 accel = force * inverseMass;
		//std::cout << "object floor contact " << object->GetFloorContact() << std::endl;
		if (applyGravity && inverseMass > 0 && !(object->GetFloorContact()) && (object->getAffectedByGravity())) {
			accel += gravity;
		}
		// testing drag
		float drag;
		drag = std::sqrt(std::sqrt(object->GetLinearVelocity().LengthSquared()));
		//testing drag
		Vector3 earlyAcceleration = ((accel * dt) - (Vector3(1, 1, 1) * (drag * dt)));
		Vector3 lateAcceleration = Vector3(0, 0, 0);
		float disrminantFirst = (accel * dt).Length();
		float discriminantSecond = (Vector3(1, 1, 1) * (drag * dt)).Length();
		//linearVel += accel * dt
		Vector3 accelDirection = accel.Normalised();
		linearVel += (disrminantFirst > discriminantSecond) ? (accelDirection * (earlyAcceleration).Length()) : (accelDirection * (lateAcceleration).Length());
		object->SetLinearVelocity(linearVel);
		//std::cout << linearVel <<" " << std::endl;
		//std::cout << "acelleration " << accel << " " << std::endl;
		// Angular velocity
		Vector3 torque = object->GetTorque();
		Vector3 angVel = object->GetAngularVelocity();

		object->UpdateInertiaTensor(); // UPDATING TENSOR ORIENTATION into world space

		Vector3 angAccel = object->GetInertiaTensor() * torque;

		angVel += angAccel * dt; // integration
		object->SetAngularVelocity(angVel);
	}
}

/*
This function integrates linear and angular velocity into
position and orientation. It may be called multiple times
throughout a physics update, to slowly move the objects through
the world, looking for collisions.
*/
void PhysicsSystem::IntegrateVelocity(float dt) {
	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;
	gameWorld.GetObjectIterators(first, last);
	//float frameLinearDamping = 1.0f - (0.4f * dt);
	// trying new dampenning 
	for (auto i = first; i != last; ++i) {
		PhysicsObject* object = (*i)->GetPhysicsObject();
		if (object == nullptr) {
			continue;
		}
		Transform& transform = (*i)->GetTransform();
		float df = object->GetInverseMass();
		//std::cout << std::pow(transform.GetScale().Length(),1.0/3.0) << std::endl;
		float scaleOb = std::sqrt(1.0 / (1.0 + max(std::pow(transform.GetScale().Length(), 1.0 / 4.0) - 1.5, 0.0)));
		float scaleObCast = ((scaleOb) * (dt * 60));
		float frameLinearDamping = (scaleObCast < 0000.1) ? (scaleObCast) : 1 - (0.4 * dt);
		/*	float scaleOb = (object->GetLinearVelocity()).LengthSquared();

			float frameLinearDamping = (dt>0.1)? (scaleOb * dt) : (scaleOb* (dt+1)) ;*/

			//float scaleOb = std::sqrt(1.0 / ( std::pow(transform.GetScale().Length(), 1.0 / 4.0) - 1.5));
			//float frameLinearDamping = /*1 -*/  ((scaleOb)*dt);
				/*std::cout << frameLinearDamping << std::endl;*/
			//position section
		Vector3 position = transform.GetPosition();
		Vector3 linearVel = object->GetLinearVelocity();
		position += linearVel * dt;
		transform.SetPosition(position);
		// linear Damping
		// trying ::::: object.
		linearVel = linearVel * frameLinearDamping;
		//linearVel = std::min((linearVel * frameLinearDamping), 0.99f );
		object->SetLinearVelocity(linearVel);

		//Angular velocity
		Quaternion orientation = transform.GetOrientation();
		Vector3 angVel = object->GetAngularVelocity();

		orientation = orientation + (Quaternion(angVel * dt * 0.5f, 0.0f) * orientation);
		orientation.Normalise();

		transform.SetOrientation(orientation);
		//Dampenning 
		float frameAngularDamping = 1.0f - (0.4f * dt);
		//add in torque friction
		float torqueFriction = (object->getTorqueFriction());
		//add in torque friction
		angVel = (angVel * frameAngularDamping) * (1 - torqueFriction);
		object->SetAngularVelocity(angVel);
	}
}

/*
Once we're finished with a physics update, we have to
clear out any accumulated forces, ready to receive new
ones in the next 'game' frame.
*/
void PhysicsSystem::ClearForces() {
	gameWorld.OperateOnContents(
		[](GameObject* o) {
			if (o->GetPhysicsObject() != NULL)o->GetPhysicsObject()->ClearForces();
		}
	);
}


/*

As part of the final physics tutorials, we add in the ability
to constrain objects based on some extra calculation, allowing
us to model springs and ropes etc. 

*/
void PhysicsSystem::UpdateConstraints(float dt) {
	std::vector<Constraint*>::const_iterator first;
	std::vector<Constraint*>::const_iterator last;
	gameWorld.GetConstraintIterators(first, last);

	for (auto i = first; i != last; ++i) {
		(*i)->UpdateConstraint(dt);
	}
}
