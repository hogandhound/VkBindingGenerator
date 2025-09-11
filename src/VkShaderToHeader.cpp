#define RYML_SINGLE_HDR_DEFINE_NOW
#include "rapidyaml-0.5.0.hpp"
#include "vulkan/vulkan_core.h"
#include <unordered_map>

namespace Binding
{
    enum BindingEnum
    {
        FLOAT,
        VEC2,
        VEC3,
        VEC4,
        MAT2,
        MAT3,
        MAT4,
        INT,
        IVEC2,
        IVEC3,
        IVEC4,
        UINT,
        UVEC2,
        UVEC3,
        UVEC4,
        STRUCT
    };
}
std::string BindingToStride(Binding::BindingEnum e)
{
    switch (e)
    {
    case Binding::FLOAT:
        return "4";
    case Binding::VEC2:
        return "8";
    case Binding::VEC3:
        return "12";
    case Binding::VEC4:
        return "16";
    case Binding::MAT2:
        return "16";
    case Binding::MAT3:
        return "36";
    case Binding::MAT4:
        return "64";
    case Binding::INT:
        return "4";
    case Binding::IVEC2:
        return "8";
    case Binding::IVEC3:
        return "12";
    case Binding::IVEC4:
        return "16";
    case Binding::UINT:
        return "4";
    case Binding::UVEC2:
        return "8";
    case Binding::UVEC3:
        return "12";
    case Binding::UVEC4:
        return "16";
    default:
        return "16";
    }
}
int BindingToStrideI(Binding::BindingEnum e)
{
    switch (e)
    {
    case Binding::FLOAT:
        return 4;
    case Binding::VEC2:
        return 8;
    case Binding::VEC3:
        return 12;
    case Binding::VEC4:
        return 16;
    case Binding::MAT2:
        return 16;
    case Binding::MAT3:
        return 36;
    case Binding::MAT4:
        return 64;
    case Binding::INT:
        return 4;
    case Binding::IVEC2:
        return 8;
    case Binding::IVEC3:
        return 12;
    case Binding::IVEC4:
        return 16;
    case Binding::UINT:
        return 4;
    case Binding::UVEC2:
        return 8;
    case Binding::UVEC3:
        return 12;
    case Binding::UVEC4:
        return 16;
    default:
        return 16;
    }
}
struct BindingDef
{
    std::string name;
    int loc, binding;
    std::string offset, stride;
    Binding::BindingEnum type;
    std::string format, rate;
};
struct TextureDef
{
    std::string name;
    int binding, set;
};
struct UniformDef
{
    std::string name;
    int binding, set;
};
struct ShaderFragDef
{
    std::string name;
    std::vector<TextureDef> texs;
    std::vector<UniformDef> ubos;
    std::string push;
    std::string pushStages;
};
struct ShaderVertDef
{
    std::string name;
    std::vector<BindingDef> inputs;
    std::vector<TextureDef> texs;
    std::vector<UniformDef> ubos;
    std::string push;
    std::string pushStages;
};
struct StencilDef
{
    bool active;
    std::string compareMask,
        writeMask,
        reference,
        compareOp,
        failOp,
        depthFailOp,
        passOp;
};
struct BlendDef
{
    std::string
        blendEnable = "VK_TRUE",
        colorWriteMask = "VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT",
        srcColorBlendFactor = "VK_BLEND_FACTOR_SRC_ALPHA",
        dstColorBlendFactor = "VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA",
        colorBlendOp = "VK_BLEND_OP_ADD",
        srcAlphaBlendFactor = "VK_BLEND_FACTOR_SRC_ALPHA",
        dstAlphaBlendFactor = "VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA",
        alphaBlendOp = "VK_BLEND_OP_ADD";
};

struct DepthBias
{
    bool enabled;
    std::string depthBiasConstantFactor;
    std::string depthBiasClamp;
    std::string depthBiasSlopeFactor;
};

struct ShaderDef
{
    std::string name;
    ShaderVertDef vert;
    ShaderFragDef frag;
    std::string topo, depth, cullMode = "VK_CULL_MODE_NONE", frontFace = "VK_FRONT_FACE_COUNTER_CLOCKWISE";
    DepthBias depthBias = {};
    bool depthWrite;
    std::vector<std::string> dynamicStates;
    StencilDef stencil;
    BlendDef blend;
};
struct ShaderStructPart
{
    Binding::BindingEnum type;
    std::string structName;
    std::string name;
    int count;
};
struct ShaderStruct
{
    std::string name;
    std::vector<ShaderStructPart> parts;
    int totalStride;
};

template<class T>
struct StagesDef
{
    T def;
    std::string stages;
};

struct ShaderProcess
{
    std::string name;
    std::vector<std::string> includes;
    std::vector<ShaderDef> shaders;
    std::unordered_map<std::string, ShaderStruct> structs;
};

