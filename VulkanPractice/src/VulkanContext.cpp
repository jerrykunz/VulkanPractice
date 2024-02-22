#include "VulkanContext.h"




namespace VulkanRenderer
{
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

        _textureSlots.resize(_maxTextures);
        _textureIndex = 0;

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
        //CreateGraphicsPipeline2DQuad();

        _quadPipeline = new VulkanPipeline(Device->Device,
                                           _renderPass,
                                           PhysicalDevice->MsaaSamples,
                                           _descriptorSetLayout,
                                           "shaders/quad2dvert.spv",
                                           "shaders/quad2dfrag.spv",
                                           QuadVertex::getBindingDescription(),
                                           QuadVertex::getAttributeDescriptions2(),
                                           VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                                           VK_POLYGON_MODE_FILL);

        _linePipeline = new VulkanPipeline(Device->Device,
                                           _renderPass,
                                           PhysicalDevice->MsaaSamples,
                                           _descriptorSetLayout,
                                           "shaders/line2dvert.spv",
                                           "shaders/line2dfrag.spv",
                                           LineVertex::getBindingDescription(),
                                           LineVertex::getAttributeDescriptions(),
                                           VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
                                           VK_POLYGON_MODE_FILL);


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
        InitQuadRendering();
        InitLineRendering();

