#include "VkTexture.h"

#include "vulkan/vulkan.h"
#include "vk_mem_alloc.h"

VkTexture::VkTexture()
{
}

//void PipelineImageDraw::createDescriptorSets(VkRenderTarget* target) {
//    VkDescriptorSetAllocateInfo allocInfo = {};
//    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
//    allocInfo.descriptorPool = descriptorPool;
//    allocInfo.descriptorSetCount = 1;
//    allocInfo.pSetLayouts = &descriptorSetLayout;
//
//    if (vkAllocateDescriptorSets(target->device, &allocInfo, &descriptorSet) != VK_SUCCESS) {
//        throw std::runtime_error("failed to allocate descriptor sets!");
//    }
//
//    //VkDescriptorBufferInfo bufferInfo = {};
//    //bufferInfo.buffer = uniformBuffers[i];
//    //bufferInfo.offset = 0;
//    //bufferInfo.range = sizeof(UniformBufferObject);
//
//    VkDescriptorImageInfo imageInfo = {};
//    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//    imageInfo.imageView = textureImageView;
//    imageInfo.sampler = textureSampler;
//
//    std::array<VkWriteDescriptorSet, 2> descriptorWrites = {};
//
//    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//    descriptorWrites[0].dstSet = descriptorSets[i];
//    descriptorWrites[0].dstBinding = 0;
//    descriptorWrites[0].dstArrayElement = 0;
//    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//    descriptorWrites[0].descriptorCount = 1;
//    descriptorWrites[0].pBufferInfo = &bufferInfo;
//
//    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//    descriptorWrites[1].dstSet = descriptorSets[i];
//    descriptorWrites[1].dstBinding = 1;
//    descriptorWrites[1].dstArrayElement = 0;
//    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
//    descriptorWrites[1].descriptorCount = 1;
//    descriptorWrites[1].pImageInfo = &imageInfo;
//
//    vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
//}

void VkTexture::CreateImage(VkRenderTarget* target, uint32_t width, uint32_t height, uint32_t* rgba)
{
    CreateImage(target, width, height, (uint8_t*)rgba, VK_FORMAT_R8G8B8A8_UNORM);
}

void VkTexture::CreateImage(VkRenderTarget* target, uint32_t width, uint32_t height, uint8_t* rgba, VkFormat format)
{
    VkDeviceSize imageSize; 
    switch (format)
    {
    case VK_FORMAT_R8G8B8A8_UNORM: imageSize = width * height * 4; break;
    case VK_FORMAT_R8G8B8_UNORM:imageSize = width * height * 4; break;
    }

    VkBufferCreateInfo stagingBufInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    stagingBufInfo.size = imageSize;
    stagingBufInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    VmaAllocationCreateInfo stagingBufAllocCreateInfo = {};
    stagingBufAllocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
    stagingBufAllocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

    VkBuffer stagingBuf = VK_NULL_HANDLE;
    VmaAllocation stagingBufAlloc = VK_NULL_HANDLE;
    VmaAllocationInfo stagingBufAllocInfo = {};
    vmaCreateBuffer(target->allocator, &stagingBufInfo, &stagingBufAllocCreateInfo, &stagingBuf, &stagingBufAlloc, &stagingBufAllocInfo);

    char* const pImageData = (char*)stagingBufAllocInfo.pMappedData;
    switch (format)
    {
    case VK_FORMAT_R8G8B8A8_UNORM: memcpy(pImageData, rgba, imageSize); break;
    case VK_FORMAT_R8G8B8_UNORM:
    {
        for (int i = 0; i < width * height; ++i)
        {
            pImageData[i * 4 + 0] = rgba[i * 3 + 0];
            pImageData[i * 4 + 1] = rgba[i * 3 + 1];
            pImageData[i * 4 + 2] = rgba[i * 3 + 2];
            pImageData[i * 4 + 3] = 255;
        }
        format = VK_FORMAT_R8G8B8A8_UNORM;
        break;
    }
    }

    VkImageCreateInfo imageInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags = 0;

    VmaAllocationCreateInfo imageAllocCreateInfo = {};
    imageAllocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    vmaCreateImage(target->allocator, &imageInfo, &imageAllocCreateInfo, &image, &memory, nullptr);

    // Transition image layouts, copy image.

    target->BeginSingleTimeCommands();

    VkImageMemoryBarrier imgMemBarrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
    imgMemBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imgMemBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imgMemBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imgMemBarrier.subresourceRange.baseMipLevel = 0;
    imgMemBarrier.subresourceRange.levelCount = 1;
    imgMemBarrier.subresourceRange.baseArrayLayer = 0;
    imgMemBarrier.subresourceRange.layerCount = 1;
    imgMemBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imgMemBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    imgMemBarrier.image = image;
    imgMemBarrier.srcAccessMask = 0;
    imgMemBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    vkCmdPipelineBarrier(
        target->temporaryCommandBuffer,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &imgMemBarrier);

    VkBufferImageCopy region = {};
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.layerCount = 1;
    region.imageExtent.width = width;
    region.imageExtent.height = height;
    region.imageExtent.depth = 1;

    vkCmdCopyBufferToImage(target->temporaryCommandBuffer, stagingBuf, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    imgMemBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    imgMemBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imgMemBarrier.image = image;
    imgMemBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    imgMemBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(
        target->temporaryCommandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &imgMemBarrier);

    target->EndSingleTimeCommands();

    vmaDestroyBuffer(target->allocator, stagingBuf, stagingBufAlloc);

    // Create ImageView

    VkImageViewCreateInfo textureImageViewInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    textureImageViewInfo.image = image;
    textureImageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    textureImageViewInfo.format = format;
    textureImageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    textureImageViewInfo.subresourceRange.baseMipLevel = 0;
    textureImageViewInfo.subresourceRange.levelCount = 1;
    textureImageViewInfo.subresourceRange.baseArrayLayer = 0;
    textureImageViewInfo.subresourceRange.layerCount = 1;
    vkCreateImageView(target->device, &textureImageViewInfo, nullptr, &imageView);

    createTextureSampler(target);
}

void VkTexture::createTextureSampler(VkRenderTarget* target) {
    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_NEAREST;
    samplerInfo.minFilter = VK_FILTER_NEAREST;

    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 16;

    samplerInfo.unnormalizedCoordinates = VK_FALSE;

    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    if (vkCreateSampler(target->device, &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
    }
}