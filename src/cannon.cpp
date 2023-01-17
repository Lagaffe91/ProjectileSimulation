#include <stdio.h>
#include <math.h>

#include "calc.hpp"
#include "cannon.hpp"
#include "types.hpp"

CannonRenderer::CannonRenderer()
{
    constexpr int vectorDefSize = 20;

    curvePoints.reserve(vectorDefSize);

    //To fix few warnings
    this->io = nullptr;
    this->dl = nullptr;

    this->worldOrigin = {};
    this->worldScale = {};
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
    float2 left  = this->ToPixels({ -100.f, 0.f });
    float2 right = this->ToPixels({ +100.f, 0.f });

    dl->AddLine(left, right, IM_COL32_WHITE);
}

void CannonRenderer::DrawCannon(const CannonState& cannon)
{   
    float2 pos = this->ToPixels(cannon.position);
    dl->AddRect(pos, { pos.x - 10.f, pos.y - 10.f }, IM_COL32_WHITE);
}


inline float2 SimulateProjectilePos(float time, CannonState cannon)
{
    return {
        time * cannon.initialSpeed * cosf(cannon.angle) + cannon.position.x,
        -(1.f / 2.f * GRAVITY * cannon.projectile.mass * (time * time)) + cannon.initialSpeed * time * sinf(cannon.angle) + cannon.position.y
    };
}


void CannonRenderer::DrawProjectileMotion(const CannonState& cannon)
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
        this->ToPixels(cannon.projectile.position),
        cannon.projectile.mass * 0.25f,
        IM_COL32_WHITE);
}

CannonGame::CannonGame(CannonRenderer& renderer)
    : renderer(renderer)
{
    cannonState.position.x = -15.f;
    cannonState.position.y = 10.f;
    cannonState.initialSpeed = 15.f,
    cannonState.angle = TAU / 8.f;
    cannonState.projectile = {
        false,
        30.f,
        cannonState.position,
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
    Projectile* p = &cannonState.projectile;

    renderer.PreUpdate();

    if (ImGui::Begin("Cannon settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        if (ImGui::Button(p->launched ? "Simulating" : "Launch") && !p->launched)
        {
            p->launched = true;
            absolute_time = 0;
            p->position = cannonState.position;

            renderer.curvePoints.clear();
        }

        ImGui::NewLine();
        ImGui::SliderFloat("Time Scale", &timeScale, 0.f, 2.f);
        ImGui::NewLine();

        if (!p->launched)
        {
            ImGui::SliderFloat("Displacement", &cannonState.position.x, -20.f, -10.f);
            ImGui::SliderFloat("Height", &cannonState.position.y, 0.f, 15.f);
            ImGui::SliderFloat("Initial Speed", &cannonState.initialSpeed, 0.f, 99.f);
            ImGui::SliderFloat("Angle", &cannonState.angle, 0.f, TAU / 2.f);
            ImGui::SliderFloat("Mass", &cannonState.projectile.mass, 0.f, 100.f);
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
                cannonState.initialSpeed * cosf(cannonState.angle),
                cannonState.initialSpeed * sinf(cannonState.angle) - (p->acceleration.y * c_deltaTime)
            };

            if (p->position.y <= 0.f)
                p->launched = false;
            else
                p->position = cannonState.position + p->speed * absolute_time + (p->acceleration * absolute_time * absolute_time * 0.5);

            simulationTime += c_deltaTime;
        }
    }

    renderer.DrawGround();
    renderer.DrawCannon(cannonState);
    renderer.DrawProjectileMotion(cannonState);
}