        //test
       /* glm::vec3 _position(1.0f, 1.0f, 1.0f);
        prevCameraFront = glm::vec3(0.0f, 0.0f, 0.0f);
        viewMatrix = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));*/
        //viewMatrix = glm::translate(viewMatrix, -_position);
    }

    VulkanContext::~VulkanContext()
    {

    }

    void VulkanContext::InitQuadRendering()
    {
        uint32_t white = 0xffffffff;
        WhiteTexture = new VulkanImage(&white, PhysicalDevice->Device, *Device, CommandPool, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        _textureSlots[_textureIndex] = WhiteTexture;
        _textureIndex++; //1 after this

        _maxQuadVertices = 40000;
        _maxQuadIndices = 60000;

        QuadVertexCount = 0;
        QuadIndexCount = 0;

        QuadVertexBuffer.resize(_maxFramesInFlight);
        QuadVertices.resize(_maxFramesInFlight);

        for (int i = 0; i < _maxFramesInFlight; i++)
        {
            QuadVertexBuffer[i] = VulkanVertexBuffer();
            QuadVertices[i] = new QuadVertex[_maxQuadVertices];
        }

        _quadVertexPositions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
        _quadVertexPositions[1] = { -0.5f,  0.5f, 0.0f, 1.0f };
        _quadVertexPositions[2] = { 0.5f,  0.5f, 0.0f, 1.0f };
        _quadVertexPositions[3] = { 0.5f, -0.5f, 0.0f, 1.0f };



        //Load indices, always the same so no need for per frame indices
        QuadIndexBuffer = VulkanIndexBuffer();        
        //uint32_t* quadIndices = new uint32_t[_maxQuadIndices];
        std::vector<uint32_t> quadIndices(_maxQuadIndices);
        uint32_t offset = 0;
        for (uint32_t i = 0; i < _maxQuadIndices; i += 6)
        {
            quadIndices[i + 0] = offset + 0;
            quadIndices[i + 1] = offset + 1;
            quadIndices[i + 2] = offset + 2;

            quadIndices[i + 3] = offset + 2;
            quadIndices[i + 4] = offset + 3;
            quadIndices[i + 5] = offset + 0;

            offset += 4;
        }
        QuadIndexBuffer.LoadIndices(quadIndices,
                                    PhysicalDevice->Device,
                                    Device->Device,
                                    Device->GraphicsQueue,
                                    CommandPool);
    }

    void VulkanContext::InitLineRendering()
    {
        _maxLineVertices = 20000;
        _maxLineIndices = 20000;

        LineVertexCount = 0;
        LineIndexCount = 0;

        LineVertexBuffer.resize(_maxFramesInFlight);
        LineVertices.resize(_maxFramesInFlight);

        for (int i = 0; i < _maxFramesInFlight; i++)
        {
            LineVertexBuffer[i] = VulkanVertexBuffer();
            LineVertices[i] = new LineVertex[_maxLineVertices];
        }



        LineIndexBuffer = VulkanIndexBuffer();
        std::vector<uint32_t> lineIndices(_maxLineIndices);

        for (int i = 0; i < _maxLineIndices; i++)
        {
            lineIndices[i] = i;
        }

        LineIndexBuffer.LoadIndices(lineIndices,
                                    PhysicalDevice->Device,
                                    Device->Device,
                                    Device->GraphicsQueue,
                                    CommandPool);
    }

    void VulkanContext::DrawFrame(GLFWwindow* window)
    {
        vkWaitForFences(Device->Device, 1, &_inFlightFences[CurrentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(Device->Device, _swapChain->SwapChain, UINT64_MAX, _imageAvailableSemaphores[CurrentFrame], VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) 
        {
            RecreateSwapChain(window);
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) 
        {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        vkResetFences(Device->Device, 1, &_inFlightFences[CurrentFrame]);

        vkResetCommandBuffer(_commandBuffers[CurrentFrame], /*VkCommandBufferResetFlagBits*/ 0);

        RecordCommandBuffer(_commandBuffers[CurrentFrame], imageIndex);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { _imageAvailableSemaphores[CurrentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &_commandBuffers[CurrentFrame];

        VkSemaphore signalSemaphores[] = { _renderFinishedSemaphores[CurrentFrame] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(Device->GraphicsQueue, 1, &submitInfo, _inFlightFences[CurrentFrame]) != VK_SUCCESS)
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

        CurrentFrame = (CurrentFrame + 1) % _maxFramesInFlight;

        //Reset Quad rendering textures
        _textureIndex = 1;
    }

    void VulkanContext::CleanUp()
    {
        delete _frameBuffer;
        _frameBuffer = nullptr;

        delete _swapChain;
        _swapChain = nullptr;

        /*vkDestroyPipeline(Device->Device, _GraphicsPipeline2DQuad, nullptr);
        vkDestroyPipelineLayout(Device->Device, _pipelineLayout2DQuad, nullptr);*/
        _quadPipeline->Dispose(Device->Device);

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

        //we can do this after viewport and scissor
        //vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _graphicsPipeline);

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

        //2D quad rendering
        {
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _quadPipeline->Pipeline  /*_GraphicsPipeline2DQuad*/);

            //moved here, still causes errors
            UpdateTextureDescriptorSets();

            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _quadPipeline->PipelineLayout /*_pipelineLayout2DQuad*/, 0, 1, &_descriptorSets[CurrentFrame], 0, nullptr);

            QuadVertexBuffer[CurrentFrame].LoadVertices(QuadVertices[CurrentFrame],
                QuadVertexCount,
                PhysicalDevice->Device,
                Device->Device,
                Device->GraphicsQueue,
                CommandPool);

            VkBuffer vertexBuffers[] = { QuadVertexBuffer[CurrentFrame].VertexBuffer };
            VkDeviceSize offsets[] = { 0 };
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

            vkCmdBindIndexBuffer(commandBuffer, QuadIndexBuffer.IndexBuffer, 0, VK_INDEX_TYPE_UINT32);

            vkCmdDrawIndexed(commandBuffer,
                static_cast<uint32_t>(QuadIndexCount * sizeof(uint32_t)),
                1,
                0,
                0,
                0);

            QuadVertexCount = 0;
            QuadIndexCount = 0;
        }
        //END 2D quad rendering END

        //2d line rendering
        if (LineVertexCount > 0)
        {
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _linePipeline->Pipeline);

            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _linePipeline->PipelineLayout, 0, 1, &_descriptorSets[CurrentFrame], 0, nullptr);

            LineVertexBuffer[CurrentFrame].LoadVertices(LineVertices[CurrentFrame],
                                                        LineVertexCount,
                                                        PhysicalDevice->Device,
                                                        Device->Device,
                                                        Device->GraphicsQueue,
                                                        CommandPool);

            VkBuffer vertexBuffers[] = { LineVertexBuffer[CurrentFrame].VertexBuffer };
            VkDeviceSize offsets[] = { 0 };
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

            vkCmdBindIndexBuffer(commandBuffer, LineIndexBuffer.IndexBuffer, 0, VK_INDEX_TYPE_UINT32);

            vkCmdDrawIndexed(commandBuffer,
                static_cast<uint32_t>(LineIndexCount * sizeof(uint32_t)),
                1,
                0,
                0,
                0);

            LineVertexCount = 0;
            LineIndexCount = 0;
        }
        //END 2D line rendering END

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

    void VulkanContext::CreateDescriptorPool()
    {
        size_t sz = UniformBuffers.size();

        std::vector<VkDescriptorPoolSize> poolSizes{};
        poolSizes.resize(sz + 1);

        for (int i = 0; i < sz; i++)
        {
            poolSizes[i].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            poolSizes[i].descriptorCount = static_cast<uint32_t>(_maxFramesInFlight);
        }

        poolSizes[sz] = VkDescriptorPoolSize
                        {
                            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                            //Added Maxtextures here, previously just _maxFramesInFlight, so 1 texture slot per frame
                            .descriptorCount = static_cast<uint32_t>(_maxFramesInFlight * _maxTextures)
                        };

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
            }

            //size_t imgsSz = Images.size();
            size_t imgsSz = _textureIndex;

            std::vector<VkDescriptorImageInfo> imageDescriptors(_maxTextures);
            for (int j = 0; j < imgsSz; j++)
            {
                imageDescriptors[j] = _textureSlots[j]->Descriptor;
            }

            if (imgsSz < _maxTextures)
            {
                for (int j = imgsSz; j < _maxTextures; j++)
                {
                    imageDescriptors[j] = _textureSlots[0]->Descriptor; //use this as a test default
                }
            }

            //here we create the uniform buffer which shaders use
            //int sz = Images.size() + 1;
            //int sz = ubsSz + imgsSz;
            int sz = ubsSz + 1; //always 1 because it holds all the textures

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

            //texture sampler(s) for shader use
            for (int j = ubsSz, k = 0; j < sz; j++, k++)
            {
                descriptorWrites[j].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrites[j].dstSet = _descriptorSets[i];
                descriptorWrites[j].dstBinding = ubsSz; //1; //next available binding after ubs
                descriptorWrites[j].dstArrayElement = 0;
                descriptorWrites[j].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                descriptorWrites[j].descriptorCount = _maxTextures; //1;
                //descriptorWrites[j].pImageInfo = &imageDescriptors[k]; //&imageInfo;
                descriptorWrites[j].pImageInfo = imageDescriptors.data();
            }


            vkUpdateDescriptorSets(Device->Device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }
    }

    void VulkanContext::UpdateTextureDescriptorSets()
    {
        std::vector<VkDescriptorImageInfo> imageDescriptors(_maxTextures);
        for (int j = 0; j < _textureIndex; j++)
        {
            imageDescriptors[j] = _textureSlots[j]->Descriptor;
        }

        if (_textureIndex < _maxTextures)
        {
            for (int j = _textureIndex; j < _maxTextures; j++)
            {
                imageDescriptors[j] = _textureSlots[0]->Descriptor; //use this as a test default
            }
        }

        std::vector<VkWriteDescriptorSet> descriptorWrites(1);
        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = _descriptorSets[CurrentFrame]; //Only updates the currentframe, otherwise errors accessing data that's being rendered
        descriptorWrites[0].dstBinding = 2; //next available binding after ubs
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[0].descriptorCount = _maxTextures;
        descriptorWrites[0].pImageInfo = imageDescriptors.data();

        vkUpdateDescriptorSets(Device->Device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }

    void VulkanContext::RenderQuad(const glm::mat4& transform, const glm::vec4& color)
    {
        //constexpr size_t quadVertexCount = 4;
        const float textureIndex = 0.0f; // White Texture
        constexpr glm::vec2 textureCoords[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };
        const float tilingFactor = 1.0f;

        for (size_t i = 0; i < 4; i++)
        {
            size_t index = QuadVertexCount + i;
            QuadVertices[CurrentFrame][index].pos = transform * _quadVertexPositions[i];
            QuadVertices[CurrentFrame][index].color = color;
            QuadVertices[CurrentFrame][index].texCoord = textureCoords[i];
            QuadVertices[CurrentFrame][index].texIndex = textureIndex;
            QuadVertices[CurrentFrame][index].tilingFactor = tilingFactor;
        }

        QuadVertexCount += 4;
        QuadIndexCount += 6;
    }

    void VulkanContext::RenderQuad(const glm::mat4& transform, VulkanImage& texture, float tilingFactor, const glm::vec4& tintColor, glm::vec2 uv0, glm::vec2 uv1)
    {
        constexpr size_t quadVertexCount = 4;
        glm::vec2 textureCoords[] = { uv0, { uv1.x, uv0.y }, uv1, { uv0.x, uv1.y } };

        float textureIndex = 0.0f;
        for (uint32_t i = 1; i < _textureIndex; i++)
        {
            if (_textureSlots[i] == &texture)
            {
                textureIndex = (float)i;
                break;
            }
        }

        if (textureIndex == 0.0f)
        {
            textureIndex = (float)_textureIndex;
            _textureSlots[_textureIndex] = &texture;
            _textureIndex++;
        }

        for (size_t i = 0; i < 4; i++)
        {
            size_t index = QuadVertexCount + i;
            QuadVertices[CurrentFrame][index].pos = transform * _quadVertexPositions[i];
            QuadVertices[CurrentFrame][index].color = tintColor;
            QuadVertices[CurrentFrame][index].texCoord = textureCoords[i];
            QuadVertices[CurrentFrame][index].texIndex = textureIndex;
            QuadVertices[CurrentFrame][index].tilingFactor = tilingFactor;
        }

        QuadVertexCount += 4;
        QuadIndexCount += 6;
    }

    void VulkanContext::RenderLine(const glm::vec3 p1, const glm::vec3 p2, const glm::vec4& color1, const glm::vec4& color2)
    {
        LineVertices[CurrentFrame][LineVertexCount].pos = p1;
        LineVertices[CurrentFrame][LineVertexCount++].color = color1;

        LineVertices[CurrentFrame][LineVertexCount].pos = p2;
        LineVertices[CurrentFrame][LineVertexCount++].color = color2;

        LineIndexCount += 2;
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

    ViewProjectionUBO* VulkanContext::GetViewProjectionUBO()
    {
        return (ViewProjectionUBO*)ViewProjectionUniformBuffer->UniformBuffersMapped[CurrentFrame];
    }

    InstanceDataUBO* VulkanContext::GetInstanceDataUBO()
    {
        return (InstanceDataUBO*)InstanceDataUniformBuffer->UniformBuffersMapped[CurrentFrame];
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
        samplerLayoutBinding.descriptorCount = _maxTextures; //1;
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

    /*void VulkanContext::CreateGraphicsPipeline2DQuad()
    {
        auto vertShaderCode = ReadFile("shaders/2dvert.spv");
        auto fragShaderCode = ReadFile("shaders/2dfrag.spv");

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

        auto bindingDescription = QuadVertex::getBindingDescription();
        auto attributeDescriptions = QuadVertex::getAttributeDescriptions();

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
        //we want this clockwise, as our quad rendering is in that order
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
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

        std::vector<VkDynamicState> dynamicStates =
        {
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

        if (vkCreatePipelineLayout(Device->Device, &pipelineLayoutInfo, nullptr, &_pipelineLayout2DQuad) != VK_SUCCESS)
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
        pipelineInfo.layout = _pipelineLayout2DQuad;
        pipelineInfo.renderPass = _renderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        if (vkCreateGraphicsPipelines(Device->Device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_GraphicsPipeline2DQuad) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create graphics pipeline!");
        }

        vkDestroyShaderModule(Device->Device, fragShaderModule, nullptr);
        vkDestroyShaderModule(Device->Device, vertShaderModule, nullptr);
    }*/

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