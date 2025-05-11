#ifndef VK_TEXTURE_H
#define VK_TEXTURE_H

#include "VkRenderTarget.h"

class VkTexture
{
public:
    VkImage image;
    VkImageView imageView;
    VmaAllocation memory;
    VkSampler sampler;

    VkTexture();

    void CreateImage(VkRenderTarget* target, uint32_t width, uint32_t height, uint32_t* rgba);
    void CreateImage(VkRenderTarget* target, uint32_t width, uint32_t height, uint8_t* rgba, VkFormat format);

private:
    void createTextureSampler(VkRenderTarget* target);
};

#endif
