#pragma once

#include <imgui.h>

struct Texture
{
    ImTextureID id;
    int width;
    int height;
};

class ImGuiUtils
{
public:
    static Texture LoadTexture(const char* file);
    static void UnloadTexture(Texture texture);
<<<<<<< HEAD

    void DrawTextureEx(Texture tex, ImVec2 pos, ImVec2 scale, float angle);
=======
>>>>>>> c5589a5c6ec074e49de06f01a6fa982ffe0570f0
};