std::unordered_map<std::string, std::string> ParseStruct(ryml::Tree& doc, ShaderProcess& process)
{
    std::unordered_map<std::string, std::string> fileMapping;
    if (doc.has_child(doc.root_id(), "types"))
    {
        for (const auto& type : doc["types"].children())
        {
            std::string name, key;
            key = std::string(type.key().str, type.key().len);
            type["name"] >> name;
            if (name.compare("gl_PerVertex") == 0)
                continue;

            fileMapping[key] = name;
            if (process.structs.find(name) == process.structs.end())
            {
                ShaderStruct strct = {};
                strct.name = name;
                for (const auto& mem : type["members"])
                {
                    ShaderStructPart part = {};
                    mem["name"] >> part.name;
                    part.count = 1;
                    if (mem.has_child("array"))
                    {
                        auto arr = mem["array"];
                        for (int c = 0; c < arr.num_children(); ++c)
                        {
                            part.count *= atoi(arr[c].val().data());
                        }
                    }
                    if (mem["type"] == "float")
                    {
                        part.type = Binding::FLOAT;
                    }
                    else if (mem["type"] == "vec2")
                    {
                        part.type = Binding::VEC2;
                    }
                    else if (mem["type"] == "vec3")
                    {
                        part.type = Binding::VEC3;
                    }
                    else if (mem["type"] == "vec4")
                    {
                        part.type = Binding::VEC4;
                    }
                    else if (mem["type"] == "mat2")
                    {
                        part.type = Binding::MAT2;
                    }
                    else if (mem["type"] == "mat3")
                    {
                        part.type = Binding::MAT3;
                    }
                    else if (mem["type"] == "mat4")
                    {
                        part.type = Binding::MAT4;
                    }
                    else if (mem["type"] == "int")
                    {
                        part.type = Binding::INT;
                    }
                    else if (mem["type"] == "ivec2")
                    {
                        part.type = Binding::IVEC2;
                    }
                    else if (mem["type"] == "ivec3")
                    {
                        part.type = Binding::IVEC3;
                    }
                    else if (mem["type"] == "ivec4")
                    {
                        part.type = Binding::IVEC4;
                    }
                    else if (mem["type"] == "uint")
                    {
                        part.type = Binding::UINT;
                    }
                    else if (mem["type"] == "uvec2")
                    {
                        part.type = Binding::UVEC2;
                    }
                    else if (mem["type"] == "uvec3")
                    {
                        part.type = Binding::UVEC3;
                    }
                    else if (mem["type"] == "uvec4")
                    {
                        part.type = Binding::UVEC4;
                    }
                    else
                    {
                        part.type = Binding::STRUCT;
                        mem["type"] >> part.structName;
                        part.structName = fileMapping[part.structName];
                    }
                    if (part.type != Binding::STRUCT)
                        strct.totalStride += BindingToStrideI(part.type) * part.count;
                    else
                    {
                        strct.totalStride += process.structs[part.structName].totalStride* part.count;
                    }
                    strct.parts.push_back(part);
                }
                process.structs[name] = strct;
            }
        }
    }
    return fileMapping;
}
void ReadVertJson(std::string file, ShaderProcess& process, ShaderDef& shader)
{
    ShaderVertDef& vert = shader.vert;
    std::vector<char> fileBuf;

    FILE* f = 0;
    auto err = fopen_s(&f, (file + ".json").c_str(), "rb");
    fseek(f, 0, SEEK_END);
    size_t fLen = ftell(f);
    fseek(f, 0, SEEK_SET);
    fileBuf.resize(fLen);
    fread(fileBuf.data(), fLen, 1, f);
    fclose(f);

    auto doc = ryml::parse_in_arena(ryml::to_csubstr(fileBuf));
    std::unordered_map<std::string, std::string> structMap = ParseStruct(doc, process);
    for (const auto& yinput : doc["inputs"])
    {
        BindingDef input = {};

        yinput["name"] >> input.name;
        yinput["location"] >> input.loc;

        if (yinput["type"] == "float")
        {
            input.type = Binding::FLOAT;
            input.format = "VK_FORMAT_R32_SFLOAT";
        }
        else if (yinput["type"] == "vec2")
        {
            input.type = Binding::VEC2;
            input.format = "VK_FORMAT_R32G32_SFLOAT";
        }
        else if (yinput["type"] == "vec3")
        {
            input.type = Binding::VEC3;
            input.format = "VK_FORMAT_R32G32B32_SFLOAT";
        }
        else if (yinput["type"] == "vec4")
        {
            input.type = Binding::VEC4;
            input.format = "VK_FORMAT_R32G32B32A32_SFLOAT";
        }
        else if (yinput["type"] == "mat2")
        {
            input.type = Binding::MAT2;
            input.format = "VK_FORMAT_R32G32_SFLOAT";
        }
        else if (yinput["type"] == "mat3")
        {
            input.type = Binding::MAT3;
            input.format = "VK_FORMAT_R32G32B32_SFLOAT";
        }
        else if (yinput["type"] == "mat4")
        {
            input.type = Binding::MAT4;
            input.format = "VK_FORMAT_R32G32B32A32_SFLOAT";
        }
        else if (yinput["type"] == "int")
        {
            input.type = Binding::INT;
            input.format = "VK_FORMAT_R32_SINT";
        }
        else if (yinput["type"] == "ivec2")
        {
            input.type = Binding::IVEC2;
            input.format = "VK_FORMAT_R32G32_SINT";
        }
        else if (yinput["type"] == "ivec3")
        {
            input.type = Binding::IVEC3;
            input.format = "VK_FORMAT_R32G32B32_SINT";
        }
        else if (yinput["type"] == "ivec4")
        {
            input.type = Binding::IVEC4;
            input.format = "VK_FORMAT_R32G32B32A32_SINT";
        }
        else if (yinput["type"] == "uint")
        {
            input.type = Binding::UINT;
            input.format = "VK_FORMAT_R32_UINT";
        }
        else if (yinput["type"] == "uvec2")
        {
            input.type = Binding::UVEC2;
            input.format = "VK_FORMAT_R32G32_UINT";
        }
        else if (yinput["type"] == "uvec3")
        {
            input.type = Binding::UVEC3;
            input.format = "VK_FORMAT_R32G32B32_UINT";
        }
        else if (yinput["type"] == "uvec4")
        {
            input.type = Binding::UVEC4;
            input.format = "VK_FORMAT_R32G32B32A32_UINT";
        }

        vert.inputs.push_back(input);
    }
    if (doc.has_child(doc.root_id(), "ubos"))
    {
        for (const auto& ytex : doc["ubos"])
        {
            UniformDef input = {};

            ytex["name"] >> input.name;
            ytex["binding"] >> input.binding;
            ytex["set"] >> input.set;
            int i = 0;
            for (; i < vert.ubos.size(); ++i)
            {
                if (vert.ubos[i].binding > input.binding)
                {
                    vert.ubos.insert(vert.ubos.begin() + i, input);
                    break;
                }
            }
            if (i == vert.ubos.size())
                vert.ubos.push_back(input);
        }
    }
    if (doc.has_child(doc.root_id(), "textures"))
    {
        for (const auto& ytex : doc["textures"])
        {
            TextureDef input = {};

            ytex["name"] >> input.name;
            ytex["binding"] >> input.binding;
            ytex["set"] >> input.set;
            vert.texs.push_back(input);
            int i = 0;
            for (; i < vert.texs.size(); ++i)
            {
                if (vert.texs[i].binding > input.binding)
                {
                    vert.texs.insert(vert.texs.begin() + i, input);
                    break;
                }
            }
            if (i == vert.texs.size())
                vert.texs.push_back(input);
        }
    }
    if (doc.has_child(doc.root_id(), "push_constants"))
    {
        std::string id;
        doc["push_constants"][0]["type"] >> id;
        shader.vert.push = structMap[id];
        shader.vert.pushStages = "VK_SHADER_STAGE_VERTEX_BIT";
    }
}
void ReadFragJson(std::string file, ShaderProcess& process, ShaderDef& shader)
{
    ShaderFragDef& frag = shader.frag;
    std::vector<char> fileBuf;

    FILE* f = 0;
    auto err = fopen_s(&f, (file + ".json").c_str(), "rb");
    fseek(f, 0, SEEK_END);
    size_t fLen = ftell(f);
    fseek(f, 0, SEEK_SET);
    fileBuf.resize(fLen);
    fread(fileBuf.data(), fLen, 1, f);
    fclose(f);

    auto doc = ryml::parse_in_arena(ryml::to_csubstr(fileBuf));
    std::unordered_map<std::string, std::string> structMap = ParseStruct(doc, process);


    if (doc.has_child(doc.root_id(), "ubos"))
    {
        for (const auto& ytex : doc["ubos"])
        {
            UniformDef input = {};

            ytex["name"] >> input.name;
            ytex["binding"] >> input.binding;
            ytex["set"] >> input.set;
            frag.ubos.push_back(input);
        }
    }
    if (doc.has_child(doc.root_id(), "textures"))
    {
        for (const auto& ytex : doc["textures"])
        {
            TextureDef input = {};

            ytex["name"] >> input.name;
            ytex["binding"] >> input.binding;
            ytex["set"] >> input.set;
            frag.texs.push_back(input);
        }
    }
    if (doc.has_child(doc.root_id(), "push_constants"))
    {
        std::string id;
        doc["push_constants"][0]["type"] >> id;
        shader.frag.push = structMap[id];
        shader.frag.pushStages = "VK_SHADER_STAGE_FRAGMENT_BIT";
    }
}

template<class T>
std::vector<StagesDef<T>> BuildStages(std::vector<T>& vertex, std::vector<T>& frags)
{
    std::vector<StagesDef<T>> ret;
    for (auto& def : vertex)
    {
        StagesDef<T> add = {};
        add.def = def;
        add.stages = "VK_SHADER_STAGE_VERTEX_BIT";
        ret.push_back(add);
    }
    for (auto& def : frags)
    {
        bool added = false;
        for (auto& existing : ret)
        {
            if (def.name.compare(existing.def.name) == 0 && def.binding == existing.def.binding && def.set == existing.def.set)
            {
                existing.stages += "|VK_SHADER_STAGE_FRAGMENT_BIT";
                added = true;
                break;
            }
        }
        if (!added)
        {
            StagesDef<T> add = {};
            add.def = def;
            add.stages = "VK_SHADER_STAGE_FRAGMENT_BIT";
            ret.push_back(add);
        }
    }
    return ret;
}

