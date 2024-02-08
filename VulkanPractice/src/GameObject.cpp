#include "GameObject.h"


//GameObject::GameObject() : Model(nullptr)
//{
//    RotationMultiplier = 1.0f;
//}

void GameObject::Update(float deltaTime)
{
    //test
   /* Transform = glm::mat4(1.0f);
    return;*/

    /*static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    Transform = glm::rotate(glm::mat4(1.0f), RotationMultiplier * time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));*/

    Transform = glm::rotate(Transform, RotationMultiplier * deltaTime * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
}

void GameObject::Render(VulkanRenderer::VulkanContext& vulkanContext)
{
    if (Visible)
    {
        //TODO: alpha blending is not working, need to change stuff in renderpipeline and renderpass maybe to allow it
        //vulkanContext.RenderQuad(Transform, Color);

        if (Texture != nullptr)
        {
            vulkanContext.RenderQuad(Transform, *Texture, 1.0f, Color, glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 1.0f));
        }
        else
        {
            vulkanContext.RenderQuad(Transform, Color);
        }
    }
}
