
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
#include "VulkanModel.h"
#include "VulkanImage.h"
#include "GameObject.h"

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


    VulkanRenderer::VulkanImage texture("textures/viking_room.png",
                                        vulkanContext.PhysicalDevice->Device,
                                        *vulkanContext.Device,
                                        vulkanContext.CommandPool,
                                        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vulkanContext.Images.push_back(&texture);
    

    VulkanRenderer::VulkanModel model("models/viking_room.obj",
                                      texture,
                                      vulkanContext.PhysicalDevice->Device,
                                      vulkanContext.Device->Device, 
                                      vulkanContext.Device->GraphicsQueue,
                                      vulkanContext.CommandPool);
    vulkanContext.Models.push_back(&model);

    GameObject object1
    {
        .Transform = glm::mat4(),
        .Model = &model,
        .RotationMultiplier = 1.0f,        
    };

    GameObject object2
    {
        .Transform = glm::mat4(),
        .Model = &model,
        .RotationMultiplier = 0.1f,
    };
    
    //Do this now that we have all the images/models set up
    vulkanContext.CreateDescriptorSets();

    //loop
    while (!glfwWindowShouldClose(window)) 
    {
        glfwPollEvents();
        object1.Update();
        object2.Update();
        object1.Render();
        object2.Render();
        vulkanContext.DrawFrame(window);
    }

    vkDeviceWaitIdle(vulkanContext.Device->Device);    

    //cleanup
    vulkanContext.CleanUp();
    glfwDestroyWindow(window);
    glfwTerminate();

	return 0;
}
