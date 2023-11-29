#include "Application.h"

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <format>
#include <map>
#include <set>
#include <cstdalign>
#include <limits>
#include <algorithm>

#include <utils.h>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;
const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};
const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};
#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

/**
 * 一个处理来自Vulkan API的调试消息的回调函数。
 *
 * @param messageSeverity 调试消息的严重程度级别。
 * @param messageType 调试消息的类型。
 * @param pCallbackData 指向调试消息数据的指针。
 * @param pUserData 指向用户数据的指针。
 *
 * @return VK_FALSE 表示调试回调不会中止Vulkan函数调用。
 *
 * @throws 无。
 */
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {

    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

EngineCore::MioEngine::MioEngine(){
    m_physicalDevices = std::vector<VkPhysicalDevice>();
    m_physicalDevices.reserve(1);
}

EngineCore::MioEngine::~MioEngine(){

}

/**
 * 运行 EngineCore 的 MioEngine。
 *
 * @throws ErrorType 如果在初始化、主循环或清理过程中出现错误。
 */
void EngineCore::MioEngine::run(){
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
}

/**
 * 初始化窗口。
 *
 * 该函数用于初始化窗口，包括初始化GLFW库、设置窗口属性等操作。
 *
 * @throws ErrorType 如果在初始化窗口过程中出现错误。
 *
 * @return Vulkan结果代码
 */
VkResult EngineCore::MioEngine::initWindow(){
    glfwInit(); //初始化GLFW库
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); //声明并非创建OpenGL的context
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); //暂时禁用修改窗口大小

    m_window = glfwCreateWindow(WIDTH, HEIGHT, "MioEnigine", nullptr, nullptr);
    if (m_window == nullptr) {
        glfwTerminate();
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    return VK_SUCCESS;
}

/**
 * 初始化Vulkan相关设置。
 *
 * 该函数用于初始化Vulkan相关设置，包括创建Vulkan实例、选择物理设备、创建逻辑设备等。
 *
 * @throws ErrorType 如果在初始化Vulkan过程中出现错误。
 *
 * @return void
 */
VkResult EngineCore::MioEngine::initVulkan(){
    auto result = VK_SUCCESS;

    //初始化Vulkan实例
    createInstance();

    //设置调试信使
    setupDebugMessenger();

    //创建显示窗口
    createSurface();

    //选取物理设备
    pickPhysicalDevice();

    //创建逻辑设备
    createLogicalDevice();

    //创建交换链
    createSwapChain();

    //创建图片视图
    createImageViews();

    //创建渲染通道
    createRenderPass();

    //创建渲染管线
    createGraphicsPipeline();

    return result;
}

/**
 * 初始化MioEngine的Vulkan实例。
 *
 * @return VkResult 初始化Vulkan实例的结果。
 *
 * @throws std::runtime_error 如果硬件不支持所需的GLFW扩展。
 * @throws std::runtime_error 如果硬件不支持所需的扩展。
 */
void EngineCore::MioEngine::createInstance() {
    VkResult result = VK_SUCCESS;
    //检查是否支持验证层
    if (enableValidationLayers && !checkValidationLayerSupport()) {
        throw std::runtime_error("Validation layers requested, but not available!");
    }

    //创建Vulkan应用程序信息
    VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Mio",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "MioEngine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_0
    };

    //创建Vulkan实例信息
    VkInstanceCreateInfo instanceInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo
    };

    //获取所需的扩展
    auto extensions = getRequiredExtensions();
    instanceInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    instanceInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    //在实例信息里设置验证层
    if (enableValidationLayers) {
        instanceInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        instanceInfo.ppEnabledLayerNames = validationLayers.data();

        populateDebugMessengerCreateInfo(debugCreateInfo);
        instanceInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    } else {
        instanceInfo.enabledLayerCount = 0;
        instanceInfo.pNext = nullptr;
    }
    
    //创建Vulkan实例
    result = vkCreateInstance(&instanceInfo, nullptr, &m_instances);
    
    //检查Vulkan实例是否创建成功
    if (result == VK_ERROR_EXTENSION_NOT_PRESENT) {
        throw std::runtime_error("Hardware does not support desired extensions.");
    }
}

/**
 * 创建一个用于 Vulkan 实例的调试工具信使。
 *
 * @param instance Vulkan 实例。
 * @param pCreateInfo 指向包含调试工具信使创建信息的结构体。
 * @param pAllocator 指向分配器回调结构体的指针。
 * @param pDebugMessenger 指向调试工具信使句柄的指针。
 *
 * @return VkResult 调试工具信使创建的结果。
 *
 * @throws None
 */
