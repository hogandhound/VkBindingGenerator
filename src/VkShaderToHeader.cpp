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
		IVEC4
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
	default:
		return "16";
	}
}
struct BindingDef
{
	std::string name;
	int loc, binding;
	std::string offset, stride;
	Binding::BindingEnum type;
	size_t format;
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
};
struct ShaderVertDef
{
	std::string name;
	std::vector<BindingDef> inputs;
	std::vector<TextureDef> texs;
	std::vector<UniformDef> ubos;
};
struct ShaderDef
{
	std::string name;
	ShaderVertDef vert;
	ShaderFragDef frag;
	std::string push;
};
struct ShaderStructPart
{
   Binding::BindingEnum type;
   std::string name;
};
struct ShaderStruct
{
   std::string name;
   std::vector<ShaderStructPart> parts;
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
	auto err = fopen_s(&f, (file+".json").c_str(), "rb");
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
			input.format = VK_FORMAT_R32_SFLOAT;
		}
		else if (yinput["type"] == "vec2")
		{
			input.type = Binding::VEC2;
			input.format = VK_FORMAT_R32G32_SFLOAT;
		}
		else if (yinput["type"] == "vec3")
		{
			input.type = Binding::VEC3;
			input.format = VK_FORMAT_R32G32B32_SFLOAT;
		}
		else if (yinput["type"] == "vec4")
		{
			input.type = Binding::VEC4;
			input.format = VK_FORMAT_R32G32B32A32_SFLOAT;
		}
		else if (yinput["type"] == "mat2")
		{
			input.type = Binding::MAT2;
			input.format = VK_FORMAT_R32G32_SFLOAT;
		}
		else if (yinput["type"] == "mat3")
		{
			input.type = Binding::MAT3;
			input.format = VK_FORMAT_R32G32B32_SFLOAT;
		}
		else if (yinput["type"] == "mat4")
		{
			input.type = Binding::MAT4;
			input.format = VK_FORMAT_R32G32B32A32_SFLOAT;
		}
		else if (yinput["type"] == "int")
		{
			input.type = Binding::INT;
			input.format = VK_FORMAT_R32_SINT;
		}
		else if (yinput["type"] == "ivec2")
		{
			input.type = Binding::IVEC2;
			input.format = VK_FORMAT_R32G32_SINT;
		}
		else if (yinput["type"] == "ivec3")
		{
			input.type = Binding::IVEC3;
			input.format = VK_FORMAT_R32G32B32_SINT;
		}
		else if (yinput["type"] == "ivec4")
		{
			input.type = Binding::IVEC4;
			input.format = VK_FORMAT_R32G32B32A32_SINT;
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
		}
	}
	if (doc.has_child(doc.root_id(), "push_constants"))
	{
		std::string id;
		doc["push_constants"][0]["type"] >> id;
		shader.push = structMap[id];
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
		shader.push = structMap[id];
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
void OutputShaderImpl(ShaderProcess& process)
{
	std::string out = R"(#include ")" + process.name + R"(_shaderdef.h"
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
		out += "    VkVertexInputBindingDescription bindingDescription["+std::to_string(shader.vert.inputs.size())+"] = {};\n";
		for (int i = 0; i < shader.vert.inputs.size(); ++i)
		{
			std::string indexStr = "[" + std::to_string(i) + "]";
			out += "    {\n";
			out += "        bindingDescription" + indexStr + ".binding = " + std::to_string(shader.vert.inputs[i].binding) + ";\n";
			out += "        bindingDescription" + indexStr + ".stride = " + shader.vert.inputs[i].stride + ";\n";
			out += "        bindingDescription" + indexStr + ".inputRate = VK_VERTEX_INPUT_RATE_VERTEX;\n";
			out += "    }\n";
		}
		out += "    VkVertexInputAttributeDescription attributeDescriptions[" + std::to_string(shader.vert.inputs.size()) + "] = {};\n";
		for (int i = 0; i < shader.vert.inputs.size(); ++i)
		{
			std::string indexStr = "[" + std::to_string(i) + "]";
			out += "    {\n";
			out += "        attributeDescriptions" + indexStr + ".binding = " + std::to_string(shader.vert.inputs[i].binding) + ";\n";
			out += "        attributeDescriptions" + indexStr + ".location = " + std::to_string(shader.vert.inputs[i].loc) + ";\n";
			out += "        attributeDescriptions" + indexStr + ".format = (VkFormat)" + std::to_string(shader.vert.inputs[i].format) + ";\n";
			out += "        attributeDescriptions" + indexStr + ".offset = " + shader.vert.inputs[i].offset + ";\n";
			out += "    }\n";
		}

		out += "    vertexInputInfo.vertexBindingDescriptionCount = " + std::to_string(shader.vert.inputs.size()) + ";\n";
		out += "    vertexInputInfo.vertexAttributeDescriptionCount = " + std::to_string(shader.vert.inputs.size()) + ";\n";
		out += R"z(
    vertexInputInfo.pVertexBindingDescriptions = bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;)z";
		out += shader_mid;
		out += shader_end;



		//Create Desc Layout
		std::vector<StagesDef<TextureDef>> texs = BuildStages(shader.vert.texs, shader.frag.texs);
		std::vector<StagesDef<UniformDef>> ubos = BuildStages(shader.vert.ubos, shader.frag.ubos);
		out += "\n\nvoid " + process.name + "_Create" + shader.name + "DescriptorSetLayout(VkRenderTarget * target, VKPipelineData& pipeline) {\n" +
			"    VkDescriptorSetLayoutBinding bindings[" + 
			std::to_string( texs.size() + ubos.size())
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
			std::string indexStr = "[" + std::to_string(ubos.size() + i) + "]";
			out += "    {\n";
			out += "        bindings" + indexStr + ".binding = " + std::to_string(ubos[i].def.binding) + ";\n";
			out += "        bindings" + indexStr + ".descriptorCount = 1;\n";
			out += "        bindings" + indexStr + ".descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;\n";
			out += "        bindings" + indexStr + ".pImmutableSamplers = nullptr;\n";
			out += "        bindings" + indexStr + ".stageFlags = " + ubos[i].stages + ";\n";
			out += "    }\n";
		}
		std::string dstype = "";
		for (auto& def : texs)
			dstype += "T";
		for (auto& def : ubos)
			dstype += "U";
		out += R"(
    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = )" + std::to_string(texs.size() + ubos.size()) + R"(;
    layoutInfo.pBindings = bindings;

    pipeline.subIndex = target->getDescSetSubIndex(VkDS_)" + dstype + R"(, bindings, )" + std::to_string(texs.size() + ubos.size()) + R"();
    if (vkCreateDescriptorSetLayout(target->device, &layoutInfo, nullptr, &pipeline.descriptorSetLayout) != VK_SUCCESS) {}
}
)";

		//Update Desc Set