#include "VkShader2HeaderConst.h"
std::string GetShaderArray(const std::string& super, std::string name)
{
    for (size_t charPos = name.find_first_of("./\\"); charPos != std::string::npos; charPos = name.find_first_of("./\\"))
    {
        name[charPos] = '_';
    }
    for (size_t charPos = name.find("__"); charPos != std::string::npos; charPos = name.find_first_of("__"))
    {
        name.erase(name.begin() + charPos);
    }
    return super + "_" + name;
}
std::string GetDrawFunctionName(ShaderProcess& process, ShaderDef& shader, std::vector<std::pair<int, int>>& bindingDescSets, bool drawIndexed, bool instanced)
{
    bool descSets = shader.frag.texs.size() + shader.frag.ubos.size() + shader.vert.texs.size() + shader.vert.ubos.size() > 0;
    std::string out = "void " + process.name + "_Draw" + shader.name + (drawIndexed ? "" : "_NI") + "(" + process.name + R"(_Pipeline_Collection& pipeline, VkCommandBuffer command)";
    if (descSets)
        out += ", std::vector<VkDescriptorSet>& sets";
    if (drawIndexed)
        out += ", VkBuffer indexBuffer, uint32_t indexCount, uint32_t indexOffset, VkIndexType indexType";
    else
        out += ", uint32_t vertexCount";
    if (instanced)
        out += ", uint32_t instanceCount";
    for (auto& inp : bindingDescSets)
    {
        out += ", VkBuffer " + shader.vert.inputs[inp.second].name + ", VkDeviceSize offset_" + shader.vert.inputs[inp.second].name;
    }
    if (!shader.vert.push.empty())
    {
        if (!shader.frag.push.empty() && shader.frag.push.compare(shader.vert.push))
        {
            out += ", " + shader.vert.push + "_" + shader.frag.push + "* push";
        }
        else
        {
            out += ", " + shader.vert.push + "* push";
        }
    }
    else if (!shader.frag.push.empty())
    {
        out += ", " + shader.frag.push + "* push";
    }
    out += std::string(")");
    return out;
}
std::string GetDescSetFunctionName(ShaderProcess& process, ShaderDef& shader,
    std::vector<StagesDef<UniformDef>>& ubos, std::vector<StagesDef<TextureDef>>& texs)
{
    std::string out = "void " + process.name + "_Update" + shader.name + "DescriptorSets(VkRenderTarget * target, "+process.name+"_Pipeline_Collection& pipeline, std::vector<VkDescriptorSet>& output";
    for (auto& def : texs)
    {
        out += ", VK::Texture* texture_" + def.def.name;
    }
    for (auto& def : ubos)
    {
        out += ", VK::Buffer ubo_" + def.def.name;
    }
    out += ")";
    return out;
}
std::vector<std::pair<int,int>> GetVertBindings(ShaderDef& shader)
{
    std::vector<std::pair<int, int>> bindingDescIndexes;
    for (int i = 0; i < shader.vert.inputs.size(); ++i)
    {
        int j;
        for (j = 0; j < bindingDescIndexes.size(); ++j)
        {
            if (bindingDescIndexes[j].first == shader.vert.inputs[i].binding)
                break;
        }
        if (j == bindingDescIndexes.size())
        {
            std::pair<int, int> pair;
            pair.first = shader.vert.inputs[i].binding;
            pair.second = i;
            bindingDescIndexes.push_back(pair);
        }
    }
    return bindingDescIndexes;
}
void OutputShaderImpl(ShaderProcess& process, std::string baseFolder)
{
    std::string out = R"(//THIS FILE WAS AUTO-GENERATED BY VKSHADERTOHEADER
#include ")" + process.name + R"(_shaderdef.h"
#include "VkRenderTarget.h"
#include <stdexcept>
)";
    for (auto& p : process.includes)
    {
        out += "#include \"" + p + "\"\n";
    }

    out += createShaderModule;

    for (auto& shader : process.shaders)
    {
        std::string vert = GetShaderArray(process.name, shader.vert.name);
        std::string frag = GetShaderArray(process.name, shader.frag.name);

        //CREATE SHADER
        out += "void " + process.name + "_Create" + shader.name + "Pipeline(VkRenderTarget* target, VKPipelineData& pipeline) {\n" +
            "    VkShaderModule vertShaderModule = createShaderModule(target->device, " + vert + ", sizeof(" + vert + "));" +
            "    VkShaderModule fragShaderModule = createShaderModule(target->device, " + frag + ", sizeof(" + frag + "));";
        out += vert_frag_1;

        std::vector<std::pair<int, int>> bindingDescIndexes = GetVertBindings(shader);
        out += "    VkVertexInputBindingDescription bindingDescription[" + std::to_string(bindingDescIndexes.size()) + "] = {};\n";
        for (int i = 0; i < bindingDescIndexes.size(); ++i)
        {
            std::string indexStr = "[" + std::to_string(i) + "]";
            out += "    {\n";
            out += "        bindingDescription" + indexStr + ".binding = " + std::to_string(bindingDescIndexes[i].first) + ";\n";
            out += "        bindingDescription" + indexStr + ".stride = " + shader.vert.inputs[bindingDescIndexes[i].second].stride + ";\n";
            out += "        bindingDescription" + indexStr + ".inputRate = " + shader.vert.inputs[bindingDescIndexes[i].second].rate + ";\n";
            out += "    }\n";
        }
        out += "    VkVertexInputAttributeDescription attributeDescriptions[" + std::to_string(shader.vert.inputs.size()) + "] = {};\n";
        for (int i = 0; i < shader.vert.inputs.size(); ++i)
        {
            std::string indexStr = "[" + std::to_string(i) + "]";
            out += "    {\n";
            out += "        attributeDescriptions" + indexStr + ".binding = " + std::to_string(shader.vert.inputs[i].binding) + ";\n";
            out += "        attributeDescriptions" + indexStr + ".location = " + std::to_string(shader.vert.inputs[i].loc) + ";\n";
            out += "        attributeDescriptions" + indexStr + ".format = " + shader.vert.inputs[i].format + ";\n";
            out += "        attributeDescriptions" + indexStr + ".offset = " + shader.vert.inputs[i].offset + ";\n";
            out += "    }\n";
        }

        out += "    vertexInputInfo.vertexBindingDescriptionCount = " + std::to_string(bindingDescIndexes.size()) + ";\n";
        out += "    vertexInputInfo.vertexAttributeDescriptionCount = " + std::to_string(shader.vert.inputs.size()) + ";\n";
        out += R"z(
    vertexInputInfo.pVertexBindingDescriptions = bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = )z" + shader.topo + R"z(;
    inputAssembly.primitiveRestartEnable = )z" + 
            ((shader.topo.compare("VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST") == 0 || 
                (shader.topo.compare("VK_PRIMITIVE_TOPOLOGY_POINT_LIST") == 0))
                ? "VK_FALSE" : "VK_TRUE") 
            + ";";
        //Create Desc Layout
        std::vector<StagesDef<TextureDef>> texs = BuildStages(shader.vert.texs, shader.frag.texs);
        std::vector<StagesDef<UniformDef>> ubos = BuildStages(shader.vert.ubos, shader.frag.ubos);
        std::string dstype = "";
        for (auto& def : texs)
            dstype += "T";
        for (auto& def : ubos)
            dstype += "U";
        out += shader_mid1;
        out += "VkDynamicState dynamicState[" + std::to_string(2 + shader.dynamicStates.size()) + "] = { VK_DYNAMIC_STATE_VIEWPORT , VK_DYNAMIC_STATE_SCISSOR";
        for (auto& dyn : shader.dynamicStates)
        {
            out += ", " + dyn;
        }
        out += shader_mid2;

        out += "    rasterizer.frontFace = " + shader.frontFace + ";\n";
        out += "    rasterizer.cullMode = " + shader.cullMode + ";\n";
        if (shader.depthBias.enabled)
        {
            /*
    VkBool32                                   depthBiasEnable;
    float                                      depthBiasConstantFactor;
    float                                      depthBiasClamp;
    float                                      depthBiasSlopeFactor;
            */
            out += "    rasterizer.depthBiasEnable = VK_TRUE;\n";
            if (!shader.depthBias.depthBiasClamp.empty()) out += "    rasterizer.depthBiasClamp = " + shader.depthBias.depthBiasClamp + ";\n";
            if (!shader.depthBias.depthBiasConstantFactor.empty()) out += "    rasterizer.depthBiasConstantFactor = " + shader.depthBias.depthBiasConstantFactor + ";\n";
            if (!shader.depthBias.depthBiasSlopeFactor.empty()) out += "    rasterizer.depthBiasSlopeFactor = " + shader.depthBias.depthBiasSlopeFactor + ";\n";
        }

        out += "    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};\n";
        out += "    colorBlendAttachment.blendEnable = " + shader.blend.blendEnable + ";\n";
        out += "    colorBlendAttachment.colorWriteMask = " + shader.blend.colorWriteMask + ";\n";
        out += "    colorBlendAttachment.srcColorBlendFactor = " + shader.blend.srcColorBlendFactor + ";\n";
        out += "    colorBlendAttachment.dstColorBlendFactor = " + shader.blend.dstColorBlendFactor + ";\n";
        out += "    colorBlendAttachment.colorBlendOp = " + shader.blend.colorBlendOp + ";\n";
        out += "    colorBlendAttachment.srcAlphaBlendFactor = " + shader.blend.srcAlphaBlendFactor + ";\n";
        out += "    colorBlendAttachment.dstAlphaBlendFactor = " + shader.blend.dstAlphaBlendFactor + ";\n";
        out += "    colorBlendAttachment.alphaBlendOp = " + shader.blend.alphaBlendOp + ";\n";

        out += shader_mid3;
        if (texs.size() + ubos.size() > 0)
        {
            out += R"(

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &pipeline.descriptorSetLayout;)";
        }
        else
        {
            out += R"(

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;)";
        }
        if (shader.vert.push.empty() == false && shader.frag.push.empty() == false)
        {
            if (shader.vert.push.compare(shader.frag.push) == 0)
            {
                out += R"(
    VkPushConstantRange pushConstantRange;
    pushConstantRange.stageFlags = )" + shader.vert.pushStages + "|" + shader.frag.pushStages + R"(;
    pushConstantRange.size = sizeof()" + shader.vert.push + R"();
    pushConstantRange.offset = 0;

    // Push constant ranges are part of the pipeline layout
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;)";
            }
            else
            {
                out += R"(
    VkPushConstantRange pushConstantRange[2];
    pushConstantRange[0].stageFlags = )" + shader.vert.pushStages + R"(;
    pushConstantRange[0].size = sizeof()" + shader.vert.push + R"();
    pushConstantRange[0].offset = 0;

    pushConstantRange[1].stageFlags = )" + shader.frag.pushStages + R"(;
    pushConstantRange[1].size = sizeof()" + shader.frag.push + R"();
    pushConstantRange[1].offset = sizeof()" + shader.vert.push + R"();

    // Push constant ranges are part of the pipeline layout
    pipelineLayoutInfo.pushConstantRangeCount = 2;
    pipelineLayoutInfo.pPushConstantRanges = pushConstantRange;)";
            }
        }
        else if (shader.vert.push.empty() == false)
        {
            out += R"(
    VkPushConstantRange pushConstantRange;
    pushConstantRange.stageFlags = )" + shader.vert.pushStages + R"(;
    pushConstantRange.size = sizeof()" + shader.vert.push + R"();
    pushConstantRange.offset = 0;

    // Push constant ranges are part of the pipeline layout
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;)";
        }
        else if (shader.frag.push.empty() == false)
        {
            out += R"(
    VkPushConstantRange pushConstantRange;
    pushConstantRange.stageFlags = )" + shader.frag.pushStages + R"(;
    pushConstantRange.size = sizeof()" + shader.frag.push + R"();
    pushConstantRange.offset = 0;

    // Push constant ranges are part of the pipeline layout
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;)";
        }
        out += R"(
    if (vkCreatePipelineLayout(target->device, &pipelineLayoutInfo, nullptr, &pipeline.pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo = {};)";
        if (shader.depth.empty() == false)
        {

            out += R"(
    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = )" + std::string(shader.depthWrite ? "VK_TRUE" : "VK_FALSE") + R"(;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f; // Optional
    depthStencil.maxDepthBounds = 1.0f; // Optional
    depthStencil.depthCompareOp = )" + shader.depth + R"(;
    pipelineInfo.pDepthStencilState = &depthStencil;
)";
        }
        else
        {
            out += R"(
    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_FALSE;
    depthStencil.depthWriteEnable = VK_FALSE;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f; // Optional
    depthStencil.maxDepthBounds = 1.0f; // Optional
    depthStencil.depthCompareOp = VK_COMPARE_OP_NEVER;
    pipelineInfo.pDepthStencilState = &depthStencil;
)";
        }
        if (shader.stencil.active)
        {
            out += "    depthStencil.stencilTestEnable = VK_TRUE;\n";
            out += "    depthStencil.front.compareMask = " + shader.stencil.compareMask + ";\n";
            out += "    depthStencil.front.writeMask = " + shader.stencil.writeMask + ";\n";
            out += "    depthStencil.front.reference = " + shader.stencil.reference + ";\n";
            out += "    depthStencil.front.compareOp = " + shader.stencil.compareOp + ";\n";
            out += "    depthStencil.front.failOp = " + shader.stencil.failOp + ";\n";
            out += "    depthStencil.front.depthFailOp = " + shader.stencil.depthFailOp + ";\n";
            out += "    depthStencil.front.passOp = " + shader.stencil.passOp + ";\n";
            out += "    depthStencil.back = depthStencil.front;\n";
        }
        out += shader_end;



        if (texs.size() + ubos.size() > 0)
        {
            out += "\n\nvoid " + process.name + "_Create" + shader.name + "DescriptorSetLayout(VkRenderTarget * target, VKPipelineData& pipeline) {\n" +
                "    VkDescriptorSetLayoutBinding bindings[" +
                std::to_string(texs.size() + ubos.size())
                + "] = {};\n";
            for (int i = 0; i < texs.size(); ++i)
            {
                std::string indexStr = "[" + std::to_string(i) + "]";
                out += "    {\n";
                out += "        bindings" + indexStr + ".binding = " + std::to_string(texs[i].def.binding) + ";\n";
                out += "        bindings" + indexStr + ".descriptorCount = 1;\n";
                out += "        bindings" + indexStr + ".descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;\n";
                out += "        bindings" + indexStr + ".pImmutableSamplers = nullptr;\n";
                out += "        bindings" + indexStr + ".stageFlags = " + texs[i].stages + ";\n";
                out += "    }\n";
            }
            for (int i = 0; i < ubos.size(); ++i)
            {
                std::string indexStr = "[" + std::to_string(texs.size() + i) + "]";
                out += "    {\n";
                out += "        bindings" + indexStr + ".binding = " + std::to_string(ubos[i].def.binding) + ";\n";
                out += "        bindings" + indexStr + ".descriptorCount = 1;\n";
                out += "        bindings" + indexStr + ".descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;\n";
                out += "        bindings" + indexStr + ".pImmutableSamplers = nullptr;\n";
                out += "        bindings" + indexStr + ".stageFlags = " + ubos[i].stages + ";\n";
                out += "    }\n";
            }
            out += R"(
    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = )" + std::to_string(texs.size() + ubos.size()) + R"(;
    layoutInfo.pBindings = bindings;

    pipeline.subIndex = target->getDescSetSubIndex(VkDS_)" + dstype + R"(, bindings, )" + std::to_string(texs.size() + ubos.size()) + R"();
    if (vkCreateDescriptorSetLayout(target->device, &layoutInfo, nullptr, &pipeline.descriptorSetLayout) != VK_SUCCESS) {}
}
)";
        }

        //Update Desc Set
