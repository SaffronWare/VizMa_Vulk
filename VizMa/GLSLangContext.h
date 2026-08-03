#pragma once

#include <ShaderLang.h>
#include <ResourceLimits.h>
#include <glslang/SPIRV/GlslangToSpv.h>

#include <vector>
#include <string>
#include <optional>


#include "DEBUG.h"


struct GLSLShaderCompileInfo
{
	static constexpr glslang::EShSource language = glslang::EShSourceGlsl;
	static constexpr glslang::EShClient dialect = glslang::EShClientVulkan;
	static constexpr int dialect_version = 100; // lowkey arbitrary until i
	// understand it further. It is meant to be the version of
	// GL_KHR_vulkan_glsl but im never using it explicitely soo...

	std::string source;
	EShLanguage stage;

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

	std::vector<uint32_t> compileShader(const GLSLShaderCompileInfo& compileInfo) const;
};