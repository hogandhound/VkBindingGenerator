#ifndef VK_RENDER_TARGET_H
#define VK_RENDER_TARGET_H

#include "vulkan/vulkan.h"
#include "vk_mem_alloc.h"
#include <vector>

static const uint32_t COMMAND_BUFFER_COUNT = 2;
struct QueueFamilyIndices {
    uint32_t graphicsFamily = 0xFFFFFFFF;
    uint32_t presentFamily = 0xFFFFFFFF;

    bool isComplete() {
        return graphicsFamily != 0xFFFFFFFF && presentFamily != 0xFFFFFFFF;
    }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

enum VkDescFormat
{
    VkDS_T,
    VkDS_TT,
    VkDS_U,
    VkDS_TU,
    VkDS_TTU,
    VkDS_UU,
    VkDS_TUU,
    VkDS_TTUU,
    VkDS_MaxType,
    VkDS_ = VkDS_MaxType, //Dummy Value
};
struct VkDescSetSubType
{
    std::vector<VkDescriptorSet> sets[COMMAND_BUFFER_COUNT];
    int start;
    std::vector<VkDescriptorSetLayoutBinding> defs;
};
struct VkDescSetCol
{
    std::vector<VkDescSetSubType> subs[VkDS_MaxType];
};

class VkRenderTarget
{
public:
    VkInstance instance = 0;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkSurfaceKHR surface;

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;
    VmaAllocator allocator;

    VkQueue graphicsQueue;
    VkQueue presentQueue;

    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;

    VkCommandPool commandPool;
    VkCommandBuffer temporaryCommandBuffer;
    VkCommandBuffer mainCommandBuffers[COMMAND_BUFFER_COUNT];
    VkCommandBuffer currentCmd;
    
    VkRenderPass renderPass;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    std::vector<VkFence> imagesInFlight;
    VkDescSetCol descSets;
    VkDescriptorPool descPools[VkDS_MaxType];
    size_t currentFrame = -1;

    void InitVulkan(void* window);

    void BeginSingleTimeCommands();
    void EndSingleTimeCommands();

    void StartRender();
    void RecreateSwapChain();
    void EndRender();
    uint32_t getDescSetSubIndex(VkDescFormat fmt, VkDescriptorSetLayoutBinding* bindings, int bCount);
    VkDescriptorSet getDescSet(VkDescFormat fmt, uint32_t subIndex, VkDescriptorSetLayout* layout);

private:
    uint32_t imageIndex;

    bool checkValidationLayerSupport();
    void createInstance(void* window);
    void createSurface(void* window);
    void createMemAlloc();

    void createCommandPool();

    void createFramebuffers();

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

    void createLogicalDevice();

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

    VkExtent2D chooseSwapExtent(void* window, const VkSurfaceCapabilitiesKHR& capabilities);

    void createSwapChain(void* window);

    bool checkDeviceExtensionSupport(VkPhysicalDevice device);

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

    bool IsDeviceSuitable(VkPhysicalDevice device);

    void pickPhysicalDevice();

    void createImageViews();

    void createRenderPass();

    std::vector<const char*> getRequiredExtensions(void* window);

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
    void createSyncObjects();
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    void setupDebugMessenger();

    VkImageView createImageView(VkImage image, VkFormat format);
    void createDescPools();
};

#endif