/*
void PipelineImageDraw::UpdateDescriptorSets(VkRenderTarget* target, VK::Texture* texture) {

    VkDescriptorSet descriptorSet;
    if (++currSet >= descriptorSets.size())
    {
        currSet = 0;
    }
    descriptorSet = descriptorSets[currSet];
    if (!descriptorSet)
    {
        VkDescriptorSetAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &descriptorSetLayout;

        if (vkAllocateDescriptorSets(target->device, &allocInfo, &descriptorSet) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }
        descriptorSets[currSet] = descriptorSet;
    }

    VkDescriptorImageInfo imageInfo = {};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = texture->imageView;
    imageInfo.sampler = texture->sampler;

    VkWriteDescriptorSet descriptorWrites = {};

    descriptorWrites.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites.dstSet = descriptorSet;
    descriptorWrites.dstBinding = 0;
    descriptorWrites.dstArrayElement = 0;
    descriptorWrites.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites.descriptorCount = 1;
    descriptorWrites.pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(target->device, 1, &descriptorWrites, 0, nullptr);
}
*/
        if (texs.size() + ubos.size() > 0)
        {
            std::string pipeline = "pipeline.pipelines[PIPELINE_" + process.name + "_" + shader.name + "]";
            out += GetDescSetFunctionName(process, shader, ubos, texs);
            out += " {\n";

            out += "    VkDescriptorSet descriptorSet = target->getDescSet(VkDS_" + dstype + ", " + pipeline + ".subIndex, &" + pipeline +
                ".descriptorSetLayout);\n";
            out += "    VkWriteDescriptorSet descriptorWrites[" + std::to_string(texs.size() + ubos.size()) + "] = {};\n\n";
            for (int i = 0; i < texs.size(); ++i)
            {
                std::string indexStr = "[" + std::to_string(i) + "]";
                out += "    VkDescriptorImageInfo imageInfo_" + texs[i].def.name + " = {};\n";
                out += "    imageInfo_" + texs[i].def.name + ".imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;\n";
                out += "    imageInfo_" + texs[i].def.name + ".imageView = texture_" + texs[i].def.name + "->imageView;\n";
                out += "    imageInfo_" + texs[i].def.name + ".sampler = texture_" + texs[i].def.name + "->sampler;\n";
                out += "    \n";
                out += "    descriptorWrites" + indexStr + ".sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;\n";
                out += "    descriptorWrites" + indexStr + ".dstSet = descriptorSet;\n";
                out += "    descriptorWrites" + indexStr + ".dstBinding = " + std::to_string(texs[i].def.binding) + ";\n";
                out += "    descriptorWrites" + indexStr + ".dstArrayElement = 0;\n";
                out += "    descriptorWrites" + indexStr + ".descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;\n";
                out += "    descriptorWrites" + indexStr + ".descriptorCount = 1;\n";
                out += "    descriptorWrites" + indexStr + ".pImageInfo = &imageInfo_" + texs[i].def.name + ";\n";
            }
            for (int i = 0; i < ubos.size(); ++i)
            {
                std::string indexStr = "[" + std::to_string(texs.size() + i) + "]";
                out += "    VkDescriptorBufferInfo bufferInfo_" + ubos[i].def.name + " = {};\n";
                out += "    bufferInfo_" + ubos[i].def.name + ".buffer = ubo_" + ubos[i].def.name + ".buffer;\n";
                out += "    bufferInfo_" + ubos[i].def.name + ".offset = 0;\n";
                out += "    bufferInfo_" + ubos[i].def.name + ".range = sizeof(" + ubos[i].def.name + ");\n";
                out += "    \n";
                out += "    descriptorWrites" + indexStr + ".sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;\n";
                out += "    descriptorWrites" + indexStr + ".dstSet = descriptorSet;\n";
                out += "    descriptorWrites" + indexStr + ".dstBinding = " + std::to_string(ubos[i].def.binding) + ";\n";
                out += "    descriptorWrites" + indexStr + ".dstArrayElement = 0;\n";
                out += "    descriptorWrites" + indexStr + ".descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;\n";
                out += "    descriptorWrites" + indexStr + ".descriptorCount = 1;\n";
                out += "    descriptorWrites" + indexStr + ".pBufferInfo = &bufferInfo_" + ubos[i].def.name + ";\n";
            }
            out += "    vkUpdateDescriptorSets(target->device, " + std::to_string(texs.size() + ubos.size()) + ", descriptorWrites, 0, nullptr);\n";
            out += "    output.clear();\n";
            out += "    output.push_back(descriptorSet);\n";
            out += "}\n";
        }
    }
    out += "void " + process.name + "_PopulatePipeline(VkRenderTarget* target, " + process.name + "_Pipeline_Collection& col)\n"
        "{\n";
    for (auto& p : process.shaders)
    {
        //void vktest_PopulatePipeline(VkRenderTarget* target, vktest_Pipeline_Collection& col)
        //{
        //  TLVK_CreateTexture2DDescriptorSetLayout(target, col.pipelines[PIPELINE_TLVK_Texture2D]);
        //	vktest_CreateTexturePipeline(target, col.pipelines[PIPELINE_vktest_Texture]);
        //}
        bool descSets = p.frag.texs.size() + p.frag.ubos.size() + p.vert.texs.size() + p.vert.ubos.size() > 0;
        if (descSets)
            out += "    " + process.name + "_Create" + p.name + "DescriptorSetLayout(target, col.pipelines[PIPELINE_" + process.name + "_" + p.name + "]);\n";
        out += "    " + process.name + "_Create" + p.name + "Pipeline(target, col.pipelines[PIPELINE_" + process.name + "_" + p.name + "]);\n";
    }
    out += "}\n";

    for (auto& shader : process.shaders)
    {
        bool descSets = shader.frag.texs.size() + shader.frag.ubos.size() + shader.vert.texs.size() + shader.vert.ubos.size() > 0;
        auto bindingDescIndexes = GetVertBindings(shader);
        bool instanced = false;
        for (auto bdi : bindingDescIndexes)
        {
            if (shader.vert.inputs[bdi.second].rate.compare("VK_VERTEX_INPUT_RATE_INSTANCE") == 0)
            {
                instanced = true;
                break;
            }
        }
        std::string pipeline = "pipeline.pipelines[PIPELINE_" + process.name + +"_" + shader.name + "]";
        //Draw Command
        for (int var = 0; var < 2; var++)
        {
            out += "\n\n";
            out += GetDrawFunctionName(process, shader, bindingDescIndexes, var == 1, instanced);
            out += R"(
{
    vkCmdBindPipeline(command, VK_PIPELINE_BIND_POINT_GRAPHICS, )" + pipeline +
                R"(.graphicsPipeline);

    VkBuffer vertexBuffers[] = { )";
            for (int bi = 0; bi < bindingDescIndexes.size(); ++bi)
            for (auto& inp : bindingDescIndexes)
            {
                if (bi == inp.first)
                {
                    out += shader.vert.inputs[inp.second].name + ", ";
                    break;
                }
            }
            out += R"( };
    VkDeviceSize offsets[] = { )";
            for (int bi = 0; bi < bindingDescIndexes.size(); ++bi)
            for (auto& inp : bindingDescIndexes)
            {
                if (bi == inp.first)
                {
                    out += "offset_" + shader.vert.inputs[inp.second].name + ", ";
                    break;
                }
            }
            out += R"( };
)";
            if (!shader.vert.push.empty() && !shader.frag.push.empty())
            {
                if (shader.vert.push.compare(shader.frag.push) == 0)
                {
                    out += R"(    vkCmdPushConstants(
        command,
        )" + pipeline + R"(.pipelineLayout,
        )" + shader.vert.pushStages + "|" + shader.frag.pushStages + R"(,
        0,
        sizeof()" + shader.vert.push + ")" + R"(,
        push);
)";
                }
                else
                {
                    out += R"(    vkCmdPushConstants(
        command,
        )" + pipeline + R"(.pipelineLayout,
        )" + shader.vert.pushStages + R"(,
        0,
        sizeof()" + shader.vert.push + ")" + R"(,
        push);
)";
                    out += R"(    vkCmdPushConstants(
        command,
        )" + pipeline + R"(.pipelineLayout,
        )" + shader.frag.pushStages + R"(,
        offsetof()" + shader.vert.push +"_"+shader.frag.push + R"(, frag),
        sizeof()" + shader.frag.push + ")" + R"(,
        &push->frag);
)";
                }
            }
            else if (!shader.vert.push.empty())
            {
                out += R"(    vkCmdPushConstants(
        command,
        )" + pipeline + R"(.pipelineLayout,
        )" + shader.vert.pushStages + R"(,
        0,
        sizeof()" + shader.vert.push + ")" + R"(,
        push);
)";
            }
            else if (!shader.frag.push.empty())
            {
                out += R"(    vkCmdPushConstants(
        command,
        )" + pipeline + R"(.pipelineLayout,
        )" + shader.frag.pushStages + R"(,
        0,
        sizeof()" + shader.frag.push + ")" + R"(,
        push);
)";
            }
            out += R"(
    vkCmdBindVertexBuffers(command, 0, )" + std::to_string(bindingDescIndexes.size()) + R"(, vertexBuffers, offsets);

)";
            if (var == 1)
                out += "    vkCmdBindIndexBuffer(command, indexBuffer, 0, indexType);\n\n";
            if (descSets)
                out += R"(    vkCmdBindDescriptorSets(command, VK_PIPELINE_BIND_POINT_GRAPHICS, 
        )" + pipeline + R"(.pipelineLayout, 0, )" +
                    std::string("static_cast<uint32_t>(sets.size()), sets.data()") +
                    ", 0, nullptr);\n\n";
            if (instanced)
            {
                out += std::string(var == 0 ? "    vkCmdDraw(command, vertexCount, instanceCount, 0, 0); " :
                    "    vkCmdDrawIndexed(command, indexCount, instanceCount, indexOffset, 0, 0);")
                    + "\n}";
            }
            else
            {
                out += std::string(var == 0 ? "    vkCmdDraw(command, vertexCount, 1, 0, 0); " :
                    "    vkCmdDrawIndexed(command, indexCount, 1, indexOffset, 0, 0);")
                    + "\n}";
            }
        }
    }
    //OutputDebugStringA(out.c_str());
    FILE* f = 0;
    std::string filename = baseFolder + process.name + "_shaderdef.cpp";
    fopen_s(&f, filename.c_str(), "wb");
    fwrite(out.c_str(), 1, out.size(), f);
    fclose(f);
}

