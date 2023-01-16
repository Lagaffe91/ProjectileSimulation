#include <stdio.h>
#include <math.h>

#include "calc.hpp"
#include "cannon.hpp"

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
    // TODO: Draw cannon
    
    // For example (Remove this and do your own)
    float2 pos = this->ToPixels(cannon.position);
    dl->AddCircle(pos, 10.f, IM_COL32_WHITE);
}

void CannonRenderer::DrawProjectileMotion(const CannonState& cannon)
{
    // TODO: Draw cannon projectile using ImDrawList
    // e.g. dl->AddLine(...)
    // e.g. dl->PathLineTo(...)
    // etc...
}

CannonGame::CannonGame(CannonRenderer& renderer)
    : renderer(renderer)
{
    cannonState.position.x = -15.f;
    // TODO: Init cannonState here
}

CannonGame::~CannonGame()
{

}

void CannonGame::UpdateAndDraw()
{
    renderer.PreUpdate();

    if (ImGui::Begin("Canon state", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        // TODO: Add UI to edit other cannon state variables here
        ImGui::SliderFloat("Height", &cannonState.position.y, 0.f, 15.f);
    }
    ImGui::End();

    // Draw cannon
    renderer.DrawGround();
    renderer.DrawCannon(cannonState);
    renderer.DrawProjectileMotion(cannonState);
}
