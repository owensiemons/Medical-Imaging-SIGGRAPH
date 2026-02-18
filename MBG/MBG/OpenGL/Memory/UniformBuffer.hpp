#pragma once

#include <glad/glad.h>

#include "../Info.hpp"
#include "../FrameGraph.hpp"
#include "DescriptorSetBuffer.hpp"

//------------------------------------------------------------
// UNIFORM BUFFER OBJECT
//	This creates a uniform buffer struct
//------------------------------------------------------------

namespace MBG {

class UniformBuffer {
public:
	UniformBuffer(const UniformBufferParams& params) : size_(params.size) {
		assert(size_ > 0);
		glGenBuffers(1, &uniform_id_);
		glBindBuffer(GL_UNIFORM_BUFFER, uniform_id_);
		glBufferData(GL_UNIFORM_BUFFER, params.size, params.data, GLenum(params.buffer_usage));
	}

	// This mapped pointer is unsynchronized and must be mapped to memory the GPU is not using
	inline const void* mapPtr(size_t byte_start, size_t byte_size) {
		glBindBuffer(GL_UNIFORM_BUFFER, uniform_id_);
		glMapBufferRange(GL_UNIFORM_BUFFER, byte_start, byte_size,
			GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
	}

	inline const void unmapPtr() {
		glBindBuffer(GL_UNIFORM_BUFFER, uniform_id_);
		glUnmapBuffer(GL_UNIFORM_BUFFER);
	}

	size_t getSize() const { return size_; }

protected:
	GLuint uniform_id_;
	size_t size_;

	friend class FrameGraph;
	friend class DescriptorSetBuffer;
private:
};

}