void HandleUBOOffset(std::string& output, int& currentOffset, int newOffset, int& dummyCount)
{
    if (currentOffset + newOffset <= 4)
    {
        currentOffset += newOffset;
        if (currentOffset == 4)
            currentOffset = 0;
        return;
    }
    int dummySize = 4 - currentOffset;
    output += "    float dummy" + std::to_string(dummyCount++) + "["+ std::to_string(dummySize) +"];\n";
    currentOffset = newOffset & 0x3;
    if (currentOffset == 4)
        currentOffset = 0;
}

void AddStructToOutput(std::pair<const std::string, ShaderStruct>& s, std::string& output)
{
    output += "struct " + s.second.name + " {\n";
    int currOffset = 0, dummyCount = 0;
    for (auto& p : s.second.parts)
    {
        std::string arr = p.count > 1 ? "[" + std::to_string(p.count) + "]" : "";
        switch (p.type)
        {
        case Binding::FLOAT:
            HandleUBOOffset(output, currOffset, 1 * p.count, dummyCount); output += "    float " + p.name + arr + "[1];"; break;
        case Binding::VEC2:
            HandleUBOOffset(output, currOffset, 2 * p.count, dummyCount); output += "    float " + p.name + arr + "[2];"; break;
        case Binding::VEC3:
            HandleUBOOffset(output, currOffset, 3 * p.count, dummyCount); output += "    float " + p.name + arr + "[3];"; break;
        case Binding::VEC4:
            HandleUBOOffset(output, currOffset, 4, dummyCount); output += "    float " + p.name + arr + "[4];"; break;
        case Binding::MAT2:
            HandleUBOOffset(output, currOffset, 4, dummyCount); output += "    float " + p.name + arr + "[4];"; break;
        case Binding::MAT3:
            HandleUBOOffset(output, currOffset, 4, dummyCount); output += "    float " + p.name + arr + "[12];"; break;
        case Binding::MAT4:
            HandleUBOOffset(output, currOffset, 4, dummyCount); output += "    float " + p.name + arr + "[16];"; break;
        case Binding::INT:
            HandleUBOOffset(output, currOffset, 1 * p.count, dummyCount); output += "    int " + p.name + arr + "[1];"; break;
        case Binding::IVEC2:
            HandleUBOOffset(output, currOffset, 2 * p.count, dummyCount); output += "    int " + p.name + arr + "[2];"; break;
        case Binding::IVEC3:
            HandleUBOOffset(output, currOffset, 3 * p.count, dummyCount); output += "    int " + p.name + arr + "[3];"; break;
        case Binding::IVEC4:
            HandleUBOOffset(output, currOffset, 4, dummyCount); output += "    int " + p.name + arr + "[4];"; break;
        case Binding::UINT:
            HandleUBOOffset(output, currOffset, 1 * p.count, dummyCount); output += "    uint32_t " + p.name + arr + "[1];"; break;
        case Binding::UVEC2:
            HandleUBOOffset(output, currOffset, 2 * p.count, dummyCount); output += "    uint32_t " + p.name + arr + "[2];"; break;
        case Binding::UVEC3:
            HandleUBOOffset(output, currOffset, 3 * p.count, dummyCount); output += "    uint32_t " + p.name + arr + "[3];"; break;
        case Binding::UVEC4:
            HandleUBOOffset(output, currOffset, 4, dummyCount); output += "    uint32_t " + p.name + arr + "[4];"; break;
        case Binding::STRUCT:
        {
            HandleUBOOffset(output, currOffset, 4, dummyCount); output += "    " + p.structName + " " + p.name + arr + ";"; break;
            break;
        }
        }
        output += "\n";
    }
    HandleUBOOffset(output, currOffset, 4, dummyCount);
    output += "};\n";
}

