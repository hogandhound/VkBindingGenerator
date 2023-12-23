#include "VkBufferTools.h"

void VkBufferTools::CreateBuffer(VkRenderTarget* target, VkDeviceSize size, void* memory, VkBufferUsageFlags usage, VmaMemoryUsage properties, VkBuffer& buffer, VmaAllocation& allocation)
{
    VkBufferCreateInfo vbInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    vbInfo.size = size;
    vbInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    vbInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo vbAllocCreateInfo = {};
    vbAllocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
    vbAllocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    VmaAllocation stagingBufferAlloc = VK_NULL_HANDLE;
    VmaAllocationInfo stagingBufferAllocInfo = {};
    vmaCreateBuffer(target->allocator, &vbInfo, &vbAllocCreateInfo, &stagingBuffer, &stagingBufferAlloc, &stagingBufferAllocInfo);

    memcpy(stagingBufferAllocInfo.pMappedData, memory, size);

    // No need to flush stagingVertexBuffer memory because CPU_ONLY memory is always HOST_COHERENT.

    vbInfo.usage = usage;
    vbAllocCreateInfo.usage = properties;
    vbAllocCreateInfo.flags = 0;
    vmaCreateBuffer(target->allocator, &vbInfo, &vbAllocCreateInfo, &buffer, &allocation, nullptr);
    
    // Copy buffers

    target->BeginSingleTimeCommands();

    VkBufferCopy vbCopyRegion = {};
    vbCopyRegion.srcOffset = 0;
    vbCopyRegion.dstOffset = 0;
    vbCopyRegion.size = vbInfo.size;
    vkCmdCopyBuffer(target->temporaryCommandBuffer, stagingBuffer, buffer, 1, &vbCopyRegion);

    target->EndSingleTimeCommands();

    vmaDestroyBuffer(target->allocator, stagingBuffer, stagingBufferAlloc);
}
