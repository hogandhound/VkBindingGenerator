#ifndef VK_BUFFER_TOOLS_H
#define VK_BUFFER_TOOLS_H

#include "VkRenderTarget.h"

class VkBufferTools
{
public:
    static void CreateBuffer(VkRenderTarget* target, VkDeviceSize size, void* memory, VkBufferUsageFlags usage, VmaMemoryUsage properties, VkBuffer& buffer, VmaAllocation& allocation);
};

#endif