void HandleStructs(ShaderProcess& process, std::string& output)
{
    std::unordered_map<std::string, int> addedStructs;
    std::vector<std::string> toAdd;
    for (auto& s : process.structs)
    {
        if (s.second.parts.size() == 0)
            continue;

        bool delay = false;
        for (auto sp : s.second.parts)
        {
            if (sp.structName.empty())
                continue;
            if (!addedStructs[sp.structName])
            {
                delay = true;
                break;
            }
        }
        if (delay)
        {
            toAdd.push_back(s.first);
            continue;
        }

        addedStructs[s.first] = 1;
        AddStructToOutput(s, output);
    }

    int lastSize = 0;
    //lastSize check is in case we have an unknown sturct. The code should continue.
    while (!toAdd.empty() && lastSize != toAdd.size())
    {
        lastSize = toAdd.size();
        
        for (int i = toAdd.size() - 1; i >= 0; --i)
        {
            auto& str = toAdd[i];
            auto& s = *process.structs.find(str);

            bool delay = false;
            for (auto sp : s.second.parts)
            {
                if (sp.structName.empty())
                    continue;
                if (!addedStructs[sp.structName])
                {
                    delay = true;
                    break;
                }
            }
            if (delay)
                continue;

            addedStructs[s.first] = 1;
            AddStructToOutput(s, output);
            toAdd.erase(toAdd.begin() + i);
        }
    }
    //Fallback to just adding the structs we couldn't figure out
    for (auto& str : toAdd)
    {
        auto& s = *process.structs.find(str);
        AddStructToOutput(s, output);
    }
}

