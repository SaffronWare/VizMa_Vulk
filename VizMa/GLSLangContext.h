#pragma once

#include <ShaderLang.h>
#include <ResourceLimits.h>

#include <vector>
#include <string>

#include "DEBUG.h"

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