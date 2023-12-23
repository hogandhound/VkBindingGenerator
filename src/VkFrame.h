#ifndef SDL_VK_FRAME_H
#define SDL_VK_FRAME_H

#include <vulkan/vulkan.h>

#include <vector>

#include "VkRenderTarget.h"

class VkFrame
{
public:
    VkFrame();

    void Initialize(const char* title, float x, float y);

    void InitVulkan();

    VkRenderTarget* GetTarget();

private:
    VkRenderTarget target;

    void Thread(const char* title, float x, float y);
    void* window, * handle, * context;
    bool alive;

};

#endif
