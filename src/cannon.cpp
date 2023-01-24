#include <stdio.h>
#include <math.h>

#include "calc.hpp"
#include "cannon.hpp"
#include "types.hpp"

#define N_CURVE_POINTS 1000

CannonRenderer::CannonRenderer()
{
    curvePoints.reserve(N_CURVE_POINTS);
}

CannonRenderer::~CannonRenderer()
{
    // Unload resources if needed
}

void CannonRenderer::PreUpdate()
{
    // Get frequently used variables
    dl = ImGui::GetBackgroundDrawList();
    io = &ImGui::GetIO();

    // Compute world space
    worldOrigin.x = io->DisplaySize.x / 2.f;
    worldOrigin.y = io->DisplaySize.y - io->DisplaySize.y / 4.f;
    worldScale.x  = io->DisplaySize.x / 50.f;
    worldScale.y= -worldScale.x;
}

float2 CannonRenderer::ToPixels(float2 coordinatesInMeters)
{
    return (coordinatesInMeters * worldScale) + worldOrigin;
}

float2 CannonRenderer::ToWorld(float2 coordinatesInPixels)
{
    return (coordinatesInPixels - worldOrigin) / worldScale;
}

void CannonRenderer::DrawGround()
{
    float2 left  = this->ToPixels({ -100.f, -0.5f });
    float2 right = this->ToPixels({ +100.f, -0.5f });

    dl->AddLine(left, right, IM_COL32_WHITE);
}

inline void RotateAround(const float2& origin, float2& point, const float angle)
{
    point = point - origin;
    float2 n_point = {
        point.x * cosf(angle) - point.y * sinf(angle),
        point.x * sinf(angle) + point.y * cosf(angle)
    };
    point = n_point + origin;
}

void CannonRenderer::DrawCannon(const Cannon& cannon)
{   
	float2 o = cannon.position;
	const float hW  = cannon.L;
	const float hH = 0.5f;
	float2 p1 =  this->ToPixels({o.x + hW, o.y + hH});
	float2 p2 =  this->ToPixels({o.x     , o.y + hH});
	float2 p3 =  this->ToPixels({o.x     , o.y - hH});
	float2 p4 =  this->ToPixels({o.x + hW, o.y - hH});
    
    float2 p[4] = { p1, p2, p3, p4 };
    for (int i = 0; i < 4; i++)
		RotateAround(this->ToPixels(o), p[i], -cannon.angle);
    
    dl->AddQuad(p[0], p[1], p[2], p[3], IM_COL32_WHITE);
}

inline float2 SimulateProjectilePos(float time, const Cannon& cannon, float2 p0)
{
    float2 position = {};
    float2 speed =
    {
        cannon.v0 * cosf(cannon.angle),
        cannon.v0 * sinf(cannon.angle)
    };
    position = p0 + speed * time + (cannon.projectile.acceleration * time * time * 0.5f);
    return (position);
}
 
void CannonRenderer::DrawProjectileMotion(const Cannon& cannon, bool update)
{
    Projectile *p = (Projectile *)&cannon.projectile;
    this->curvePoints.clear();
    float2 point = cannon.position;
    static float2 prevPos = point;
    float time = 0;
    float dTime = 0.016f;
    float prevTime = 0;
    if (point.y > 0)
    {
        for (size_t i = 0; i < this->curvePoints.capacity(); i++)
        {
            time += dTime;
            float cannonLenght = cosf(cannon.angle) * cannon.L;
            if (point.x - cannon.position.x < -0.01f)
                ;
            else if (point.x - cannon.position.x < cannonLenght)
            {
                p->acceleration = { -GRAVITY * cosf(cannon.angle), -GRAVITY * sinf(cannon.angle)};
                point    = SimulateProjectilePos(time, cannon, cannon.position);
                prevPos  = point;
                prevTime = time;
            }
            else
            {
                p->acceleration = { 0, -GRAVITY};
                point = SimulateProjectilePos(time - prevTime, cannon, prevPos);
            }
            if (point.y < -0.5 || point.x - cannon.position.x < 0)
            {
                break;
            }
            else
                this->curvePoints.push_back(this->ToPixels(point));
        }
    }
    for (size_t i = 1; i < curvePoints.size(); i++)
    {
        dl->AddLine(curvePoints[i-1], curvePoints[i], IM_COL32_WHITE);
    }

    //Draw projectile itself
    dl->AddCircle(
        this->ToPixels(cannon.projectile.position), 10.f, IM_COL32_WHITE);
}

