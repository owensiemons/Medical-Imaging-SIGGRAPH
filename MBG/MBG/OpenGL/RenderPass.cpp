#include "RenderPass.hpp"

#include <glad/glad.h>

#include <cassert>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>

namespace MBG {

GLuint RenderPass::buildShader(const std::string& shader_code, GLenum shader_type) {
	if (shader_code.size() == 0) return 0; // we can ignore this shader then

	GLuint shader_id = glCreateShader(shader_type);
	assert(shader_id != 0);

	const char* str = shader_code.c_str();
	glShaderSource(shader_id, 1, &str, NULL);
	glCompileShader(shader_id);

	int success;
	glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);
	if (!success) {
		char log[512];
		glGetShaderInfoLog(shader_id, 512, NULL, log);
		std::cout << "Error Shader:\n" << log << std::endl;
	}

	glAttachShader(shader_program_, shader_id);

	return shader_id;
}

// TODO: this block can be slow because of stringstream
RenderPass::ShaderBlock RenderPass::getShaderBlocks(const std::string& shader_file) {
	std::ifstream stream(shader_file);
	if (!stream) {
		std::cerr << "Error: shader file doesn't exist " << shader_file << std::endl;
		return {};
	}

	std::string line;
	std::stringstream ss[5];
	SHADER_TYPE type = SHADER_TYPE::NONE;

	while (getline(stream, line)) {
		if (line.find("#shader") != std::string::npos) {
			if (line.find("VERTEX") != std::string::npos) 
				type = SHADER_TYPE::VERTEX;
			else if (line.find("TESSELLATION_CONTROL") != std::string::npos) 
				type = SHADER_TYPE::TESSELLATION_CONTROL;
			else if (line.find("TESSELLATION_EVALUATION") != std::string::npos) 
				type = SHADER_TYPE::TESSELLATION_EVALUATION;
			else if (line.find("GEOMETRY") != std::string::npos) 
				type = SHADER_TYPE::GEOMETRY;
			else if (line.find("FRAGMENT") != std::string::npos) 
				type = SHADER_TYPE::FRAGMENT;
			else 
				type = SHADER_TYPE::NONE;
		}
		else if (type != SHADER_TYPE::NONE) {
			ss[(int)type] << line << '\n';
		}
	}

	return { ss[0].str(), ss[1].str(), ss[2].str(), ss[3].str(), ss[4].str() };
}

RenderPass::RenderPass(const std::string& shader_file) {
	ShaderBlock shader = getShaderBlocks(shader_file);
	
	shader_program_ = glCreateProgram();
	assert(shader_program_ != 0); // Error creating shader program

	int vertex_id = buildShader(shader.vertex_code, GL_VERTEX_SHADER);
	int geometry_id = buildShader(shader.geometry_code, GL_GEOMETRY_SHADER);
	int tess_control_id = buildShader(shader.tessellation_control_code, GL_TESS_CONTROL_SHADER);
	int tess_evaluation_id = buildShader(shader.tessellation_evaluation_code, GL_TESS_EVALUATION_SHADER);
	int fragment_id = buildShader(shader.fragment_code, GL_FRAGMENT_SHADER);

	glLinkProgram(shader_program_);

	// check for linking errors
	int success;
	glGetProgramiv(shader_program_, GL_LINK_STATUS, &success);
	if (!success) {
		char log[512];
		glGetProgramInfoLog(shader_program_, 512, NULL, log);
		std::cout << "Error linking shaders: " << log << std::endl;
	}

	// shader's with id = 0 will be silently ignored
	glDeleteShader(vertex_id);
	glDeleteShader(geometry_id);
	glDeleteShader(tess_control_id);
	glDeleteShader(tess_evaluation_id);
	glDeleteShader(fragment_id);

	// Finnally we build all of the naming that we will use for the textures
	GLint maxCombinedUnits = 0;
	glUseProgram(shader_program_);
	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxCombinedUnits);
	for (int i = 0; i < maxCombinedUnits; ++i) {
		std::string name = "tex" + std::to_string(i);
		GLint loc = glGetUniformLocation(shader_program_, name.c_str());
		if (loc != -1) {
			glUniform1i(loc, i); // assign uniform to matching texture slot
		}
	}
}

RenderPass::~RenderPass() {
	glDeleteProgram(shader_program_);
}

}