VkResult EngineCore::MioEngine::createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger){
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

/**
 * 获取EngineCore所需的扩展。
 *
 * @return 包含所需扩展的const char*向量。
 *
 * @throws ErrorType 错误描述
 */
std::vector<const char*> EngineCore::MioEngine::getRequiredExtensions(){
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    getSupportedExtensions();

    std::cout << "glfw extension:\n"; 
    for (int i = 0; i < glfwExtensionCount; i++) {
        std::cout << std::format("\t{}\n", glfwExtensions[i]);
        
        bool isSupport = false;
        for (const auto& extension : m_extensions) {
            if (std::string(extension.extensionName) == std::string(glfwExtensions[i])) {
                isSupport = true;
                break;
            }
        }
        
        if (!isSupport) {
            throw std::runtime_error(std::format("Hardware does not support desired GLFW extensions:{}.", glfwExtensions[i]));
        }
    }

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

/**
 * 检查验证层是否支持。
 *
 * @return 如果验证层支持则返回 true，否则返回 false。
 */
bool EngineCore::MioEngine::checkValidationLayerSupport(){
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : validationLayers){
        bool layerFound = false;

        for(const auto& layerProperties : availableLayers){
            if (strcmp(layerName, layerProperties.layerName) == 0){
                layerFound = true;
                break;
            }
        }

        if (!layerFound){
            return false;
        }
    }

    return true;
}

/**
 * 检查给定的 Vulkan 物理设备是否支持所有必需的扩展。
 *
 * @param device 要检查的 Vulkan 物理设备。
 *
 * @return 如果设备支持所有必需的扩展，则返回 true，否则返回 false。
 *
 * @throws 无。
 */
bool EngineCore::MioEngine::checkDeviceExtensionSupport(VkPhysicalDevice device){
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

/**
 * 确定给定的物理设备是否适用于使用。
 *
 * @param device 要检查的物理设备
 *
 * @return 如果设备适用，则为true；否则为false
 *
 * @throws None
 */
bool EngineCore::MioEngine::isDeviceSuitable(VkPhysicalDevice device){
    EngineCore::QueueFamilyIndices indices = findQueueFamily(device);

    bool extensionsSupported = checkDeviceExtensionSupport(device);

    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    //独立显卡且支持几何着色器
    return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

/**
 * 查找给定的 Vulkan 物理设备的队列族索引。
 *
 * @param device 要查找队列族索引的 Vulkan 物理设备。
 *
 * @return 指定的 Vulkan 物理设备的队列族索引。
 *
 * @throws 无
 */
EngineCore::QueueFamilyIndices EngineCore::MioEngine::findQueueFamily(VkPhysicalDevice device){
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &presentSupport);

        if (presentSupport) {
            indices.presentFamily = i;
        }

        if (indices.isComplete()) {
            break;
        }
        i++;
    }

    return indices;
}

/**
 * 查询给定 Vulkan 物理设备的交换链支持详情。
 *
 * @param device 要查询交换链支持的 Vulkan 物理设备
 *
 * @return 给定物理设备的交换链支持详情
 *
 * @throws None
 */
EngineCore::SwapChainSupportDetails EngineCore::MioEngine::querySwapChainSupport(VkPhysicalDevice device){
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &details.capabilities);

    //获取所支持的所有格式
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, details.formats.data());
    }

    //获取所支持的表示模式
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, details.presentModes.data());
    }

    return details;    
}

VkSurfaceFormatKHR EngineCore::MioEngine::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats){
    //format控制颜色通道深度以及透明度等，colorSpace控制颜色空间
    for (const auto& availableFormate : availableFormats) {
        if (availableFormate.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormate.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR) {
            return availableFormate;
        }
    }
    return availableFormats.front();
}

VkPresentModeKHR EngineCore::MioEngine::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes){
    /*
    * 控制显示模式，这里采用三缓冲机制，
    * 还可以选择立刻刷新(存在画面撕裂问题)、
    * 队列模式(类垂直同步，存在延迟问题)、
    * 队列Relax模式(也存在可见的画面撕裂)
    * 以及三缓冲机制
    */
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

/**
 * 选择Vulkan表面的交换范围。
 *
 * @param capabilities Vulkan表面的能力
 *
 * @return 选择的交换范围
 *
 * @throws ErrorType 如果在选择过程中发生错误
 */
VkExtent2D EngineCore::MioEngine::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities){
    //分辨率
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(m_window, &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };
        
        //剪切分辨率到兼容的最小和最大范围内
        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}


