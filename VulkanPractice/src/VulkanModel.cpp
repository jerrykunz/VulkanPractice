#include "VulkanModel.h"


namespace VulkanRenderer
{
    VulkanModel::VulkanModel(const std::string& path, VulkanImage& image/*, VulkanUniformBuffer& uniformBuffer*/, VkPhysicalDevice& physicalDevice, VkDevice& device, VkQueue& graphicsQueue, VkCommandPool& commandPool) :
        Texture(&image)
    {
        //Texture = &image;
        //TransformIndex = uniformBuffer.AddTransform();
        //Transform = glm::mat4();

        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str())) 
        {
            throw std::runtime_error(warn + err);
        }

        std::unordered_map<Vertex, uint32_t> uniqueVertices{};

        for (const auto& shape : shapes)
        {
            for (const auto& index : shape.mesh.indices)
            {
                Vertex vertex{};

                vertex.pos = 
                {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };

                vertex.texCoord = 
                {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                };

                vertex.color = { 1.0f, 1.0f, 1.0f };

                //new
                vertex.texIndex = 0.0f;
                vertex.tilingFactor = 1.0f;

                if (uniqueVertices.count(vertex) == 0) {
                    uniqueVertices[vertex] = static_cast<uint32_t>(Vertices.size());
                    Vertices.push_back(vertex);
                }

                Indices.push_back(uniqueVertices[vertex]);
            }
        }

        VertexBuffer.LoadVertices(Vertices, physicalDevice, device, graphicsQueue, commandPool);
        IndexBuffer.LoadIndices(Indices, physicalDevice, device, graphicsQueue, commandPool);
    }

    /*void VulkanModel::AddInstanceData(glm::mat4 transform)
    {
        Instances.push_back(InstanceData{ .Transform = transform });
    }*/

    void VulkanModel::Dispose(VkDevice& device, VulkanUniformBuffer& uniformBuffer)
    {
        //uniformBuffer.RemoveTransform(TransformIndex);
        VertexBuffer.Dispose(device);
        IndexBuffer.Dispose(device);
        delete Texture;
    }

    uint8_t VulkanModel::GetFirstInstanceIndex()
    {
        return instancesEnd - instanceCount;
    }

}