#include <stdio.h>
#include <math.h>

#include "calc.hpp"
#include "cannon.hpp"
#include "types.hpp"

#define N_CURVE_POINTS 20

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
	const float hW  = cannon.L / 2.f;
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


inline float2 SimulateProjectilePos(float time, const Cannon& cannon)
{
    return {
        time * cannon.v0 * cosf(cannon.angle) + cannon.position.x,
        -(1.f / 2.f * GRAVITY * cannon.projectile.mass * time * time) + cannon.v0 * time * sinf(cannon.angle) + cannon.position.y
    };
}

void CannonRenderer::DrawProjectileMotion(const Cannon& cannon)
{
    //Get-Draw curve of projectile
    this->curvePoints.clear();
    for (float i = 0; i < this->curvePoints.capacity(); i++)
    {
        float2 point = SimulateProjectilePos(i * 0.05, cannon);
        
        if (point.y < -15) //Dont compute everything
            break;
        else 
            this->curvePoints.push_back(this->ToPixels(point));
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
    cannon.position.y = 10.f;
    cannon.angle = TAU / 8.f;
    cannon.L = 10.f;
    cannon.v0 = 15.f,
    cannon.projectile = {
        false,
        30.f,
        cannon.position,
        { 0.f, 0.f },
        { 0.f, 0.f }
    };
}

CannonGame::~CannonGame()
{

}

void CannonGame::UpdateAndDraw(const float& deltaTime)
{
    static float absolute_time = 0.f;
    static float timeScale = 1.f;

    Projectile* p = &cannon.projectile;

    renderer.PreUpdate();

    if (ImGui::Begin("Cannon settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        if (ImGui::Button(p->launched ? "Simulating" : "Launch") && !p->launched)
        {
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
            ImGui::SliderFloat("Displacement", &cannon.position.x, -20.f, -10.f);
            ImGui::SliderFloat("Initial Speed", &cannon.v0, 0.f, 99.f);
            ImGui::SliderFloat("Angle", &cannon.angle, 0.f, TAU / 2.f);
            ImGui::SliderFloat("Mass", &cannon.projectile.mass, 10.f, 100.f);
            ImGui::SliderFloat("Cannon Lenght", &cannon.L, 5.f, 15.f);
            ImGui::SliderFloat("Cannon Height", &cannon.position.y, 0.f, 15.f);
        }

        ImGui::Text("\nProjectile State :\n  Acceleration (x = %.2f, y = %.2f)\n  Velocity (x = %.2f, y = %.2f)\n  Position (x = %.2f, y = %.2f)",
                    p->acceleration.x, p->acceleration.y, p->speed.x, p->speed.y, p->position.x, p->position.y);
    }
    ImGui::End();

    if (p->launched)
    {
        const float c_deltaTime = 0.001666f;
        float simulationTime = 0;

        while (simulationTime < deltaTime)
        {
            absolute_time += c_deltaTime * timeScale;

            p->acceleration = { 0.f, p->mass * -GRAVITY };

            p->speed =
            {
                cannon.v0 * cosf(cannon.angle),
                cannon.v0 * sinf(cannon.angle) - (p->acceleration.y * c_deltaTime)
            };

            if (p->position.y <= 0.f)
                p->launched = false;
            else
                p->position = cannon.position + p->speed * absolute_time + (p->acceleration * absolute_time * absolute_time * 0.5);

            simulationTime += c_deltaTime;
        }
    }

    renderer.DrawGround();
    renderer.DrawCannon(cannon);
    renderer.DrawProjectileMotion(cannon);
}

