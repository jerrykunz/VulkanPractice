
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
#include "Input.h"
#include "Camera.h"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;


bool firstMouse = true;
int prev_mouse_x;
int prev_mouse_y;
int mouse_offset_x;
int mouse_offset_y;

static void FramebufferResizeCallback(GLFWwindow* window, int width, int height) 
{
    auto app = reinterpret_cast<VulkanRenderer::VulkanContext*>(glfwGetWindowUserPointer(window));
    app->FrameBufferResized = true;
}

static void MouseCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {        
        prev_mouse_x = xpos;
        prev_mouse_y = ypos;
        firstMouse = false;
    }

    mouse_offset_x += xpos - prev_mouse_x;
    mouse_offset_y += ypos - prev_mouse_y;
    prev_mouse_x = xpos;
    prev_mouse_y = ypos;
}


static Input GetInput(GLFWwindow* window)
{
    int forward = glfwGetKey(window, GLFW_KEY_W);
    int backward = glfwGetKey(window, GLFW_KEY_S);
    int left = glfwGetKey(window, GLFW_KEY_A);
    int right = glfwGetKey(window, GLFW_KEY_D);

    int up = glfwGetKey(window, GLFW_KEY_SPACE);
    int down = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL);

    if (up || down)
    {
        int a = 3;
    }

    return Input
    {
        .forward = forward,
        .backward = backward,
        .left = left,
        .right = right,
        .up = up,
        .down = down,
        .mouseMoveX = mouse_offset_x,
        .mouseMoveY = mouse_offset_y
    };
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

    //lock mouse cursor to window
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwSetCursorPosCallback(window, MouseCallback);


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
    VulkanRenderer::VulkanModel model2("models/viking_room.obj",
        texture,
        vulkanContext.PhysicalDevice->Device,
        vulkanContext.Device->Device,
        vulkanContext.Device->GraphicsQueue,
        vulkanContext.CommandPool);
    vulkanContext.Models.push_back(&model2);

    std::vector<GameObject> objects;

    Camera camera;
    camera.type = Camera::CameraType::firstperson;
    camera.flipY = -1.0f;
    camera.setPerspective(60.0f, (float)WIDTH / (float)HEIGHT, 0.1f, 512.0f);
    //1.0f to x means we go left from origin even we should go right, this sucks absolute balls and I don't know how to fix it
    //looks like this is now fixed with some '-'s added/removed in camera.cpp
    camera.setTranslation(glm::vec3(1.0f, 0.0f, -2.0f));
    camera.setRotation(glm::vec3(0.0f, 0.0f, 0.0f));
    camera.movementSpeed = 3.0f;

    GameObject object1
    {
        .Visible = true,
        .Transform = glm::mat4(),
        .Model = &model,
        .RotationMultiplier = 1.0f,        
    };
    objects.push_back(object1);

    GameObject object2
    {
        .Visible = true,
        .Transform = glm::mat4(),
        .Model = &model2,
        .RotationMultiplier = 0.1f,
    };
    objects.push_back(object2);
    
    //Do this now that we have all the images/models set up
    vulkanContext.CreateDescriptorSets();

    const double targetFrameRate = 60.0;
    const double frameTime = 1.0 / targetFrameRate;
    double lastFrameTime = glfwGetTime();


    //loop
    while (!glfwWindowShouldClose(window)) 
    {
        glfwPollEvents();

        double currentFrameTime = glfwGetTime();
        double deltaTime = currentFrameTime - lastFrameTime;

        if (deltaTime >= frameTime) 
        {
            lastFrameTime = currentFrameTime;

            Input input = GetInput(window);
            //reset mouse movement amount for next frame
            mouse_offset_x = 0;
            mouse_offset_y = 0;


            camera.keys.left = input.left;
            camera.keys.right = input.right;
            camera.keys.forward = input.forward;
            camera.keys.backward = input.backward;
            camera.keys.up = input.up;
            camera.keys.down = input.down;
            camera.rotate(glm::vec3(-input.mouseMoveY * 0.01f, input.mouseMoveX * 0.01f, 0.0f));
            camera.update(deltaTime);

            //get instance visibility
            for (GameObject& obj : objects)
            {
                obj.Update();
            }

            //Update instanceDataUBO allocation according to visible instances
            int modelCount = vulkanContext.Models.size();
            int index = 0;
            for (int i = 0; i < modelCount; i++)
            {
                //get count of instances marked as visible in GameObject.Update()
                uint8_t instanceCount = vulkanContext.Models[i]->instanceCount;

                //no visible instances, skip altogether
                //not needed I think
               /* if (instanceCount <= 0)
                    continue;*/

                    //set range of instances
                vulkanContext.Models[i]->instancesEnd = index + instanceCount;
                vulkanContext.Models[i]->instancesIndex = index;

                //update index for next model
                index = index + instanceCount;
            }

            auto viewProjUBO = vulkanContext.GetViewProjectionUBO();
            viewProjUBO->proj = camera.matrices.perspective;
            //viewProjUBO->proj[1][1] *= -1;
            viewProjUBO->view = camera.matrices.view;
           

            auto instanceDataUBO = vulkanContext.GetInstanceDataUBO();
            for (GameObject& obj : objects)
            {
                obj.Render(instanceDataUBO);
            }

            glm::mat4 transformMatrix(1.0f);
            float scaleFactor = 100.0f;  
            transformMatrix = glm::scale(transformMatrix, glm::vec3(scaleFactor, scaleFactor, 1.0f));
            vulkanContext.RenderQuad(transformMatrix, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

            vulkanContext.DrawFrame(input, window);
        }
    }

    vkDeviceWaitIdle(vulkanContext.Device->Device);    

    //cleanup
    vulkanContext.CleanUp();
    glfwDestroyWindow(window);
    glfwTerminate();

	return 0;
}
