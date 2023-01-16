#include <stdio.h>
<<<<<<< HEAD
#include <math.h>
=======
>>>>>>> c5589a5c6ec074e49de06f01a6fa982ffe0570f0
#include <stb_image.h>
#include <GLFW/glfw3.h>

#include "imgui_utils.hpp"

typedef void (*glGenerateMipmapFuncType)(GLenum);

Texture ImGuiUtils::LoadTexture(const char* file)
{
    Texture result = {};
    stbi_uc* pixels = stbi_load(file, &result.width, &result.height, NULL, STBI_rgb_alpha);
    if (pixels == nullptr)
    {
        fprintf(stderr, "Cannot load texture '%s'\n", file);
        return result;
    }

    // Create texture on OpenGL side
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, result.width, result.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    
    // Gen mipmaps
    glGenerateMipmapFuncType glGenerateMipmapFunc = (glGenerateMipmapFuncType)glfwGetProcAddress("glGenerateMipmap");
    if (glGenerateMipmapFunc == nullptr)
        fprintf(stderr, "Cannot load glGenerateMipmap func\n");
    else
        glGenerateMipmapFunc(GL_TEXTURE_2D);

    // Unbind
    glBindTexture(GL_TEXTURE_2D, 0);
    
    // Free ram
    stbi_image_free(pixels);

    result.id = (ImTextureID)((size_t)texture);

    return result;
}

void ImGuiUtils::UnloadTexture(Texture texture)
{
    GLuint tex = (GLuint)((size_t)texture.id);
    glDeleteTextures(1, &tex);
}

<<<<<<< HEAD

void ImGuiUtils::DrawTextureEx(Texture tex, ImVec2 pos, ImVec2 scale, float angle)
{
    ImVec2 p[4] = {
        { -0.5f, -0.5f },
        {  0.5f, -0.5f },
        {  0.5f,  0.5f },
        { -0.5f,  0.5f },
    };

    ImVec2 uv[4] = {
        { 0.f, 0.f },
        { 1.f, 0.f },
        { 1.f, 1.f },
        { 0.f, 1.f },
    };

    // Transform quad (scale + rotate + translate)
    float c = cosf(angle);
    float s = sinf(angle);
    for (int i = 0; i < 4; ++i)
    {
        p[i].x *= tex.width * scale.x;
        p[i].y *= tex.height * scale.y;
        float px = p[i].x;
        p[i].x = p[i].x * c - p[i].y * s;
        p[i].y = p[i].y * c + px * s;
        p[i].x += pos.x;
        p[i].y += pos.y;
    }

    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    dl->AddImageQuad(tex.id, 
        p[0],  p[1],  p[2],  p[3],
        uv[0], uv[1], uv[2], uv[3], 
        IM_COL32_WHITE
    );
}
=======
>>>>>>> c5589a5c6ec074e49de06f01a6fa982ffe0570f0