void OutputShaderHeader(ShaderProcess& process, std::string baseFolder)
{
    //std::string baseFolder = "shaders\\";
    std::unordered_map<std::string, bool> dumped;
    std::string output;

    output += "//THIS FILE WAS AUTO-GENERATED BY VKSHADERTOHEADER\n";
    output += "#pragma once\n";
    output += R"(#include <vector>
#include "VkStructs.h"
class VkRenderTarget;
namespace VK { struct Texture; struct Buffer; })";
    output += "\n";

    output += "enum " + process.name + "_Pipeline_Entry {\n";
    for (auto& p : process.shaders)
    {
        output += "    PIPELINE_" + process.name + "_" + p.name + ",\n";
    }
    output += "    PIPELINE_" + process.name + "_MAX\n";
    output += "};\n";
    output += "struct " + process.name + "_Pipeline_Collection {\n"
        "    VKPipelineData pipelines[PIPELINE_" + process.name + "_MAX];\n"
        "};\n";

    HandleStructs(process, output);
    for (auto& p : process.shaders)
    {
        if (!p.vert.push.empty() && !p.frag.push.empty() && p.vert.push.compare(p.frag.push) != 0)
        {
            std::string comb = p.vert.push +"_"+p.frag.push;
            if (process.structs.find(comb) == process.structs.end())
            {
                process.structs[comb] = {};
                output += "struct " + p.vert.push + "_" + p.frag.push + " { ";
                output += p.vert.push + " vert; " + p.frag.push + " frag; };\n";
            }
        }
    }

    for (auto& p : process.shaders)
    {
        auto bindingIndexes = GetVertBindings(p);
        bool instanced = false;
        for (auto bdi : bindingIndexes)
        {
            if (p.vert.inputs[bdi.second].rate.compare("VK_VERTEX_INPUT_RATE_INSTANCE") == 0)
            {
                instanced = true;
                break;
            }
        }
        for (int var = 0; var < 2; ++var)
            output += GetDrawFunctionName(process, p, bindingIndexes, var == 0, instanced) + ";\n";
        std::vector<StagesDef<TextureDef>> texs = BuildStages(p.vert.texs, p.frag.texs);
        std::vector<StagesDef<UniformDef>> ubos = BuildStages(p.vert.ubos, p.frag.ubos);
        if (texs.size() + ubos.size() > 0)
            output += GetDescSetFunctionName(process, p, ubos, texs) + ";\n";
    }
    output += "void " + process.name + "_PopulatePipeline(VkRenderTarget* target, " + process.name + "_Pipeline_Collection& col);\n";

    for (auto& p : process.shaders)
    {
        std::string name;
        name = p.frag.name;
        if (!dumped[name])
        {
            dumped[name] = true;
            FILE* f = 0;
            auto err = fopen_s(&f, (baseFolder + "/" + name + ".spv").c_str(), "rb");
            fseek(f, 0, SEEK_END);
            size_t fLen = ftell(f);
            fseek(f, 0, SEEK_SET);
            std::vector<unsigned char> fileBuf;
            fileBuf.resize(fLen);
            fread(fileBuf.data(), fLen, 1, f);
            fclose(f);
            for (size_t charPos = name.find_first_of("./\\"); charPos != std::string::npos; charPos = name.find_first_of("./\\"))
            {
                name[charPos] = '_';
            }
            for (size_t charPos = name.find("__"); charPos != std::string::npos; charPos = name.find_first_of("__"))
            {
                name.erase(name.begin() + charPos);
            }
            output += "const unsigned char " + process.name + "_" + name + "[] = {\n";
            for (int i = 0; i < fileBuf.size(); ++i)
            {
                char tmp[16];
                sprintf_s(tmp, "0x%x,", fileBuf[i]);
                output += tmp;
                if ((i & 0xF) == 0xF)
                    output += '\n';
            }
            output += "\n};\n";
        }

        name = p.vert.name;
        if (!dumped[name])
        {
            dumped[name] = true;
            FILE* f = 0;
            auto err = fopen_s(&f, (baseFolder + "/" + name + ".spv").c_str(), "rb");
            fseek(f, 0, SEEK_END);
            size_t fLen = ftell(f);
            fseek(f, 0, SEEK_SET);
            std::vector<unsigned char> fileBuf;
            fileBuf.resize(fLen);
            fread(fileBuf.data(), fLen, 1, f);
            fclose(f);
            for (size_t charPos = name.find_first_of("./\\"); charPos != std::string::npos; charPos = name.find_first_of("./\\"))
            {
                name[charPos] = '_';
            }
            for (size_t charPos = name.find("__"); charPos != std::string::npos; charPos = name.find_first_of("__"))
            {
                name.erase(name.begin() + charPos);
            }
            output += "const unsigned char " + process.name + "_" + name + "[] = {\n";
            for (int i = 0; i < fileBuf.size(); ++i)
            {
                char tmp[10];
                sprintf_s(tmp, "0x%x,", fileBuf[i]);
                output += tmp;
                if ((i & 0xF) == 0xF)
                    output += '\n';
            }
            output += "\n};\n";
        }
    }
    FILE* f = 0;
    std::string filename = baseFolder + process.name + "_shaderdef.h";
    fopen_s(&f, filename.c_str(), "wb");
    fwrite(output.c_str(), 1, output.size(), f);
    fclose(f);
}

