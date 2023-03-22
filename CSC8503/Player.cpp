#include"Player.h"
#include "GameTechRenderer.h"
#include<algorithm>
#include<cmath>


NCL::CSC8503::Player::Player()
{

}

NCL::CSC8503::Player::~Player()
{
}

void NCL::CSC8503::Player::Update(float dt)
{
	Input(dt);
    Rotate();  
    Fire();
}

void NCL::CSC8503::Player::Move(float dt)
{
	
}

void NCL::CSC8503::Player::Fire()
{
    if (Window::GetMouse()->ButtonDown(NCL::MouseButtons::LEFT))
    {
        std::cout << "Fire" << std::endl;
    }
}



void NCL::CSC8503::Player::Rotate()
{
        Matrix4 view = GameWorld::GetInstance().GetMainCamera()->BuildViewMatrix();
        Matrix4 camWorld = view.Inverse();

        Vector3 rightAxis = Vector3(camWorld.GetColumn(0)); //view is inverse of model!

        Vector3 fwdAxis = Vector3::Cross(Vector3(0, 1, 0), rightAxis);

        fwdAxis.y = 0.0f;
        fwdAxis.Normalise();
        /*std::cout << "Self Direction" << this->transform.GetForward() << std::endl;
        std::cout << "Camera Direction" << fwdAxis << std::endl;*/

        
        if (DupDamp != 0 || DrightDamp != 0)
        {
            Vector3 targetForward = fwdAxis * DupDamp - rightAxis * DrightDamp;

            float temp = Vector3::Dot(transform.GetForward(), targetForward) / (transform.GetForward().Length() * targetForward.Length());
            temp = temp > 1 ? 1 : temp;
            temp = temp < -1 ? -1 : temp;

            float degree = (acos((temp))) / 3.1415926 * 180;

            if (Vector3::Cross(transform.GetForward(), targetForward).y > 0)
            {
                degree = 360 - degree;
            }
            
            if (degree != 0 || degree != 360)
            {
                GetTransform().Rotate(Vector3(0, -degree, 0));
            }
        }
    
}

void NCL::CSC8503::Player::Input(float dt)
{

        int a = Window::GetKeyboard()->KeyHeld(KeyboardKeys::A) ? 1 : 0;
        int b = Window::GetKeyboard()->KeyHeld(KeyboardKeys::D) ? 1 : 0;
        int w = Window::GetKeyboard()->KeyHeld(KeyboardKeys::W) ? 1 : 0;
        int s = Window::GetKeyboard()->KeyHeld(KeyboardKeys::S) ? 1 : 0;

        Dup = w - s;
        Dright = a - b;

        DupDamp = SmoothDamp(DupDamp, Dup, upDampVelocity, 0.1f, 1.0f, dt);
        DrightDamp = SmoothDamp(DrightDamp, Dright, rightDampVelocity, 0.1f, 1.0f, dt);

        DupDamp = DupDamp > 1 ? 1 : DupDamp;
        DupDamp = DupDamp < -1 ? -1 : DupDamp;


        if (abs(DupDamp) <= 0.001f)
        {
            DupDamp = 0;
        }

        DrightDamp = DrightDamp > 1 ? 1 : DrightDamp;
        DrightDamp = DrightDamp < -1 ? -1 : DrightDamp;

        if (abs(DrightDamp) <= 0.001f)
        {
            DrightDamp = 0;
        }
}

float NCL::CSC8503::Player::SmoothDamp(float current, float target, float& currentVelocity, float smoothTime,  float maxSpeed,  float deltaTime)
{
    smoothTime = 0.0001f > smoothTime ? 0.0001f : smoothTime;
    //smoothTime = std::max(0.0001f, smoothTime);
    float omega = 2 / smoothTime;

    float x = omega * deltaTime;
    float exp = 1 / (1 + x + 0.48f * x * x + 0.235f * x * x * x);
    float change = current - target;
    float originalTo = target;

    // Clamp maximum speed
    float maxChange = maxSpeed * smoothTime;
    change = std::clamp(change, -maxChange, maxChange);
    target = current - change;

    float temp = (currentVelocity + omega * change) * deltaTime;
    currentVelocity = (currentVelocity - omega * temp) * exp;
    float output = target + (change + temp) * exp;

    // Prevent overshooting
    if (originalTo - current > 0.0f == output > originalTo)
    {
        output = originalTo;
        currentVelocity = (output - originalTo) / deltaTime;
    }

    return output;
}
