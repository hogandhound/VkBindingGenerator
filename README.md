# VkBindingGenerator

Define a 'compileinfo.json' which associates vertex and fragment stages.

Requires each shader to be compiled to SPIRV and then the SPIRV to be reflected into JSON. The SPIRV and JSON should have the same name except for the extension.

Example to reflect the SPIRV into JSON:
spirv-cross texture.vert.spv --reflect --output texture.vert.json

Running the 'Shader2Header()' function will output matching cpp and h files. The header file will contain the SPIRV file as character arrays. The source will contain functions to generate the pipelines, update their descriptor sets, and run the pipelines.