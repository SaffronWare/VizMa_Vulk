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