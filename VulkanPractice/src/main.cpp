
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

//#ifndef STB_IMAGE_IMPLEMENTATION
//#define STB_IMAGE_IMPLEMENTATION
//#include <stb_image.h>
//#endif

//#ifndef TINYOBJLOADER_IMPLEMENTATION
//#define TINYOBJLOADER_IMPLEMENTATION
//#include <tiny_obj_loader.h>
//#endif

#include "VulkanContext.h"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

static void FramebufferResizeCallback(GLFWwindow* window, int width, int height) 
{
    auto app = reinterpret_cast<VulkanRenderer::VulkanContext*>(glfwGetWindowUserPointer(window));
    app->FrameBufferResized = true;
}

int main() 
{
    //init
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);

    VulkanRenderer::VulkanContext vulkanContext(window, "app", "engine");
    glfwSetWindowUserPointer(window, &vulkanContext);
    glfwSetFramebufferSizeCallback(window, FramebufferResizeCallback);

    //loop
    while (!glfwWindowShouldClose(window)) 
    {
        glfwPollEvents();
        vulkanContext.DrawFrame(window);
    }

    vkDeviceWaitIdle(vulkanContext.Device->Device);    

    //cleanup
    vulkanContext.CleanUp();
    glfwDestroyWindow(window);
    glfwTerminate();

	return 0;
}