void Shader2Header(std::string baseFolder)
{
    //std::string baseFolder = "shaders\\";
    if (!baseFolder.empty() && baseFolder.back() != '\\' && baseFolder.back() != '/')
        baseFolder += "/";
    std::string path = baseFolder + "\\compileinfo.json";

    ShaderProcess process = {};

    FILE* f = 0;
    auto err = fopen_s(&f, path.c_str(), "rb");
    fseek(f, 0, SEEK_END);
    size_t fLen = ftell(f);
    fseek(f, 0, SEEK_SET);
    std::vector<char> fileBuf;
    fileBuf.resize(fLen);
    fread(fileBuf.data(), fLen, 1, f);
    for (auto& c : fileBuf)
    {
        if (c == '\t') //YAML doesn't accept tabs
            c = ' ';
    }
    fclose(f);

    auto doc = ryml::parse_in_arena(ryml::to_csubstr(fileBuf));
    size_t rootID = doc.root_id();
    auto nameref = doc["name"];
    for (const auto& inc : doc["includes"])
    {
        std::string inStr;
        inc >> inStr;
        process.includes.push_back(inStr);
    }

    std::string name;
    if (doc.has_child(doc.root_id(), "name"))
    {
        doc["name"] >> process.name;
    }
    else
    {
        process.name = "Shader2Header";
    }

    for (const auto& yshader : doc["shaders"])
    {
        ShaderDef def = {};
        yshader["name"] >> def.name;
        if (yshader.has_child("dynamic"))
        {
            for (const auto& d : yshader["dynamic"])
            {
                std::string dyn;
                d >> dyn;
                def.dynamicStates.push_back(dyn);
            }
        }
        if (yshader.has_child("stencil"))
        {
            def.stencil.active = true;
            const auto& sten = yshader["stencil"];
            if (sten.has_child("compareMask"))
                sten["compareMask"] >> def.stencil.compareMask;
            else
                def.stencil.compareMask = "0xffffffff";
            if (sten.has_child("writeMask"))
                sten["writeMask"] >> def.stencil.writeMask;
            else
                def.stencil.writeMask = "0xffffffff";
            if (sten.has_child("reference"))
                sten["reference"] >> def.stencil.reference;
            else
                def.stencil.reference = "1";
            if (sten.has_child("compareOp"))
                sten["compareOp"] >> def.stencil.compareOp;
            else
                def.stencil.compareOp = "VK_COMPARE_OP_LESS_OR_EQUAL";
            if (sten.has_child("failOp"))
                sten["failOp"] >> def.stencil.failOp;
            else
                def.stencil.failOp = "VK_STENCIL_OP_KEEP";
            if (sten.has_child("depthFailOp"))
                sten["depthFailOp"] >> def.stencil.depthFailOp;
            else
                def.stencil.depthFailOp = "VK_STENCIL_OP_KEEP";
            if (sten.has_child("passOp"))
                sten["passOp"] >> def.stencil.passOp;
            else
                def.stencil.passOp = "VK_STENCIL_OP_REPLACE";
        }
        if (yshader.has_child("blend"))
        {
            const auto& blend = yshader["blend"];
            if (blend.has_child("blendEnable")) blend["blendEnable"] >> def.blend.blendEnable;
            if (blend.has_child("colorWriteMask")) blend["colorWriteMask"] >> def.blend.colorWriteMask;
            if (blend.has_child("srcColorBlendFactor")) blend["srcColorBlendFactor"] >> def.blend.srcColorBlendFactor;
            if (blend.has_child("dstColorBlendFactor")) blend["dstColorBlendFactor"] >> def.blend.dstColorBlendFactor;
            if (blend.has_child("colorBlendOp")) blend["colorBlendOp"] >> def.blend.colorBlendOp;
            if (blend.has_child("srcAlphaBlendFactor")) blend["srcAlphaBlendFactor"] >> def.blend.srcAlphaBlendFactor;
            if (blend.has_child("dstAlphaBlendFactor")) blend["dstAlphaBlendFactor"] >> def.blend.dstAlphaBlendFactor;
            if (blend.has_child("alphaBlendOp")) blend["alphaBlendOp"] >> def.blend.alphaBlendOp;
        }
        if (yshader.has_child("topo"))
        {
            yshader["topo"] >> def.topo;
            if (_strcmpi(def.topo.c_str(), "tri") == 0)
                def.topo = "VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST";
            else if (_strcmpi(def.topo.c_str(), "triStrip") == 0)
                def.topo = "VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP";
            else if (_strcmpi(def.topo.c_str(), "triFan") == 0)
                def.topo = "VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN";
        }
        else
            def.topo = "VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST";
        if (yshader.has_child("depth"))
        {
            yshader["depth"] >> def.depth;
            def.depthWrite = true;
            if (yshader.has_child("depthWrite"))
            {
                std::string dpWr;
                yshader["depthWrite"] >> dpWr;
                if (_strcmpi(dpWr.data(), "VK_FALSE") == 0 || _strcmpi(dpWr.data(), "false") == 0 || _strcmpi(dpWr.data(), "0") == 0)
                    def.depthWrite = false;
            }

            if (_strcmpi(def.depth.data(), "less") == 0)
            {
                def.depth = "VK_COMPARE_OP_LESS";
            }
            else if (_strcmpi(def.depth.data(), "lessEq") == 0) //VK_COMPARE_OP_LESS_OR_EQUAL
            {
                def.depth = "VK_COMPARE_OP_LESS_OR_EQUAL";
            }
            else if (_strcmpi(def.depth.data(), "never") == 0 || 
                _strcmpi(def.depth.data(), "none") == 0 || 
                _strcmpi(def.depth.data(), "VK_COMPARE_OP_NEVER") == 0)
            {
                def.depth = "";
            }
        }
        //cullMode = "VK_CULL_MODE_NONE", frontFace = "VK_FRONT_FACE_COUNTER_CLOCKWISE";
        if (yshader.has_child("cullMode"))
            yshader["cullMode"] >> def.cullMode;
        if (yshader.has_child("frontFace"))
            yshader["frontFace"] >> def.frontFace;
        if (yshader.has_child("depthBias"))
        {
            const auto& db = yshader["depthBias"];
            def.depthBias.enabled = true;
            if (db.has_child("depthBiasConstantFactor"))
                db["depthBiasConstantFactor"] >> def.depthBias.depthBiasConstantFactor;
            if (db.has_child("depthBiasClamp"))
                db["depthBiasClamp"] >> def.depthBias.depthBiasClamp;
            else
                def.depthBias.depthBiasClamp = "-1.0";
            if (db.has_child("depthBiasSlopeFactor"))
                db["depthBiasSlopeFactor"] >> def.depthBias.depthBiasSlopeFactor;
        }
        if (yshader.has_child("vert"))
        {
            std::string vertFile;
            yshader["vert"]["name"] >> vertFile;
            def.vert.name = vertFile;
            ReadVertJson(baseFolder + vertFile, process, def);
            auto inputs = yshader["vert"]["inputs"];
            for (const auto& yvert : inputs.cchildren())
            {
                for (auto& input : def.vert.inputs)
                {
                    if (input.name == yvert.key())
                    {
                        if (yvert.has_child("offset"))
                            yvert["offset"] >> input.offset;
                        else
                            input.offset = "0";
                        if (yvert.has_child("stride"))
                            yvert["stride"] >> input.stride;
                        else
                            input.stride = BindingToStride(input.type);
                        if (yvert.has_child("binding"))
                            yvert["binding"] >> input.binding;
                        else
                            input.binding = 0;
                        if (yvert.has_child("rate"))
                        {
                            yvert["rate"] >> input.rate;

                            std::transform(input.rate.begin(), input.rate.end(), input.rate.begin(),
                                [](unsigned char c) { return std::tolower(c); });

                            input.rate = input.rate.find("instance") != std::string::npos 
                                ? "VK_VERTEX_INPUT_RATE_INSTANCE" : "VK_VERTEX_INPUT_RATE_VERTEX";
                        }
                        else
                        {
                            input.rate = "VK_VERTEX_INPUT_RATE_VERTEX";
                        }
                        if (yvert.has_child("format"))
                        {
                            yvert["format"] >> input.format;
                        }
                    }
                }
            }
            if (inputs.num_children() == 0)
            {
                int valueCount = 0;
                std::vector<int> sizeOf;
                sizeOf.resize(def.vert.inputs.size());
                for (auto& input : def.vert.inputs)
                {
                    if (sizeOf.size() <= input.loc)
                        sizeOf.resize(input.loc + 1);
                    sizeOf[input.loc] = BindingToStrideI(input.type) / 4;
                    valueCount += sizeOf[input.loc];
                }
                for (auto& input : def.vert.inputs)
                {
                    int offset = 0;
                    for (int oi = 0; oi < input.loc; ++oi)
                    {
                        offset += sizeOf[oi];
                    }
                    input.stride = "sizeof(float) * " + std::to_string(valueCount);
                    input.offset = "sizeof(float) * " + std::to_string(offset);
                    input.rate = "VK_VERTEX_INPUT_RATE_VERTEX";
                    input.binding = 0;
                }
            }
            std::string fragFile;
            yshader["frag"]["name"] >> fragFile;
            def.frag.name = fragFile;
            ReadFragJson(baseFolder + fragFile, process, def);
        }
        process.shaders.push_back(def);
    }
    OutputShaderImpl(process, baseFolder);
    OutputShaderHeader(process, baseFolder);
}