CannonGame::CannonGame(CannonRenderer& renderer)
    : renderer(renderer)
{
    cannon.position.x = -15.f;
    cannon.position.y = 1.f;
    cannon.angle = TAU / 8.f;
    cannon.L = 5.f;
    cannon.v0 = 50.f,
    cannon.projectile = { false, 30.f, cannon.position, { 0.f, 0.f }, { 0.f, 0.f } };
}

void CannonGame::UpdateAndDraw(const float& deltaTime)
{
    static float absolute_time;
    static float timeScale = 1.f;
    bool update = false;
    Projectile* p = &cannon.projectile;

    renderer.PreUpdate();

    if (ImGui::Begin("Cannon settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        if (ImGui::Button(p->launched ? "Simulating" : "Launch") && !p->launched)
        {
            update = true;
            p->launched = true;
            absolute_time = 0;
            p->position = cannon.position;

            renderer.curvePoints.clear();
        }

        ImGui::NewLine();
        ImGui::SliderFloat("Time Scale", &timeScale, 0.f, 2.f);
        ImGui::NewLine();

        if (!p->launched)
        {
            update |= ImGui::SliderFloat("Displacement", &cannon.position.x, -20.f, -10.f);
            update |= ImGui::SliderFloat("Initial Speed", &cannon.v0, 0.f, 99.f);
            update |= ImGui::SliderFloat("Angle", &cannon.angle, 0.f, TAU / 4.f);
            update |= ImGui::SliderFloat("Mass", &cannon.projectile.mass, 10.f, 100.f);
            update |= ImGui::SliderFloat("Cannon Lenght", &cannon.L, 5.f, 10.f);
            update |= ImGui::SliderFloat("Cannon Height", &cannon.position.y, 1.f, 10.f);
        }

        ImGui::Text("Acceleration: x = %.2f y = %.2f\nVelocity:x = %.2f y = %.2f (%.2f m/s)\nPosition: x = %.2f y = %.2f",
                    p->acceleration.x, p->acceleration.y, p->dSpeed.x, p->speedMagnitude, p->dSpeed.y, p->position.x, p->position.y);
    }
    ImGui::End();

    // v^2 - v0^2 = 2gL
    // v^2 = 2gL - v0^2
    // v = sqrt 2gL - v0^2
    if (p->launched)
    {
        float c_deltaTime = 0.00001f;
        float simulationTime = 0;

        float2 prevPosition = p->position;
        static float2 lastPosition = cannon.position;
        static float prevTime;

        while (simulationTime < deltaTime * timeScale)
        {
            float cannonLenght = cosf(cannon.angle) * cannon.L;
            if (p->position.x - cannon.position.x < -0.01f)
                ;
            else if (p->position.x - cannon.position.x < cannonLenght)
            {
                p->acceleration = { -GRAVITY * cosf(cannon.angle), -GRAVITY * sinf(cannon.angle)};
                p->position  = SimulateProjectilePos(absolute_time, cannon, cannon.position);
                lastPosition = p->position;
                prevTime     = absolute_time;
            }
            else
            {
                p->acceleration = { 0, -GRAVITY};
                p->position = SimulateProjectilePos(absolute_time - prevTime, cannon, lastPosition);
            }
            if (p->position.y < -0.5 || p->position.x - cannon.position.x < 0)
            {
                p->launched = false;
                prevTime = 0;
                break;
            }
            simulationTime += c_deltaTime;
        }
        absolute_time += deltaTime * timeScale;
        p->dSpeed = p->position - prevPosition;
        p->speedMagnitude = length(p->dSpeed);
    }

    renderer.DrawGround();
    renderer.DrawCannon(cannon);
    renderer.DrawProjectileMotion(cannon, update);
}