VkShaderModule EngineCore::MioEngine::createShaderModule(const std::vector<char>& code){
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
    VkShaderModule shaderModule;
    if (vkCreateShaderModule(m_logicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }
    return shaderModule;
}

/**
 * 获取 EngineCore 类中 MioEngine 的受支持扩展。
 *
 * @return void
 *
 * @throws None
 */
void EngineCore::MioEngine::getSupportedExtensions(){
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    m_extensions = std::vector<VkExtensionProperties>(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, m_extensions.data());
#if _debug
    std::cout << std::format("available extensions:{}\n", m_extensions.size());
    for (const auto& extension : m_extensions) {
        std::cout << '\t' << extension.extensionName << '\n';
    }
#endif
}

/**
 * 为EngineCore的MioEngine类填充调试信使创建信息。
 *
 * @param createInfo 要填充的VkDebugUtilsMessengerCreateInfoEXT对象的引用。
 *
 * @return void
 *
 * @throws None
 */
void EngineCore::MioEngine::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
}

/**
 * 设置调试信使函数，用于在EngineCore类中的MioEngine中设置调试消息处理。
 * 该函数负责创建Vulkan API中处理调试消息所需的结构体和回调函数。
 *
 * @throws std::runtime_error 如果调试信使设置失败。
 */
void EngineCore::MioEngine::setupDebugMessenger() {
    if (!enableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
    populateDebugMessengerCreateInfo(createInfo);

    if (createDebugUtilsMessengerEXT(m_instances, &createInfo, nullptr, &m_debugMessenger) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to set up debug messenger!");
    }
}


/**
 * 销毁调试工具信使扩展。
 *
 * @param instance Vulkan实例。
 * @param debugMessenger 要销毁的调试信使。
 * @param pAllocator 分配回调。
 *
 * @throws ErrorType 错误的描述。
 */
void EngineCore::MioEngine::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator){
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

/**
 * 在 EngineCore 类中为 MioEngine 创建一个表面。
 *
 * @throws std::runtime_error 如果无法创建窗口表面
 */
void EngineCore::MioEngine::createSurface() {
    if (glfwCreateWindowSurface(m_instances, m_window, nullptr, &m_surface) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create window surface!");
    }
}

/**
 * 选择适合的物理设备用于 EngineCore 的 MioEngine。
 *
 * @throws std::runtime_error 如果找不到支持 Vulkan 的 GPU
 *
 * @return void
 */
void EngineCore::MioEngine::pickPhysicalDevice(){
    m_physicalDevice = VK_NULL_HANDLE;
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_instances, &deviceCount, nullptr);
    if (deviceCount == 0)
    {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }
    // 遍历所有可用的GPU后，写入m_physicalDevices
    m_physicalDevices.resize(deviceCount);
    vkEnumeratePhysicalDevices(m_instances, &deviceCount, m_physicalDevices.data());

    std::multimap<int, VkPhysicalDevice> candidates;

    for(const auto& device: m_physicalDevices){
        if (isDeviceSuitable(device)){
            int score = rateDeviceSuitability(device);
            candidates.insert(std::make_pair(score, device));
            break;
        }
    }

    //检查是否有合适的GPU  
    if (candidates.rbegin()->first > 0) {
        m_physicalDevice = candidates.rbegin()->second;
    } else {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}

/**
 * 创建逻辑设备。
 *
 * @param None
 *
 * @return None
 *
 * @throws std::runtime_error 如果逻辑设备创建失败
 */
void EngineCore::MioEngine::createLogicalDevice(){
    QueueFamilyIndices indices = findQueueFamily(m_physicalDevices.front());

    //设备队列创建信息
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    float queuePriority = 1.0f;  //队列优先级，影响执行顺序
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    //设备支持的特征，在选物理设备时已经指定
    VkPhysicalDeviceFeatures deviceFeatures{};

    //设备创建信息
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }


    //创建逻辑设备
    if (vkCreateDevice(m_physicalDevices.front(), &createInfo, nullptr, &m_logicalDevice) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }
    
    //创建设备队列
    vkGetDeviceQueue(m_logicalDevice, indices.graphicsFamily.value(), 0, &m_graphicsQueue);
    vkGetDeviceQueue(m_logicalDevice, indices.presentFamily.value(), 0, &presentQueue);
}

/**
 * 在EngineCore类中为MioEngine创建一个交换链。
 *
 * @throws std::runtime_error 如果交换链创建失败
 */
