#pragma once

#include "cannon.hpp"

class App
{
public:
    App();
    ~App();

    void Update();

private:
    CannonRenderer cannonRenderer;
    CannonGame cannonGame;
};
