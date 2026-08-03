#pragma once

#include <ShaderLang.h>
#include <ResourceLimits.h>

#include <vector>
#include <string>

#include "DEBUG.h"


struct GLSLShaderCompileInfo
{
	static const glslang::EShSource source = glslang::EShSourceGlsl;
	static const glslang::EShClient dialect = glslang::EShClientVulkan;
	static const int dialect_version = 100; // lowkey arbitrary until i
	// understand it further. It is meant to be the version of
	// GL_KHR_vulkan_glsl but im never using it explicitely soo...

	
	std::string source;
	EShLanguage shaderStage;

	// default values, no need for them to be specified
	glslang::EShTargetLanguageVersion shaderSpvVer = glslang::EShTargetSpv_1_0;
	glslang::EShTargetClientVersion vkVersion = glslang::EShTargetVulkan_1_3;

};

class GLSLangContext
{
public:
	static GLSLangContext& Get();

	GLSLangContext(const GLSLangContext&) = delete;
	GLSLangContext& operator=(const GLSLangContext&) = delete;

	GLSLangContext(GLSLangContext&&) = delete;
	GLSLangContext& operator=(GLSLangContext&&) = delete;

private:
	GLSLangContext();
	~GLSLangContext();

	std::vector<uint32_t> compileShader(const std::string& source, EShLanguage stage) const;
};