void EngineCore::MioEngine::createSwapChain() {
    SwapChainSupportDetails details = querySwapChainSupport(m_physicalDevice);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(details.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(details.presentModes);
    VkExtent2D extent = chooseSwapExtent(details.capabilities);

    uint32_t imageCount = details.capabilities.minImageCount + 1; //+1避免程序一直等待，不计算可用图像

    if (details.capabilities.maxImageCount > 0 && imageCount > details.capabilities.maxImageCount) {
        imageCount = details.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = findQueueFamily(m_physicalDevice);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0; // Optional
        createInfo.pQueueFamilyIndices = nullptr; // Optional
    }
    createInfo.preTransform = details.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(m_logicalDevice, &createInfo, nullptr, &m_swapChain) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(m_logicalDevice, m_swapChain, &imageCount, nullptr);
    m_swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(m_logicalDevice, m_swapChain, &imageCount, m_swapChainImages.data());
    m_swapChainImageFormat = surfaceFormat.format;
    m_swapChainExtent = extent;
}

/**
 * 创建交换链图像视图。
 *
 * @throws std::runtime_error 如果创建图像视图失败。
 */
void EngineCore::MioEngine::createImageViews() {
    m_swapChainImageViews.resize(m_swapChainImages.size());

    for (size_t i = 0; i < m_swapChainImages.size(); i++) {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = m_swapChainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = m_swapChainImageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(m_logicalDevice, &createInfo, nullptr, &m_swapChainImageViews[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image views!");
        }
    }
}

void EngineCore::MioEngine::createRenderPass() {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = m_swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    //子通道
    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    //渲染通道
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    if (vkCreateRenderPass(m_logicalDevice, &renderPassInfo, nullptr, &m_renderPass)) {
        throw std::runtime_error("failed to create render pass!");
    }
}

void EngineCore::MioEngine::createGraphicsPipeline() {
    //可编程管线部分
    auto vertShaderCode = EngineUtils::readFile("shaders/triangles/vert.spv");
    auto fragShaderCode = EngineUtils::readFile("shaders/triangles/frag.spv");

    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";
    vertShaderStageInfo.pSpecializationInfo = nullptr; //设置常量信息，来消除if

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";
    fragShaderStageInfo.pSpecializationInfo = nullptr; //设置常量信息，来消除if

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    //顶点输入
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexBindingDescriptions = nullptr;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = nullptr;

    //输入Assembly
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    //固定功能部分
    std::vector<VkDynamicState> dynamicStates= {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    //视口
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)m_swapChainExtent.width;
    viewport.height = (float)m_swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    //裁剪
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = m_swapChainExtent;

    //设置动态状态
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data(); 

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    //光栅化
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    //深度剪切与偏差
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;

    //反走样
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    //深度和模板测试

    //颜色混合
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    /*默认混合
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    */

    /*
    * finalColor.rgb = newAlpha * newColor + (1 - newAlpha) * oldColor
    * finalColor.a = newAlpha.a 
    */
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE; //采用位混合方式
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    //管线布局
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;

    if (vkCreatePipelineLayout(m_logicalDevice, &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    vkDestroyShaderModule(m_logicalDevice, fragShaderModule, nullptr);
    vkDestroyShaderModule(m_logicalDevice, vertShaderModule, nullptr);
}

/**
 * 评估给定的 Vulkan 物理设备的适用性。
 *
 * @param device 要评估的 Vulkan 物理设备。
 *
 * @return 表示设备适用性的分数。
 *
 * @throws None
 */
int EngineCore::MioEngine::rateDeviceSuitability(VkPhysicalDevice device){
    int score = 0;

    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        score += 1000;
    }
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
    if (deviceFeatures.geometryShader) {
        score += 1000;
    }
    return score;
}


/**
 * 主循环函数。
 *
 * 该函数用于执行引擎核心的主循环。
 *
 * @throws ErrorType 如果在主循环过程中出现错误。
 *
 * @return void
 */
void EngineCore::MioEngine::mainLoop(){
    while(!glfwWindowShouldClose(m_window)){
        glfwPollEvents();
    }
}

/**
 * 清理引擎核心的MioEngine。
 *
 * 该函数用于清理引擎核心中的MioEngine。
 *
 * @throws ErrorType 如果在清理过程中出现错误。
 *
 * @return void
 */
void EngineCore::MioEngine::cleanup(){
    //清理管线布局
    vkDestroyPipelineLayout(m_logicalDevice, m_pipelineLayout, nullptr);

    //清理渲染通道
    vkDestroyRenderPass(m_logicalDevice, m_renderPass, nullptr);

    //清理图片视图
    for (auto imageView : m_swapChainImageViews){
        vkDestroyImageView(m_logicalDevice, imageView, nullptr);
    }

    //清理交换链
    vkDestroySwapchainKHR(m_logicalDevice, m_swapChain, nullptr);

    //清理逻辑设备
    vkDestroyDevice(m_logicalDevice, nullptr);

    //清理Vulkan实例
    if (enableValidationLayers)
    {
        // 清理调试信使
        DestroyDebugUtilsMessengerEXT(m_instances, m_debugMessenger, nullptr);
    }

    // 清理显示窗口
    vkDestroySurfaceKHR(m_instances, m_surface, nullptr);
    // 清理Vulkan实例
    vkDestroyInstance(m_instances, nullptr);

    //清理GLFW
    glfwDestroyWindow(m_window);
    glfwTerminate();
}