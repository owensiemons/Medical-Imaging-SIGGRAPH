#pragma once

#include <glad/glad.h>

#include <glm/glm.hpp>
using namespace glm;

//------------------------------------------------------------
// INDIRECT DRAW BUFFER
//	This creates a indirect draw buffer that can be used with
//	multi draw. However, since we are using opengl we need to
//	emulate the behaviour. So nothing exits on the GPU.
//------------------------------------------------------------

namespace MBG {

class IndirectDrawBuffer
{
public:
	IndirectDrawBuffer(IndirectDrawBufferParams params) {

	}

	~IndirectDrawBuffer() {

	}

	// This mapped pointer is unsynchronized and must be mapped to memory the GPU is not using
	//inline const void* mapElementPtr(size_t byte_start, size_t byte_size) {
	//	glBindBuffer(GL_ARRAY_BUFFER, element_buffer_id_);
	//	glMapBufferRange(GL_ARRAY_BUFFER, byte_start, byte_size,
	//		GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
	//}
	//
	//inline const void unmapElementPtr() {
	//	glBindBuffer(GL_ARRAY_BUFFER, element_buffer_id_);
	//	glUnmapBuffer(GL_ARRAY_BUFFER);
	//}

private:

};

}