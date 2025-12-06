#pragma once

//------------------------------------------------------------
// VERTICES BUFFER
//	This class creates a vertex buffer and defines its structure
//------------------------------------------------------------

#include <glad/glad.h>

#include "../Info.hpp"
#include "../FrameGraph.hpp"
#include "DescriptorSetBuffer.hpp"

namespace MBG {

class VertexBuffer {
public:
	VertexBuffer(const VertexBufferParams& params) : size_(params.count) {
		assert(params.count > 0);
		
		vertex_byte_size_ = 0; // Total number of bytes in the struct
		for (unsigned i = 0; i < params.attributes_count; i++) {
			Attributes a = params.attributes[i];
			vertex_byte_size_ += getAttributeSize(a.attribute) * a.count;
		}

		glGenVertexArrays(1, &vertex_array_id_);
		glBindVertexArray(vertex_array_id_);
		assert(vertex_array_id_ != 0);
	
		glGenBuffers(1, &vertex_buffer_id_);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_id_);
		glBufferData(GL_ARRAY_BUFFER, params.count * vertex_byte_size_, params.data, GLenum(params.buffer_usage));
		assert(vertex_buffer_id_ != 0);
		
		size_t byte_offset = 0;
		for (unsigned i = 0; i < params.attributes_count; i++) {
			glEnableVertexAttribArray(i);

			Attributes a = params.attributes[i];

			// Check the default version the data should be interpreted as
			if (isFloat(a.attribute)) { 
				glVertexAttribPointer(i, a.count, GLenum(a.attribute), GL_FALSE, vertex_byte_size_, (void*)byte_offset);
			}
			else {
				glVertexAttribIPointer(i, a.count, GLenum(a.attribute), vertex_byte_size_, (void*)byte_offset);
			}

			byte_offset += getAttributeSize(a.attribute) * a.count;
		}
	}

	virtual ~VertexBuffer() {
		glDeleteVertexArrays(1, &vertex_array_id_);
		glDeleteBuffers(1, &vertex_buffer_id_);
	}
	
	// This mapped pointer is unsynchronized and must be mapped to memory the GPU is not using
	inline const void* mapVertexPtr(size_t byte_start, size_t byte_size) {
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_id_);
		glMapBufferRange(GL_ARRAY_BUFFER, byte_start, byte_size,
			GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
	}

	inline const void unmapVertexPtr() {
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_id_);
		glUnmapBuffer(GL_ARRAY_BUFFER);
	}

protected:
	GLuint vertex_array_id_;
	GLuint vertex_buffer_id_;
	uint size_; // number of vertices
	uint vertex_byte_size_;

	friend class FrameGraph;
	friend class DescriptorSetBuffer;
};

}