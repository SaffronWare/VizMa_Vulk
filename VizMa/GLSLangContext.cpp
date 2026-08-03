#include "GLSLangContext.h"


GLSLangContext::GLSLangContext() { 
	glslang::InitializeProcess();
	LOG("SUPPORTED GLSL VERSION " << glslang::GetGlslVersionString());
}

GLSLangContext::~GLSLangContext() { glslang::FinalizeProcess(); }

GLSLangContext& GLSLangContext::Get()
{
	static GLSLangContext context;
	return context;
}


// credits to Andrew Huang for his amazing blog on this!: 
// https://www.andrewhuang.llc/vulkan/integrating-glslang-for-runtime-shader-compilation/
std::vector<uint32_t> GLSLangContext::compileShader(const std::string& source, EShLanguage stage) const
{
	glslang::TShader shader(stage);
	
	const char* source_c_str = source.c_str();
	shader.setStrings(&source_c_str, 1);

	shader.setEnvInput(glslang::EShSourceGlsl, EShLangVertex, glslang::EShClientVulkan, 100);
	shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_3);
	shader.setEnvTarget(glslang::EshTargetSpv, glslang::EShTargetSpv_1_0);

	shader.parse(
		GetDefaultResources(),  // default TBuiltInResource from ResourceLimits.h
		100,                    // default version
		false,                  // not forward compatible
		EShMsgDefault           // report default error/warning messages
	);


	return std::vector<uint32_t>();

}