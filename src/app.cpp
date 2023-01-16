
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
<<<<<<< HEAD
}
=======
}
>>>>>>> c5589a5c6ec074e49de06f01a6fa982ffe0570f0
