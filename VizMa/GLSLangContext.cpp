#include "GLSLangContext.h"

GLSLangContext::GLSLangContext() { glslang::InitializeProcess(); }

GLSLangContext::~GLSLangContext() { glslang::FinalizeProcess(); }

GLSLangContext& GLSLangContext::Get()
{
	static GLSLangContext context;
	return context;
}