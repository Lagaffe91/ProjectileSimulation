#pragma once

#include <imgui.h>

#include "types.hpp"

struct CannonState
{
    float2 position;
    float angle; // In radians
    float initialSpeed;
};

// Renderer class
// Used to load resources
class CannonRenderer
{
public:
    CannonRenderer();
    ~CannonRenderer();

    void PreUpdate();

    // Fast access to drawlist and io
    ImDrawList* dl;
    ImGuiIO* io;

    // World coordinates conversion (world is expressed in meters)
    float2 worldOrigin; // Origin in pixels
    float2 worldScale;  // Scale relative to pixels
    float2 ToPixels(float2 coordinatesInMeters);
    float2 ToWorld(float2 coordinatesInPixels);

    // Draw functions
    void DrawGround();
    void DrawCannon(const CannonState& cannon);
    void DrawProjectileMotion(const CannonState& cannon);
};

class CannonGame
{
public:
    CannonGame(CannonRenderer& renderer);
    ~CannonGame();

    void UpdateAndDraw();

private:
    CannonRenderer& renderer;
    CannonState cannonState;
};
