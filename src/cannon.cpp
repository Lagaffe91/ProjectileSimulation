#include <stdio.h>
#include <math.h>

#include "calc.hpp"
#include "cannon.hpp"
#include "types.hpp"

CannonRenderer::CannonRenderer()
{
    constexpr int vectorDefSize = 20;

    curvePoints.reserve(vectorDefSize);
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
    worldScale.y  = -worldScale.x;
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

void CannonRenderer::DrawProjectileMotion(const CannonState& cannon)
{
    dl->AddCircle(
        this->ToPixels(cannon.projectile.position),
        cannon.projectile.mass * 0.25f, 
        IM_COL32_WHITE);
 
    for (size_t i = 1; i < curvePoints.size(); i++)
    {
        dl->AddLine(curvePoints[i-1], curvePoints[i], IM_COL32_WHITE);
    }
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


float2 SimulateProjectilePos(float time, CannonState cannon)
{
    float2 pos
    {
        time * cannon.initialSpeed * cosf(cannon.angle)+ cannon.position.x,
        -(1/2 * GRAVITY * (time * time)) + cannon.initialSpeed * time * sinf(cannon.angle) + cannon.position.y //Float approximation in division be carefull !
    };

    return pos;
}

void CannonGame::UpdateAndDraw(const float& deltaTime)
{
    static float absolute_time = 0;
    Projectile* p = &cannonState.projectile;

    renderer.PreUpdate();

    if (ImGui::Begin("Simulation settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::SliderFloat("Displacement", &cannonState.position.x, -20.f, -10.f);
        ImGui::SliderFloat("Height", &cannonState.position.y, 0.f, 15.f);
        ImGui::SliderFloat("Angle", &cannonState.angle, 0.f, TAU / 2.f);
        ImGui::SliderFloat("Mass", &cannonState.projectile.mass, 0.f, 100.f);

        ImGui::Text("Acceleration (x = %.2f, y = %.2f)\nVelocity (x = %.2f, y = %.2f)\nPosition (x = %.2f, y = %.2f)",
                    p->acceleration.x, p->acceleration.y, p->speed.x, p->speed.y, p->position.x, p->position.y);
        
        if (ImGui::Button(p->launched ? "Simulating" : "Launch") && !p->launched)
        {
            p->launched = true;
            absolute_time = 0;
            p->position = cannonState.position;

            renderer.curvePoints.clear();
        }

    }
    ImGui::End();


    renderer.curvePoints.clear();
    //Get curve of motion
    for (float i = 0; i < renderer.curvePoints.capacity(); i+= renderer.curvePoints.capacity()*0.5)
    {
        renderer.curvePoints.push_back(renderer.ToPixels(SimulateProjectilePos(i, cannonState)));
    }

    if (p->launched)
    {
        const float c_deltaTime = 0.001666f;
        float simulationTime = 0;

        while (simulationTime < deltaTime)
        {
            absolute_time += c_deltaTime;

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