/*
void PipelineImageDraw::UpdateDescriptorSets(VkRenderTarget* target, VkTexture* texture) {

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
		out += "void " + process.name + "_Update" + shader.name + "DescriptorSets(VkRenderTarget * target, VKPipelineData& pipeline";
		for (auto& def : texs)
		{
			out += ", VkTexture* texture_" + def.def.name;
		}
		for (auto& def : ubos)
		{
			out += ", VkUniform* ubo_" + def.def.name;
		}
		out += ") {\n";

		out += "    VkDescriptorSet descriptorSet = target->getDescSet(VkDS_" + dstype + ", pipeline.subIndex, &pipeline.descriptorSetLayout);\n";
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
			out += "    descriptorWrites" + indexStr + ".pImageInfo = &imageInfo_"+texs[i].def.name+";\n";
		}
		for (int i = 0; i < ubos.size(); ++i)
		{
			std::string indexStr = "[" + std::to_string(texs.size() + i) + "]";
			out += "    VkDescriptorBufferInfo bufferInfo_" + ubos[i].def.name + " = {};\n";
			out += "    bufferInfo_" + ubos[i].def.name + ".buffer = ubo_"+ubos[i].def.name+"->buffer;\n";
			out += "    bufferInfo_" + ubos[i].def.name + ".offset = 0;\n";
			out += "    bufferInfo_" + ubos[i].def.name + ".range = sizeof("+ ubos[i].def.name +");\n";
			out += "    \n";
			out += "    descriptorWrites" + indexStr + ".sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;\n";
			out += "    descriptorWrites" + indexStr + ".dstSet = descriptorSet;\n";
			out += "    descriptorWrites" + indexStr + ".dstBinding = " + std::to_string(ubos[i].def.binding) + ";\n";
			out += "    descriptorWrites" + indexStr + ".dstArrayElement = 0;\n";
			out += "    descriptorWrites" + indexStr + ".descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;\n";
			out += "    descriptorWrites" + indexStr + ".descriptorCount = 1;\n";
			out += "    descriptorWrites" + indexStr + ".pBufferInfo = &bufferInfo_" + ubos[i].def.name + ";\n";
		}
		out += "    vkUpdateDescriptorSets(target->device, 1, descriptorWrites, 0, nullptr);\n}\n";
	}
	out += "void " + process.name + "_PopulatePipeline(VkRenderTarget* target, " + process.name + "_Pipeline_Collection& col)\n"
		"{\n";
	for (auto& p : process.shaders)
	{
		//void vktest_PopulatePipeline(VkRenderTarget* target, vktest_Pipeline_Collection& col)
		//{
		//	vktest_CreateTexturePipeline(target, col.pipelines[PIPELINE_vktest_Texture]);
		//}
		out += "    " + process.name + "_Create" + p.name + "Pipeline(target, col.pipelines[PIPELINE_" + process.name + "_" + p.name + "]);\n";
	}
	out += "}\n";
	
			//Draw Command
		out += R"(

void )" + process.name + "_Draw" + p.name + R"((VKPipelineData* pipeline, VkCommandBuffer command, VkBuffer vertexBuffer, VkBuffer indexBuffer, std::vector<VkDescriptorSet>& sets)";
		if (!shader.push.empty())
		{
			out += ", " + shader.push + "* push";
		}
		out+= std::string(")") + R"(
{
    vkCmdBindPipeline(command, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->graphicsPipeline);

    VkBuffer vertexBuffers[] = { vertexBuffer };
    VkDeviceSize offsets[] = { 0 };
)";
		if (!shader.push.empty())
		{
			out += R"(    vkCmdPushConstants(
        command,
        pipeline->pipelineLayout,
        VK_SHADER_STAGE_VERTEX_BIT,
        0,
        sizeof()"+shader.push+")" + R"(,
        push);
)";
		}
out += R"(
    vkCmdBindVertexBuffers(command, 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(command, indexBuffer, 0, VK_INDEX_TYPE_UINT16);

    vkCmdBindDescriptorSets(command, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->pipelineLayout, 0, sets.size(), sets.data(), 0, nullptr);

    vkCmdDrawIndexed(command, 6, 1, 0, 0, 0);
})";
	//OutputDebugStringA(out.c_str());
	FILE* f = 0;
	std::string filename = process.name + "_shaderdef.cpp";
	fopen_s(&f, filename.c_str(), "wb");
	fwrite(out.c_str(), 1, out.size(), f);
	fclose(f);
}

void DumpShader(ShaderProcess& process)
{
	std::string baseFolder = "shaders\\";
	std::unordered_map<std::string, bool> dumped;
	std::string output;

	output += "#pragma once\n";
	output += "#include \"VkStructs.h\"\n";

	output += "enum " + process.name + "_Pipeline_Entry {\n";
	for (auto& p : process.shaders)
	{
		output += "    PIPELINE_" + process.name +"_" + p.name +",\n";
	}
	output += "    PIPELINE_" + process.name + "_MAX\n";
	output += "};\n";
	output += "struct " + process.name + "_Pipeline_Collection {\n"
		"    VKPipelineData pipelines[PIPELINE_" + process.name + "_MAX];\n"
		"};\n";
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
			std::vector<char> fileBuf;
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
			output += "const char " + process.name + "_" + name + "[] = {\n";
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
			output += "const char " + process.name + "_" + name + "[] = {\n";
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
	std::string filename = process.name + "_shaderdef.h";
	fopen_s(&f, filename.c_str(), "wb");
	fwrite(output.c_str(), 1, output.size(), f);
	fclose(f);
}

void Shader2Header(std::string baseFolder)
{
	//std::string baseFolder = "shaders\\";
	std::string path = "shaders\\compileinfo.json";

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
		if (yshader.has_child("vert"))
		{
			std::string vertFile;
			yshader["vert"]["name"] >> vertFile;
			def.vert.name = vertFile;
			ReadVertJson(baseFolder + vertFile, process, def);
			auto inputs = yshader["vert"]["inputs"];
			for (const auto& yvert : yshader["vert"]["inputs"].cchildren())
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
						if (yvert.has_child("format"))
						{
							//TODO: Non-basic format swap
						}
					}
				}
			}
			std::string fragFile;
			yshader["frag"]["name"] >> fragFile;
			def.frag.name = fragFile;
			ReadFragJson(baseFolder + fragFile, process, def);
		}
		process.shaders.push_back(def);
	}
	OutputShaderImpl(process);
	DumpShader(process);
}