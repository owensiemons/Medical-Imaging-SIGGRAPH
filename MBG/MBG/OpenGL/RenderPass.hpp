#pragma once

#include <glad/glad.h>

#include <string>
#include "FrameGraph.hpp"

//------------------------------------------------------------
// RENDER PASS
//  Place these headers where you want to identify your shader code's type
//  #shader VERTEX
//  #shader GEOMETRY
//  #shader TESSELLATION_CONTROL
//  #shader TESSELLATION_EVALUATION
//  #shader FRAGMENT
//------------------------------------------------------------

namespace MBG {

class RenderPass {
public:
	RenderPass(const std::string& shader_file); 
	~RenderPass();
	void changeShader(const std::string& shader_file);
protected:
	GLuint shader_program_;

	friend class FrameGraph;
private:
	enum class SHADER_TYPE {
		NONE = -1,
		VERTEX = 0,
		TESSELLATION_CONTROL = 1,
		TESSELLATION_EVALUATION = 2,
		GEOMETRY = 3,
		FRAGMENT = 4,
	};

	struct ShaderBlock {
		std::string vertex_code;
		std::string tessellation_control_code;
		std::string tessellation_evaluation_code;
		std::string geometry_code;
		std::string fragment_code;
	};

	ShaderBlock getShaderBlocks(const std::string& shader_file);
	GLuint buildShader(const std::string& shader_code, GLenum shader_type);
	void buildFromFile(const std::string& shader_file);
};

}