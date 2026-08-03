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
std::vector<uint32_t> GLSLangContext::compileShader(const GLSLShaderCompileInfo& compileInfo) const
{
	LOG("compiling shader of type" << compileInfo.stage << " with source:\n" << compileInfo.source);
	glslang::TShader shader(compileInfo.stage);
	
	const char* source_c_str = compileInfo.source.c_str();
	shader.setStrings(&source_c_str, 1);

	shader.setEnvInput(compileInfo.language, compileInfo.stage, compileInfo.dialect, compileInfo.dialect_version);
	shader.setEnvClient(compileInfo.dialect, compileInfo.vkVersion);
	shader.setEnvTarget(glslang::EshTargetSpv, compileInfo.shaderSpvVer);

	shader.parse(
		GetDefaultResources(),  // default TBuiltInResource from ResourceLimits.h
		compileInfo.dialect_version,// set to 100 by GlslShaderCompileInfo (default)
		false,                  // not forward compatible
		EShMsgDefault           // report default error/warning messages
	);

	LOG("Parsing shader: " << shader.getInfoLog());

	glslang::TProgram program;
	program.addShader(&shader);
	program.link(EShMsgDefault);    

	LOG("Linking program: " << program.getInfoLog()); 

	glslang::TIntermediate* intermediate = program.getIntermediate(compileInfo.stage);

	std::vector<uint32_t> spirvOut;
	glslang::GlslangToSpv(*intermediate, spirvOut);
	
	return spirvOut;
}