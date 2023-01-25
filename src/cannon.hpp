#pragma once

#include <imgui.h>
#include <vector>

#include "types.hpp"

struct Projectile
{
    bool launched;
    float mass;
    float2 position, speed, acceleration, dSpeed;
    float speedMagnitude;
};

struct Cannon
{
    float2 position;
    float angle, v0, L;
    Projectile projectile;
};

class CannonRenderer
{
public:
    CannonRenderer();
    ~CannonRenderer();

    void PreUpdate();

    ImDrawList* dl;
    ImGuiIO* io;

    // World coordinates conversion (world is expressed in meters)
    float2 worldOrigin; // Origin in pixels
    float2 worldScale;  // Scale relative to pixels
    float2 ToPixels(float2 coordinatesInMeters);
    float2 ToWorld(float2 coordinatesInPixels);

    void DrawGround();
    void DrawCannon(const Cannon& cannon);
    void DrawProjectileMotion(const Cannon& cannon, bool update);

    std::vector<float2> curvePoints;
};

class CannonGame
{
    float prevTime;
public:
    CannonGame(CannonRenderer& renderer);
    ~CannonGame() = default;

    void UpdateAndDraw(const float& deltaTime);

private:
    CannonRenderer& renderer;
    Cannon cannon;
};
