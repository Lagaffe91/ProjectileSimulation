
#include "app.hpp"

#include "imgui_utils.hpp"

App::App()
: cannonGame(cannonRenderer)
{
}

App::~App()
{
}

void App::Update()
{
    cannonGame.UpdateAndDraw();
}
