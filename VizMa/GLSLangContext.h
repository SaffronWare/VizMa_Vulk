#pragma once

#include <ShaderLang.h>

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
};