#include "VulkanUniformBuffer.h"

namespace VulkanRenderer
{
    VulkanUniformBuffer::VulkanUniformBuffer(VkPhysicalDevice& physicalDevice, VkDevice& device, int maxFramesInFlight) 
    {
        _device = &device;
        _maxFramesInFlight = maxFramesInFlight;

        _modelIndex = 0;

        //simple old version
        //VkDeviceSize bufferSize = sizeof(UniformBufferObject);

        // Size of view and proj matrices in UBO, also modelindex
        VkDeviceSize uboSize = sizeof(glm::mat4) * 2;
        // Size of each model transformation matrix
        VkDeviceSize modelTransformSize = sizeof(glm::mat4);
        // Total size of UBO (view, proj) + (model transformations for all models)
        VkDeviceSize bufferSize = uboSize + modelTransformSize * MAX_MODELS;

        UniformBuffers.resize(_maxFramesInFlight);
        UniformBuffersMemory.resize(_maxFramesInFlight);
        UniformBuffersMapped.resize(_maxFramesInFlight);

        for (size_t i = 0; i < _maxFramesInFlight; i++)
        {
            CreateBuffer(physicalDevice,
                         device,
                         bufferSize, 
                         VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         UniformBuffers[i],
                         UniformBuffersMemory[i]);

            vkMapMemory(device, UniformBuffersMemory[i], 0, bufferSize, 0, (void**) /*test*/ &UniformBuffersMapped[i]);
        }


    }

    VulkanUniformBuffer::~VulkanUniformBuffer()
    {
        for (size_t i = 0; i < _maxFramesInFlight; i++)
        {
            vkDestroyBuffer(*_device, UniformBuffers[i], nullptr);
            vkFreeMemory(*_device, UniformBuffersMemory[i], nullptr);
        }
    }

    void VulkanUniformBuffer::Update(uint32_t currentImage, VkExtent2D swapChainExtent)
    {
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        //TODO: Don't create a new one every frame...use the same one instead
        //ViewProjectionUBO ubo{};
        
        UniformBuffersMapped[currentImage]->models[0].Transform = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        
        //test
        //UniformBuffersMapped[currentImage]->models[1].Transform = glm::rotate(glm::mat4(1.0f), time * glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));

        UniformBuffersMapped[currentImage]->view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        UniformBuffersMapped[currentImage]->proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 10.0f);
        UniformBuffersMapped[currentImage]->proj[1][1] *= -1;

        //memcpy(UniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
    }

    uint32_t VulkanUniformBuffer::AddTransform()
    {
        uint32_t index = 0;

        int sz = MAX_MODELS; //UniformBuffersMapped[0]->models.size();

        if (_modelIndex >= sz)
        {
            throw std::exception();
        }

        //no slots, add a new one
        if (_modelIndex <= 0)
        {
            for (int i = 0; i < _maxFramesInFlight; i++)
                UniformBuffersMapped[i]->models[0] = ModelObject();

            return _modelIndex++;
        }

        //no free slots, add a new one
        if (_freeChunks.empty())
        {
            for (int i = 0; i < _maxFramesInFlight; i++)
                UniformBuffersMapped[i]->models[_modelIndex] = ModelObject();

            //index = sz; //note, not sz - 1 so it's the latest <- ???, we want the index to the model
            return _modelIndex++;
        }

        //get index from start of first free chunk
        FreeChunk chunk = _freeChunks.back();
        index = chunk.start;
        chunk.start++;

        //remove chunk if no more room in it
        if (chunk.start >= chunk.end)
        {
            _freeChunks.pop_back();
        }       

        //init model back to identity matrix
        for (int i = 0; i < _maxFramesInFlight; i++)
            UniformBuffersMapped[i]->models[index].Transform = glm::mat4();

        return index;
    }
    void VulkanUniformBuffer::RemoveTransform(uint32_t index)
    {
        //
        if (index == _modelIndex)
        {
            _modelIndex--;
            return;
        }

        if (_freeChunks.empty())
        {
            _freeChunks.push_back({index, index});
            return;
        }

        for (FreeChunk& chunk : _freeChunks)
        {
            if (index == chunk.start - 1)
            {
                chunk.start--;
                return;
            }

            if (index == chunk.end + 1)
            {
                chunk.end++;
                return;
            }
        }
        _freeChunks.push_back({ index, index });        
    }

    glm::mat4 VulkanUniformBuffer::GetTransform(uint32_t index, int frameIndex)
    {
        return UniformBuffersMapped[frameIndex]->models[index].Transform;
    }

    void VulkanUniformBuffer::SetTransform(uint32_t index, glm::mat4 transform, int frameIndex)
    {
        UniformBuffersMapped[frameIndex]->models[index].Transform = transform;        
    }
}