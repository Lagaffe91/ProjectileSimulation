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
    position = p0 + cannon.projectile.speed * time + (cannon.projectile.acceleration * time * time * 0.5f);
    return (position);
}
 
void CannonRenderer::DrawProjectileMotion(const Cannon& cannon, bool update)
{
    Projectile *p = (Projectile *)&cannon.projectile;
    float time = 0;
    float dTime = 0.016f;
    float prevTime = 0;
    if (update)
    {
        float2 point = cannon.position;
        this->curvePoints.clear();
        float2 cannonExit =
        {
            cosf(cannon.angle) * cannon.L + cannon.position.x,
            sinf(cannon.angle) * cannon.L + cannon.position.y
        };
        float2 cannonDeacceleration = { -GRAVITY * cosf(cannon.angle), -GRAVITY * sinf(cannon.angle)};
        float exitSpeed = sqrtf(2 * -length(p->acceleration) * cannon.L + cannon.v0 * cannon.v0);
        for (size_t i = 0; i < this->curvePoints.capacity(); i++)
        {
            time += dTime;
            float cannonLenght = cosf(cannon.angle) * cannon.L;
            if (point.x - cannon.position.x < -0.01f)
                ;
            else if (point.x - cannon.position.x <= cannonLenght)
            {
                p->speed =
                {
                    cannon.v0 * cosf(cannon.angle),
                    cannon.v0 * sinf(cannon.angle)
                };
                p->acceleration = cannonDeacceleration;
                point    = SimulateProjectilePos(time, cannon, cannon.position);
                prevTime = time;
            }
            else if (exitSpeed > 0)
            {
                p->speed =
                {
                    exitSpeed * cosf(cannon.angle),
                    exitSpeed * sinf(cannon.angle)
                };
                p->acceleration = { 0, -GRAVITY};
                point = SimulateProjectilePos(time - prevTime, cannon, cannonExit);
            }
            if (point.y < -0.5 || point.x - cannon.position.x < 0)
                break;
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

bool CannonRenderer::DrawImgui(Cannon& cannon)
{
    bool updated = false;

    if (ImGui::Begin("Cannon settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        if (ImGui::Button(cannon.projectile.launched ? "Simulating" : "Launch") && !cannon.projectile.launched)
        {
            updated = true;
            cannon.projectile.launched = true;
            cannon.projectile.position = cannon.position;

            this->curvePoints.clear();
        }

        ImGui::NewLine();
        ImGui::SliderFloat("Time Scale", &timeScale, 0.f, 2.f);
        ImGui::NewLine();

        if (!cannon.projectile.launched)
        {
            updated |= ImGui::SliderFloat("Displacement", &cannon.position.x, -20.f, -10.f);
            updated |= ImGui::SliderFloat("Initial Speed", &cannon.v0, 0.f, 99.f);
            updated |= ImGui::SliderFloat("Angle", &cannon.angle, 0.f, TAU / 4.f);
            updated |= ImGui::SliderFloat("Mass", &cannon.projectile.mass, 10.f, 100.f);
            updated |= ImGui::SliderFloat("Cannon Lenght", &cannon.L, 5.f, 10.f);
            updated |= ImGui::SliderFloat("Cannon Height", &cannon.position.y, 1.f, 10.f);
        }

        ImGui::Text("Acceleration: x = %.2f y = %.2f\nVelocity:x = %.2f y = %.2f (%.2f m/s)\nPosition: x = %.2f y = %.2f",
            cannon.projectile.acceleration.x, cannon.projectile.acceleration.y, cannon.projectile.dSpeed.x, cannon.projectile.speedMagnitude, cannon.projectile.dSpeed.y, cannon.projectile.position.x, cannon.projectile.position.y);
    }

    ImGui::End();

    return updated;
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
    prevTime = 0;
}

void CannonGame::UpdateAndDraw(const float& deltaTime)
{
    static float absolute_time;
    
    Projectile* p = &cannon.projectile;

    renderer.PreUpdate();

    bool parametersUpdated = renderer.DrawImgui(cannon);

    if (p->launched)
    {
        float c_deltaTime = 0.00001f;
        float simulationTime = 0;

        float2 cannonExit =
        {
            cosf(cannon.angle) * cannon.L + cannon.position.x,
            sinf(cannon.angle) * cannon.L + cannon.position.y
        };
        float2 prevPosition = p->position;
        float exitSpeed = sqrtf(2 * -length(p->acceleration) * cannon.L + cannon.v0 * cannon.v0);
        while (simulationTime < deltaTime * renderer.timeScale)
        {
            float cannonLenght = cosf(cannon.angle) * cannon.L;
            if (p->position.y < -0.5 || p->position.x - cannon.position.x < 0 || p->position.x - cannon.position.x < -0.01f)
            {
                p->launched = false;
                prevTime = 0;
                break;
            }
            else if (p->position.x - cannon.position.x < cannonLenght)
            {
                p->acceleration = { -GRAVITY * cosf(cannon.angle), -GRAVITY * sinf(cannon.angle)};
                p->speed =
                {
                    cannon.v0 * cosf(cannon.angle),
                    cannon.v0 * sinf(cannon.angle)
                };
                p->position  = SimulateProjectilePos(absolute_time, cannon, cannon.position);
                prevTime     = absolute_time;
            }
            else if (exitSpeed > 0)
            {
                p->speed =
                {
                    exitSpeed * cosf(cannon.angle),
                    exitSpeed * sinf(cannon.angle)
                };
                p->acceleration = { 0, -GRAVITY};
                p->position = SimulateProjectilePos(absolute_time - prevTime, cannon, cannonExit);
            }
            simulationTime += c_deltaTime;
        }
        absolute_time += deltaTime * renderer.timeScale;
        p->dSpeed = p->position - prevPosition;
        p->speedMagnitude = length(p->dSpeed);
    }
    else
    {
        absolute_time = 0;
    }

    renderer.DrawGround();
    renderer.DrawCannon(cannon);
    renderer.DrawProjectileMotion(cannon, parametersUpdated);
}