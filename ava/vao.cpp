#include "vao.hpp"
#include "detail/vao.hpp"

#include "detail/detail.hpp"

namespace ava
{
    static uint32_t vertexAttributeByteWidth(const VertexAttribute& attribute)
    {
        return detail::vulkanFormatByteWidth(attribute.format);
    }

    static std::vector<uint32_t> getVertexAttributeBindings(const std::vector<VertexAttribute>& attributes)
    {
        std::vector<uint32_t> bindings;
        for (const auto& attribute : attributes)
        {
            const auto binding = attribute.binding;
            bool foundBinding = false;
            for (auto& a : attributes)
            {
                if (a.binding == binding)
                {
                    foundBinding = true;
                    break;
                }
            }
            if (!foundBinding)
            {
                bindings.push_back(binding);
            }
        }
        return bindings;
    }

    static std::vector<VertexAttribute> fixupVertexAttributes(const std::vector<VertexAttribute>& attributes, std::vector<uint32_t>* strides)
    {
        AVA_CHECK(!attributes.empty(), "Vertex attribute vector is empty, cannot create VAO");

        const auto bindings = getVertexAttributeBindings(attributes);
        const auto maxBinding = *std::ranges::max_element(bindings);

        std::vector<uint32_t> attributeIndices, attributeOffsets;
        attributeIndices.resize(maxBinding, 0u);
        attributeOffsets.resize(maxBinding, 0u);

        std::vector<VertexAttribute> result;
        for (const auto& attribute : attributes)
        {
            const auto binding = attribute.binding;
            AVA_CHECK(attribute.format != vk::Format::eUndefined, "Vertex attribute " + std::to_string(attributeIndices.at(binding)) + " has an invalid format");
            uint32_t location = attribute.location == VertexAttribute::AUTO_LOCATION ? attributeIndices[binding] : attribute.location;
            uint32_t offset = attribute.offset == VertexAttribute::AUTO_OFFSET ? attributeOffsets[binding] : attribute.offset;

            result.emplace_back(attribute.format, location, offset, binding);

            attributeIndices[binding]++;
            attributeOffsets[binding] += vertexAttributeByteWidth(attribute);
        }

        if (strides != nullptr)
        {
            *strides = attributeOffsets;
        }

        return result;
    }

    static VAO createVAOMain(const std::vector<VertexAttribute>& vertexAttributes, const std::vector<uint32_t>& strides, const vk::PrimitiveTopology topology, const bool primitiveRestartEnable)
    {
        auto bindings = getVertexAttributeBindings(vertexAttributes);

        AVA_CHECK(strides.size() == bindings.size(), "Vertex stride mismatches number of vertex bindings");

        std::vector<vk::VertexInputAttributeDescription> attributeDescriptions;
        for (const auto& attribute : vertexAttributes)
        {
            attributeDescriptions.emplace_back(attribute.location, attribute.binding, attribute.format, attribute.offset);
        }
        std::vector<vk::VertexInputBindingDescription> bindingDescriptions;
        uint32_t strideIndex = 0;
        for (auto& binding : bindings)
        {
            bindingDescriptions.emplace_back(binding, strides[strideIndex++], vk::VertexInputRate::eVertex);
        }

        auto outVAO = new detail::VAO;
        outVAO->attributeDescriptions = attributeDescriptions;
        outVAO->bindingDescriptions = bindingDescriptions;
        outVAO->topology = topology;
        outVAO->strides = strides;
        outVAO->primitiveRestartEnable = primitiveRestartEnable;
        return outVAO;
    }

    VAO createVAO(const std::vector<VertexAttribute>& vertexAttributes, const std::vector<uint32_t>& strides, vk::PrimitiveTopology topology, bool primitiveRestartEnable)
    {
        const auto attributes = fixupVertexAttributes(vertexAttributes, nullptr);

        return createVAOMain(attributes, strides, topology, primitiveRestartEnable);
    }

    VAO createVAO(const std::vector<VertexAttribute>& vertexAttributes, vk::PrimitiveTopology topology, bool primitiveRestartEnable)
    {
        std::vector<uint32_t> strides;
        const auto attributes = fixupVertexAttributes(vertexAttributes, &strides);

        return createVAOMain(attributes, strides, topology, primitiveRestartEnable);
    }
}
