#include "GameObject.h"

//GameObject::GameObject() : Model(nullptr)
//{
//    RotationMultiplier = 1.0f;
//}

void GameObject::Update()
{
    if (Visible)
    {

    }


    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    Transform = glm::rotate(glm::mat4(1.0f), RotationMultiplier * time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
}

void GameObject::Render()
{
    //We could just render all gameobjects using the same model one after the other using EnTT
    //Then there's no need for intermediate vector inside Model
    Model->AddInstanceData(Transform);
}
