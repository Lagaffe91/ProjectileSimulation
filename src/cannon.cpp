#include <stdio.h>
#include <math.h>

#include "calc.hpp"
#include "cannon.hpp"
#include "types.hpp"

CannonRenderer::CannonRenderer()
{
    // Do not use ImDrawList or ImGuiIO here because they are invalid at this point
    // Use this function only to load resources (images, sounds) if needed
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
    // ===================
    // Origin:
    //  - x at the center of the window
    //  - y at the 3/4 bottom of the window
    // Scale:
    //  50 meters from left of the window to the right
    //  Uniform scale in y except it's pointing to up
    worldOrigin.x = io->DisplaySize.x / 2.f;
    worldOrigin.y = io->DisplaySize.y - io->DisplaySize.y / 4.f;
    worldScale.x  = io->DisplaySize.x / 50.f;
    worldScale.y  = -worldScale.x; // Same scale as X bu invert it to invert y coordinates
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
    // For example (Remove this and do your own)
    float2 pos = this->ToPixels(cannon.position);
    dl->AddRect(pos, { pos.x - 10.f, pos.y - 10.f }, IM_COL32_WHITE);
}

void CannonRenderer::DrawProjectileMotion(const CannonState& cannon)
{
    // if (cannon.projectile.launched)
    dl->AddCircle(this->ToPixels(cannon.projectile.position), cannon.projectile.mass, IM_COL32_WHITE);
}

CannonGame::CannonGame(CannonRenderer& renderer)
    : renderer(renderer)
{
    cannonState.position.x = -15.f;
    cannonState.initialSpeed = 15.f,
    cannonState.angle = TAU / 8.f;
    cannonState.projectile = {
        false,
        10.f,
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
    static float t = 0.f;
    Projectile* p = &cannonState.projectile;

    renderer.PreUpdate();

    if (ImGui::Begin("Canon state", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        if (ImGui::Button("Launch") && !p->launched)
        {
            p->launched = true;
            p->position = { cannonState.position.x, cannonState.position.y };
        }
        ImGui::SliderFloat("Height", &cannonState.position.y, 0.f, 15.f);
    }
    ImGui::End();

    if (p->launched)
    {
        t += deltaTime * 0.00000001f;

        // Physics computations
        p->acceleration =
        {
            0.f,
            p->mass * -GRAVITY
        };

        p->speed = 
        {
            cannonState.initialSpeed * cosf(cannonState.angle),
            cannonState.initialSpeed * sinf(cannonState.angle) - p->acceleration.y * t
        };

        p->position =
        {
            p->speed.x * t,
            -(GRAVITY * t * t / 2.f) + p->speed.y * t
        };
    }

    if (p->launched && p->position.y <= renderer.worldOrigin.y)
    {
        p->launched = false;
        t = 0;
    }

    // Draw cannon
    renderer.DrawGround();
    renderer.DrawCannon(cannonState);
    renderer.DrawProjectileMotion(cannonState);
}
