#pragma once

#include "DescriptorSetBuffer.hpp"
#include "ElementBuffer.hpp"

namespace MBG {


class InstancedElementBuffer final : public ElementBuffer {
public:
	InstancedElementBuffer(const InstancedElementBufferParams& params)
		: ElementBuffer({ 
			params.vertex_attributes,
			params.attributes_count,
			params.vertex_data,
			params.vertex_count,
			params.element_attribute,
			params.element_data,
			params.element_count,
			params.buffer_usage,
		}),
		instance_count_(params.instance_count)
	{
		assert(params.instance_count > 0);

		// Compute the instance struct size
		instance_struct_size_ = 0;
		for (unsigned i = 0; i < params.instance_attributes_count; i++) {
			Attributes a = params.instance_attributes[i];
			instance_struct_size_ += getAttributeSize(a.attribute) * a.count;
		}

		// Create the instance buffer
		glGenBuffers(1, &instance_buffer_id_);
		assert(instance_buffer_id_ != 0);

		glBindBuffer(GL_ARRAY_BUFFER, instance_buffer_id_);
		glBufferData(GL_ARRAY_BUFFER,
			params.instance_count * instance_struct_size_,
			params.instance_data,
			GLenum(params.buffer_usage));

		// Add the instance attributes after the normal ones
		size_t byte_offset = 0;
		uint attrib_index = params.attributes_count;

		for (unsigned i = 0; i < params.instance_attributes_count; i++) {
			glEnableVertexAttribArray(attrib_index);

			Attributes a = params.instance_attributes[i];

			if (isFloat(a.attribute)) {
				glVertexAttribPointer(attrib_index, a.count, GLenum(a.attribute), GL_FALSE,
					instance_struct_size_, (void*)byte_offset);
			}
			else {
				glVertexAttribIPointer(attrib_index, a.count, GLenum(a.attribute),
					instance_struct_size_, (void*)byte_offset);
			}

			// set divisor for instancing
			glVertexAttribDivisor(attrib_index, 1);

			byte_offset += getAttributeSize(a.attribute) * a.count;
			attrib_index++;
		}
	}

	virtual ~InstancedElementBuffer() override {
		glDeleteBuffers(1, &instance_buffer_id_);
	}

	// This mapped pointer is unsynchronized and must be mapped to memory the GPU is not using
	inline const void* mapInstancePtr(size_t byte_start, size_t byte_size) {
		glBindBuffer(GL_ARRAY_BUFFER, instance_buffer_id_);
		glMapBufferRange(GL_ARRAY_BUFFER, byte_start, byte_size,
			GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
	}

	inline const void unmapInstancePtr() {
		glBindBuffer(GL_ARRAY_BUFFER, instance_buffer_id_);
		glUnmapBuffer(GL_ARRAY_BUFFER);
	}

protected:
	GLuint instance_buffer_id_ = 0;
	uint instance_struct_size_ = 0;   // size of each instance "struct"
	uint instance_count_ = 0;

	friend class FrameGraph;
	friend class DescriptorSetBuffer;
};

}