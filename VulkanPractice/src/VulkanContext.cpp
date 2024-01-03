#include "VulkanContext.h"
#include "VulkanPushConstants.h"



namespace VulkanRenderer
{
    //testing

    float _pitch = 0.f;
    float _yaw = 0.f;
    glm::mat4 viewMatrix = glm::mat4(1.0f);
    glm::vec3 prevCameraFront;


    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                        VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, 
                                                        void* pUserData)
    {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }

    VulkanContext::VulkanContext(GLFWwindow* window,
                                 const std::string& applicationName, 
                                 const std::string& engineName)
    {       
        _enableValidationLayers = true;
        _maxFramesInFlight = 2;

        InitializeInstance(applicationName, engineName);
        InitializeDebugMessenger();
        CreateSurface(window);

        PhysicalDevice = new VulkanPhysicalDevice(Instance, 
                                                   Surface);

        Device = new VulkanDevice(_enableValidationLayers,
                                   _validationLayers,
                                   *PhysicalDevice);

        VulkanSwapChainSupportDetails swapChainDetails = PhysicalDevice->QuerySwapChainSupport(Surface);
        _swapChain = new VulkanSwapChain(*window,
                                        Device->Device,
                                        Surface,
                                        swapChainDetails,
                                        PhysicalDevice->QueueIndices);

        CreateRenderPass(PhysicalDevice,
                         Device,
                         _swapChain->SwapChainImageFormat);

        ViewProjectionUniformBuffer = new VulkanUniformBuffer(PhysicalDevice->Device,
                                                              Device->Device,
                                                              sizeof(ViewProjectionUBO),
                                                              _maxFramesInFlight,
                                                              0,
                                                              1,
                                                              VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                                              nullptr,
                                                              VK_SHADER_STAGE_VERTEX_BIT);
        UniformBuffers.push_back(ViewProjectionUniformBuffer);

        InstanceDataUniformBuffer = new VulkanUniformBuffer(PhysicalDevice->Device,
                                                            Device->Device,
                                                            sizeof(InstanceDataUBO),
                                                            _maxFramesInFlight,
                                                            1,
                                                            1,
                                                            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                                            nullptr,
                                                            VK_SHADER_STAGE_VERTEX_BIT);
        UniformBuffers.push_back(InstanceDataUniformBuffer);

        CreateDescriptorSetLayout();
        CreateGraphicsPipeline();
        CreateCommandPool();

        _frameBuffer = new VulkanFrameBuffer(*PhysicalDevice,
                                             Device->Device,
                                             _renderPass,
                                             _swapChain->SwapChainImageViews,
                                             _swapChain->SwapChainExtent,
                                             _swapChain->SwapChainImageFormat);

       

        CreateDescriptorPool();


        //CreateDescriptorSets();
        CreateCommandBuffers();
        CreateSyncObjects();

        //test
        glm::vec3 _position(1.0f, 1.0f, 1.0f);
        prevCameraFront = glm::vec3(0.0f, 0.0f, 0.0f);
        viewMatrix = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        //viewMatrix = glm::translate(viewMatrix, -_position);
    }

    VulkanContext::~VulkanContext()
    {

    }

    glm::mat4 VulkanContext::CreateViewMatrix(float pitch, float yaw, float roll, glm::vec3 position)
    {
       /* glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraFront = glm::normalize(front);*/


        glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, 0.0f);
        cameraFront.x = -cos(glm::radians(yaw)) * sin(glm::radians(pitch));
        cameraFront.y = sin(glm::radians(yaw));
        cameraFront.z = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraFront = glm::normalize(cameraFront);

        if (pitch != 0.f || yaw != 0.f)
        {
            int a = 3;
        }

        viewMatrix = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f),  cameraFront, glm::vec3(0.0f, 0.0f, 1.0f));

        // Create an identity matrix
        //glm::mat4 viewMatrix = glm::mat4(1.0f);

        // Apply roll (rotation around the forward axis)
        //viewMatrix = glm::rotate(viewMatrix, roll, glm::vec3(0.0f, 0.0f, 1.0f));

        // Apply pitch (rotation around the right axis)
        //viewMatrix = glm::rotate(viewMatrix, pitch, glm::vec3(1.0f, 0.0f, 0.0f));

        // Apply yaw (rotation around the up axis)
        //viewMatrix = glm::rotate(viewMatrix, yaw, glm::vec3(0.0f, 1.0f, 0.0f));

        // Apply translation to the camera position
        //viewMatrix = glm::translate(viewMatrix, -position);


        /*_pitch += pitch;
        _yaw += yaw;
        std::cout << _pitch << " " << _yaw << std::endl;*/

        return viewMatrix;
    }

    void VulkanContext::DrawFrame(Input& input, GLFWwindow* window)
    {
        vkWaitForFences(Device->Device, 1, &_inFlightFences[_currentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(Device->Device, _swapChain->SwapChain, UINT64_MAX, _imageAvailableSemaphores[_currentFrame], VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) 
        {
            RecreateSwapChain(window);
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) 
        {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        //ViewProjectionUniformBuffer->Update(_currentFrame, _swapChain->SwapChainExtent);

        vkResetFences(Device->Device, 1, &_inFlightFences[_currentFrame]);

        vkResetCommandBuffer(_commandBuffers[_currentFrame], /*VkCommandBufferResetFlagBits*/ 0);

        //moved here for testing purposes
        VkExtent2D swapChainExtent = _swapChain->SwapChainExtent;
        ViewProjectionUBO* viewProjUBO = (ViewProjectionUBO*)ViewProjectionUniformBuffer->UniformBuffersMapped[_currentFrame];

        if (input.mouseMoveY != 0)
        {
            int a = 3;
        }

        //viewProjUBO->view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        viewProjUBO->view = CreateViewMatrix(glm::radians(input.mouseMoveY * 0.01f), glm::radians(input.mouseMoveX * 0.01f), glm::radians(0.0f), glm::vec3(2.0f, 2.0f, 2.0f));

        viewProjUBO->proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 100.0f);
        viewProjUBO->proj[1][1] *= -1;


        RecordCommandBuffer(_commandBuffers[_currentFrame], imageIndex);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { _imageAvailableSemaphores[_currentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &_commandBuffers[_currentFrame];

        VkSemaphore signalSemaphores[] = { _renderFinishedSemaphores[_currentFrame] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(Device->GraphicsQueue, 1, &submitInfo, _inFlightFences[_currentFrame]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = { _swapChain->SwapChain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        presentInfo.pImageIndices = &imageIndex;

        result = vkQueuePresentKHR(Device->PresentQueue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || FrameBufferResized)
        {
            FrameBufferResized = false;
            RecreateSwapChain(window);
        }
        else if (result != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to present swap chain image!");
        }

        _currentFrame = (_currentFrame + 1) % _maxFramesInFlight;

    }

    void VulkanContext::CleanUp()
    {
        delete _frameBuffer;
        _frameBuffer = nullptr;

        delete _swapChain;
        _swapChain = nullptr;

        vkDestroyPipeline(Device->Device, _graphicsPipeline, nullptr);
        vkDestroyPipelineLayout(Device->Device, _pipelineLayout, nullptr);
        vkDestroyRenderPass(Device->Device, _renderPass, nullptr);

        delete ViewProjectionUniformBuffer;
        ViewProjectionUniformBuffer = nullptr;

        delete InstanceDataUniformBuffer;
        InstanceDataUniformBuffer = nullptr;

        vkDestroyDescriptorPool(Device->Device, _descriptorPool, nullptr);

        for (auto img : Images)
        {
            delete img;
        }
        Images.clear();

        vkDestroyDescriptorSetLayout(Device->Device, _descriptorSetLayout, nullptr);

        /*delete IndexBuffer;
        IndexBuffer = nullptr;

        delete VertexBuffer;
        VertexBuffer = nullptr;*/

        for (size_t i = 0; i < _maxFramesInFlight; i++) 
        {
            vkDestroySemaphore(Device->Device, _renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(Device->Device, _imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(Device->Device, _inFlightFences[i], nullptr);
        }

        vkDestroyCommandPool(Device->Device, CommandPool, nullptr);

        delete Device;
        Device = nullptr;

        if (_enableValidationLayers) 
        {
            DestroyDebugUtilsMessengerEXT(nullptr);
        }

        vkDestroySurfaceKHR(Instance, Surface, nullptr);
        vkDestroyInstance(Instance, nullptr);
    }

    void VulkanContext::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) 
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = _renderPass;
        renderPassInfo.framebuffer = _frameBuffer->SwapChainFramebuffers[imageIndex];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = _swapChain->SwapChainExtent;

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
        clearValues[1].depthStencil = { 1.0f, 0 };

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _graphicsPipeline);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)_swapChain->SwapChainExtent.width;
        viewport.height = (float)_swapChain->SwapChainExtent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = _swapChain->SwapChainExtent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 0, 1, &_descriptorSets[_currentFrame], 0, nullptr);


        //Camera
       /* VkExtent2D swapChainExtent = _swapChain->SwapChainExtent;
        ViewProjectionUBO* viewProjUBO = (ViewProjectionUBO*)ViewProjectionUniformBuffer->UniformBuffersMapped[_currentFrame];

        viewProjUBO->view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        viewProjUBO->proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 10.0f);
        viewProjUBO->proj[1][1] *= -1;*/

        //ortho test
       /* viewProjUBO->view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        viewProjUBO->proj = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 100.0f);*/


        

        //Batch drawcall per model, including all instances
        for (size_t i = 0; i < Models.size(); i++)
        {
            VkBuffer vertexBuffers[] = { Models[i]->VertexBuffer.VertexBuffer };
            VkDeviceSize offsets[] = { 0 };
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

            vkCmdBindIndexBuffer(commandBuffer, Models[i]->IndexBuffer.IndexBuffer, 0, VK_INDEX_TYPE_UINT32);

            //Update instance UBO
            size_t sz = Models[i]->Instances.size();
            InstanceDataUBO* instanceDataUBO = (InstanceDataUBO*)InstanceDataUniformBuffer->UniformBuffersMapped[_currentFrame];
            std::copy(Models[i]->Instances.begin(), Models[i]->Instances.end(), instanceDataUBO->instances);
            Models[i]->Instances.clear();            

            vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(Models[i]->IndexBuffer.Size), sz, 0, 0, 0);
        }


        vkCmdEndRenderPass(commandBuffer);

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to record command buffer!");
        }
    }

    void VulkanContext::InitializeInstance(const std::string& applicationName, const std::string& engineName)
    {
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = applicationName.c_str();
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = engineName.c_str();
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        auto extensions = GetRequiredExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        if (_enableValidationLayers)
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(_validationLayers.size());
            createInfo.ppEnabledLayerNames = _validationLayers.data();

            PopulateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
        }
        else
        {
            createInfo.enabledLayerCount = 0;

            createInfo.pNext = nullptr;
        }

        if (vkCreateInstance(&createInfo, nullptr, &Instance) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create instance!");
        }
    }

    void VulkanContext::CreateSyncObjects()
    {
        _imageAvailableSemaphores.resize(_maxFramesInFlight);
        _renderFinishedSemaphores.resize(_maxFramesInFlight);
        _inFlightFences.resize(_maxFramesInFlight);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < _maxFramesInFlight; i++) 
        {
            if (vkCreateSemaphore(Device->Device, &semaphoreInfo, nullptr, &_imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(Device->Device, &semaphoreInfo, nullptr, &_renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(Device->Device, &fenceInfo, nullptr, &_inFlightFences[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create synchronization objects for a frame!");
            }
        }
    }

    //should keep glfwindow out of this class maybe
    void VulkanContext::RecreateSwapChain(GLFWwindow* window)
    {
        int width = 0, height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        while (width == 0 || height == 0) 
        {
            glfwGetFramebufferSize(window, &width, &height);
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(Device->Device);

        delete _frameBuffer;
        delete _swapChain;

        VulkanSwapChainSupportDetails swapChainDetails = PhysicalDevice->QuerySwapChainSupport(Surface);
        _swapChain = new VulkanSwapChain(*window,
                                         Device->Device,
                                         Surface,
                                         swapChainDetails,
                                         PhysicalDevice->QueueIndices);


        _frameBuffer = new VulkanFrameBuffer(*PhysicalDevice,
                                             Device->Device,
                                             _renderPass,
                                             _swapChain->SwapChainImageViews,
                                             _swapChain->SwapChainExtent,
                                             _swapChain->SwapChainImageFormat);
    }

    void VulkanContext::CreateDescriptorPool(/*const std::vector<VulkanUniformBuffer*>& uniformBuffers*/)
    {
        size_t sz = UniformBuffers.size();

        std::vector<VkDescriptorPoolSize> poolSizes{};
        poolSizes.reserve(sz + 1);

        for (int i = 0; i < sz; i++)
        {
            poolSizes.push_back(VkDescriptorPoolSize
                                {
                                    .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                    .descriptorCount = static_cast<uint32_t>(_maxFramesInFlight)
                                });


           /* poolSizes[i].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            poolSizes[i].descriptorCount = static_cast<uint32_t>(_maxFramesInFlight);*/
        }

        poolSizes.push_back(VkDescriptorPoolSize
            {
                .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .descriptorCount = static_cast<uint32_t>(_maxFramesInFlight)
            });
    /*    poolSizes[sz].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[sz].descriptorCount = static_cast<uint32_t>(_maxFramesInFlight);*/

        /*std::array<VkDescriptorPoolSize, 2> poolSizes{};
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = static_cast<uint32_t>(_maxFramesInFlight);
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[1].descriptorCount = static_cast<uint32_t>(_maxFramesInFlight);*/

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = static_cast<uint32_t>(_maxFramesInFlight);

        if (vkCreateDescriptorPool(Device->Device, &poolInfo, nullptr, &_descriptorPool) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }

    void VulkanContext::CreateDescriptorSets()
    {
        std::vector<VkDescriptorSetLayout> layouts(_maxFramesInFlight, _descriptorSetLayout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = _descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(_maxFramesInFlight);
        allocInfo.pSetLayouts = layouts.data();

        _descriptorSets.resize(_maxFramesInFlight);
        if (vkAllocateDescriptorSets(Device->Device, &allocInfo, _descriptorSets.data()) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }

        for (size_t i = 0; i < _maxFramesInFlight; i++) 
        {
            /*VkDescriptorBufferInfo viewProjectionBufferInfo{};
            viewProjectionBufferInfo.buffer = ViewProjectionUniformBuffer->UniformBuffers[i];
            viewProjectionBufferInfo.offset = 0;
            viewProjectionBufferInfo.range = sizeof(ViewProjectionUBO);*/

            /*VkDescriptorBufferInfo InstanceDataBufferInfo{};
            viewProjectionBufferInfo.buffer = UniformBuffer->UniformBuffers[i];
            viewProjectionBufferInfo.offset = 0;
            viewProjectionBufferInfo.range = sizeof(InstanceDataUBO);*/

            size_t ubsSz = UniformBuffers.size();
            std::vector<VkDescriptorBufferInfo> uniformBuffers(ubsSz);
            for (int j = 0; j < ubsSz; j++)
            {
                uniformBuffers[j] = VkDescriptorBufferInfo
                {
                    .buffer = UniformBuffers[j]->UniformBuffers[i],
                    .offset = 0,
                    .range = UniformBuffers[j]->Size
                };
             /*   uniformBuffers.push_back(VkDescriptorBufferInfo
                {  
                    .buffer = UniformBuffers[j]->UniformBuffers[i],
                    .offset = 0,
                    .range = UniformBuffers[j]->Size
                });*/
            }

            size_t imgsSz = Images.size();
            std::vector<VkDescriptorImageInfo> imageDescriptors(imgsSz);
            for (int j = 0; j < imgsSz; j++)
            {
                imageDescriptors[j] = Images[j]->descriptor;
            }

            //here we create the uniform buffer which shaders use
            //int sz = Images.size() + 1;
            int sz = ubsSz + imgsSz;

            std::vector<VkWriteDescriptorSet> descriptorWrites(sz);

            for (int j = 0; j < ubsSz; j++)
            {
                descriptorWrites[j].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrites[j].dstSet = _descriptorSets[i];
                descriptorWrites[j].dstBinding = UniformBuffers[j]->DescriptorSetLayout.binding;  //0;
                descriptorWrites[j].dstArrayElement = 0;
                descriptorWrites[j].descriptorType = UniformBuffers[j]->DescriptorSetLayout.descriptorType; //VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                descriptorWrites[j].descriptorCount = UniformBuffers[j]->DescriptorSetLayout.descriptorCount; //1;
                descriptorWrites[j].pBufferInfo = &uniformBuffers[j]; //& viewProjectionBufferInfo;
            }

            /*descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = _descriptorSets[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &viewProjectionBufferInfo;*/

            //texture sampler(s) for shader use
            //for (int j = 1, k = 0; j < sz; j++, k++)
            for (int j = ubsSz, k = 0; j < sz; j++, k++)
            {
                descriptorWrites[j].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrites[j].dstSet = _descriptorSets[i];
                descriptorWrites[j].dstBinding = ubsSz; //1; //next available binding after ubs
                descriptorWrites[j].dstArrayElement = 0;
                descriptorWrites[j].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                descriptorWrites[j].descriptorCount = 1;
                descriptorWrites[j].pImageInfo = &imageDescriptors[k]; //&imageInfo;
            }

            vkUpdateDescriptorSets(Device->Device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }
    }

    std::vector<const char*> VulkanContext::GetRequiredExtensions()
    {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (_enableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }


    bool VulkanContext::CheckValidationLayerSupport()
    {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char* layerName : _validationLayers) {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) {
                return false;
            }
        }

        return true;
    }

    //DEBUG MESSENGER

    void VulkanContext::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
    }

    void VulkanContext::InitializeDebugMessenger()
    {
        if (!_enableValidationLayers) 
            return;

        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        PopulateDebugMessengerCreateInfo(createInfo);

        if (CreateDebugUtilsMessengerEXT(&createInfo, nullptr) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to set up debug messenger!");
        }
    }
    
    VkResult VulkanContext::CreateDebugUtilsMessengerEXT(const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator)
    {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(Instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr) 
        {
            return func(Instance, pCreateInfo, pAllocator, &_debugMessenger);
        }
        else 
        {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    void VulkanContext::DestroyDebugUtilsMessengerEXT(const VkAllocationCallbacks* pAllocator)
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(Instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) 
        {
            func(Instance, _debugMessenger, pAllocator);
        }
    } 


    //SURFACE

    void VulkanContext::CreateSurface(GLFWwindow* window)
    {
        if (glfwCreateWindowSurface(Instance, window, nullptr, &Surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
    }    

    //RENDER PASS



    void VulkanContext::CreateRenderPass(VulkanPhysicalDevice* physicalDevice,
                                         VulkanDevice* device,
                                         VkFormat& swapChainImageFormat)
    {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = swapChainImageFormat;
        colorAttachment.samples = physicalDevice->MsaaSamples;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = FindDepthFormat(physicalDevice->Device);
        depthAttachment.samples = physicalDevice->MsaaSamples;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentDescription colorAttachmentResolve{};
        colorAttachmentResolve.format = swapChainImageFormat;
        colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference colorAttachmentResolveRef{};
        colorAttachmentResolveRef.attachment = 2;
        colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;
        subpass.pResolveAttachments = &colorAttachmentResolveRef;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        std::array<VkAttachmentDescription, 3> attachments = { colorAttachment, depthAttachment, colorAttachmentResolve };
        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if (vkCreateRenderPass(device->Device, &renderPassInfo, nullptr, &_renderPass) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to create render pass!");
        }
    }

    VkFormat VulkanContext::FindDepthFormat(VkPhysicalDevice physicalDevice)
    {
        return FindSupportedFormat(physicalDevice,
                                   { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
                                   VK_IMAGE_TILING_OPTIMAL,
                                   VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    }

    VkFormat VulkanContext::FindSupportedFormat(VkPhysicalDevice physicalDevice,
                                                const std::vector<VkFormat>& candidates,
                                                VkImageTiling tiling,
                                                VkFormatFeatureFlags features)
    {
        for (VkFormat format : candidates) 
        {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                return format;
            }
            else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
                return format;
            }
        }

        throw std::runtime_error("failed to find supported format!");
    }

    void VulkanContext::CreateDescriptorSetLayout(/*const std::vector<VulkanUniformBuffer*>& uniformBuffers*/)
    {
        size_t uniformBuffersCount = UniformBuffers.size();

        std::vector< VkDescriptorSetLayoutBinding> bindings;
        for (const VulkanUniformBuffer* ub : UniformBuffers)
        {
            bindings.push_back(ub->DescriptorSetLayout);
        }

        //hardcoded UBO, not great
        /*VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.pImmutableSamplers = nullptr;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;*/

        VkDescriptorSetLayoutBinding samplerLayoutBinding{};
        samplerLayoutBinding.binding = uniformBuffersCount; //1; //next available binding after uniform buffers
        samplerLayoutBinding.descriptorCount = 1;
        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.pImmutableSamplers = nullptr;
        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        bindings.push_back(samplerLayoutBinding);

        //old bindings using array
        //std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(Device->Device, &layoutInfo, nullptr, &_descriptorSetLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }

    VkShaderModule VulkanContext::CreateShaderModule(const std::vector<char>& code)
    {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(Device->Device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to create shader module!");
        }

        return shaderModule;
    }

    static std::vector<char> ReadFile(const std::string& filename) 
    {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open()) 
        {
            throw std::runtime_error("failed to open file!");
        }

        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
    }

    void VulkanContext::CreateCommandPool()
    {
        VulkanQueueFamilyIndices queueFamilyIndices = PhysicalDevice->QueueIndices;

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

        if (vkCreateCommandPool(Device->Device, &poolInfo, nullptr, &CommandPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics command pool!");
        }
    }

    void VulkanContext::CreateGraphicsPipeline()
    {
        auto vertShaderCode = ReadFile("shaders/vert.spv");
        auto fragShaderCode = ReadFile("shaders/frag.spv");

        VkShaderModule vertShaderModule = CreateShaderModule(vertShaderCode);
        VkShaderModule fragShaderModule = CreateShaderModule(fragShaderCode);

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        auto bindingDescription = Vertex::getBindingDescription();
        auto attributeDescriptions = Vertex::getAttributeDescriptions();

        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = PhysicalDevice->MsaaSamples;

        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = VK_TRUE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.stencilTestEnable = VK_FALSE;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        //new
        VkPushConstantRange pushConstantRange = {};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; // Specify the shader stage
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(PushConstants);


        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &_descriptorSetLayout;

        //new
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

        if (vkCreatePipelineLayout(Device->Device, &pipelineLayoutInfo, nullptr, &_pipelineLayout) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to create pipeline layout!");
        }

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = _pipelineLayout;
        pipelineInfo.renderPass = _renderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        if (vkCreateGraphicsPipelines(Device->Device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_graphicsPipeline) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to create graphics pipeline!");
        }

        vkDestroyShaderModule(Device->Device, fragShaderModule, nullptr);
        vkDestroyShaderModule(Device->Device, vertShaderModule, nullptr);
    }

    void VulkanContext::CreateCommandBuffers() 
    {
        _commandBuffers.resize(_maxFramesInFlight);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = CommandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t)_commandBuffers.size();

        if (vkAllocateCommandBuffers(Device->Device, &allocInfo, _commandBuffers.data()) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }
}