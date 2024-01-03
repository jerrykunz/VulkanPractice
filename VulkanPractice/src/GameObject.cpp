#include "GameObject.h"


//GameObject::GameObject() : Model(nullptr)
//{
//    RotationMultiplier = 1.0f;
//}

void GameObject::Update()
{
    if (Visible)
    {
        Model->instanceCount++;
    }


    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    Transform = glm::rotate(glm::mat4(1.0f), RotationMultiplier * time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
}

void GameObject::Render(VulkanRenderer::InstanceDataUBO* ubo)
{
    if (Visible)
    {
        //TODO: update UBO at once before rendering, right now it's being streamed to GPU for each instance which probably sucks
        ubo->instances[Model->instancesIndex++] = VulkanRenderer::InstanceData{ .Transform = Transform };
    }
}
