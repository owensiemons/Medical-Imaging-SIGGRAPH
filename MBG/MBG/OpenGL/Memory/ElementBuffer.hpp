#pragma once

//------------------------------------------------------------
// VERTEX AND ELEMNET BUFFER
//	This class creates a vertex buffer and element buffer then
//  defines its structure
//------------------------------------------------------------

#include "VertexBuffer.hpp"
#include "../Info.hpp"
#include "../FrameGraph.hpp"
#include "DescriptorSetBuffer.hpp"

namespace MBG {

class ElementBuffer : public VertexBuffer {
public:
	ElementBuffer(ElementBufferParams params)
		: VertexBuffer({params.vertex_attributes, params.attributes_count, params.vertex_data, params.vertex_count, params.buffer_usage}), element_size_(params.element_count)
	{
		assert(params.element_count > 0);

		element_attrib_ = params.element_attribute;

		// Now we need to get the number of bytes the element is using
		element_byte_size_ = 0;
		switch (params.element_attribute) {
			case ELEMENT_ATTRUBUTE::BYTE: element_byte_size_ = 1; break;
			case ELEMENT_ATTRUBUTE::SHORT: element_byte_size_ = 2; break;
			case ELEMENT_ATTRUBUTE::UINT32: element_byte_size_ = 4; break;
			default: assert(false); break; // There is an unknown type
		}
		
		glGenBuffers(1, &element_buffer_id_);
		assert(element_buffer_id_ != 0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer_id_);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, params.element_count * element_byte_size_, params.element_data, GLenum(params.buffer_usage));
	}

	virtual ~ElementBuffer() override {
		glDeleteBuffers(1, &element_buffer_id_);
	}

	// This mapped pointer is unsynchronized and must be mapped to memory the GPU is not using
	inline const void* mapElementPtr(size_t byte_start, size_t byte_size) {
		glBindBuffer(GL_ARRAY_BUFFER, element_buffer_id_);
		glMapBufferRange(GL_ARRAY_BUFFER, byte_start, byte_size,
			GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
	}

	inline const void unmapElementPtr() {
		glBindBuffer(GL_ARRAY_BUFFER, element_buffer_id_);
		glUnmapBuffer(GL_ARRAY_BUFFER);
	}

protected:
	GLuint element_buffer_id_;
	ELEMENT_ATTRUBUTE element_attrib_; // we need this for drawing since openGL never can seam to keep things consistant
	uint element_byte_size_;
	uint element_size_; // number of elements

	friend class FrameGraph;
	friend class DescriptorSetBuffer;
};

}