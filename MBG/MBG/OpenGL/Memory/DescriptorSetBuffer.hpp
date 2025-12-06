#pragma once 

#include <vector>
#include <cassert>
#include <iostream>

#include "../CommandBuffer.hpp"
#include "../FrameGraph.hpp"
#include "../Info.hpp"

#include "VertexBuffer.hpp"
#include "ElementBuffer.hpp"
#include "InstancedVertexBuffer.hpp"
#include "InstancedElementBuffer.hpp"
#include "TextureBuffer.hpp"
#include "Texture2DBuffer.hpp"
#include "Texture3DBuffer.hpp"
#include "TextureCubeMapBuffer.hpp"
#include "Texture2DArrayBuffer.hpp"
#include "UniformBuffer.hpp"

namespace MBG {

class DescriptorSetBuffer {
public:
	DescriptorSetBuffer(const DescriptorSetBufferParams& params) {
		// First we generate all bindings for in buffers
		addInDescriptors(params.descriptors, params.count);

		// Second we generate bind for trasform feedback
		if (is_transform_feedback_) {
			glGenTransformFeedbacks(1, &transform_feedback_id_);
			addTransformFeedBackDescriptor(params.descriptors, params.count);
		}

		// Third we generate binding for frame buffers
		if (is_frame_buffer_) {
			glGenFramebuffers(1, &frame_buffer_id_);
			addFrameBufferDescriptor(params.descriptors, params.count);
		}
	}

	~DescriptorSetBuffer() {
		if (is_transform_feedback_) {
			glDeleteTransformFeedbacks(1, &transform_feedback_id_);
		}
		if (is_frame_buffer_) {
			glDeleteFramebuffers(1, &frame_buffer_id_);
		}
	}

protected:
	void bind(CommandBuffer& command_buffer) {
		for (const auto& b : binds_) {
			switch (b.type) {

			case BINDIND_TYPE::TEXTURE_BUFFER:
				command_buffer.activeTexture(GL_TEXTURE0 + b.slot);
				command_buffer.bindTexture(GL_TEXTURE_BUFFER, b.id);
				break;

			case BINDIND_TYPE::TEXTURE_2D:
				command_buffer.activeTexture(GL_TEXTURE0 + b.slot);
				command_buffer.bindTexture(GL_TEXTURE_2D, b.id);
				break;

			case BINDIND_TYPE::TEXTURE_3D:
				command_buffer.activeTexture(GL_TEXTURE0 + b.slot);
				command_buffer.bindTexture(GL_TEXTURE_3D, b.id);
				break;

			case BINDIND_TYPE::TEXTURE_CUBE_MAP:
				command_buffer.activeTexture(GL_TEXTURE0 + b.slot);
				command_buffer.bindTexture(GL_TEXTURE_CUBE_MAP, b.id);
				break;

			case BINDIND_TYPE::TEXTURE_ARRAY:
				command_buffer.activeTexture(GL_TEXTURE0 + b.slot);
				command_buffer.bindTexture(GL_TEXTURE_2D_ARRAY, b.id);
				break;

			case BINDIND_TYPE::FRAME_BUFFER:
				command_buffer.bindFramebuffer(GL_FRAMEBUFFER, b.id);
				break;

			case BINDIND_TYPE::TRANSFORM_FEEDBACK:
				command_buffer.bindTransformFeedback(GL_TRANSFORM_FEEDBACK, b.id);
				break;

			default:
				assert(false);
				break;
			}
		}

		for (const auto& b : uniform_binds_) {
			command_buffer.bindBufferRange(GL_UNIFORM_BUFFER, b.slot, b.id, b.start, b.size);
		}
	}

	void draw(CommandBuffer& command_buffer, GLenum render_type, GLsizei draw_count) {
		for (const auto& i : vertex_binds_) {
			command_buffer.bindVertexArray(i.id);

			command_buffer.drawArraysInstanced(
				render_type,
				i.start,
				i.size,
				draw_count
			);
		}

		for (const auto& i : element_binds_) {
			command_buffer.bindVertexArray(i.id);

			command_buffer.drawElementsInstanced(
				render_type,
				i.size,
				GLenum(i.element_type),
				(void*)size_t(i.start * i.byte_size),
				draw_count
			);
		}
	}
	
	uvec2 frame_buffer_size_ = uvec2(0, 0);

	friend class FrameGraph;
private:
	enum class BINDIND_TYPE {
		VERTEX_ARRAY,
		VERTEX_ELEMENT_ARRAY,
		TEXTURE_BUFFER,
		TEXTURE_2D,
		TEXTURE_3D,
		TEXTURE_CUBE_MAP,
		TEXTURE_ARRAY,
		UNIFORM,
		FRAME_BUFFER,
		TRANSFORM_FEEDBACK,
	};

	struct Binding {
		BINDIND_TYPE type;
		uint id;
		uint slot;
	};

	struct VertexBinding {
		uint id;
		uint start;						// offset from the begging
		uint size;						// always number of vertices
	};

	struct ElementBinding {
		uint id;
		uint start;
		uint size;	
		ELEMENT_ATTRUBUTE element_type;	// used by element buffer for indicies
		uint byte_size;					// byte size of the indices
	};

	struct UniformBinding {
		uint slot;
		uint id;
		uint start;
		uint size;
	};

	std::vector<Binding> binds_;
	std::vector<VertexBinding> vertex_binds_;
	std::vector<ElementBinding> element_binds_;
	std::vector<UniformBinding> uniform_binds_;
	
	bool is_transform_feedback_ = false;
	GLuint transform_feedback_id_ = 0u;

	bool is_frame_buffer_ = false;
	GLuint frame_buffer_id_ = 0u;

	void addInDescriptors(const Descriptor* descriptor_set, const uint count) {
		uint texture_slots = 0;
		uint uniform_slots = 0;

		for (unsigned i = 0; i < count; i++) {
			const Descriptor& d = descriptor_set[i];

			switch (d.type) {

			case DESCRIPTOR_TYPE::VERTEX_BUFFER_IN:{
				auto e = (VertexBuffer*)d.data;
				auto v = (VertexView*)d.view;

				if (d.view == nullptr) { // if nullptr we set to the default
					vertex_binds_.emplace_back(e->vertex_array_id_, 0, e->size_);
				}
				else {
					vertex_binds_.emplace_back(e->vertex_array_id_, v->start, v->size);
				}

				break;
			}

			case DESCRIPTOR_TYPE::ELEMENT_BUFFER_IN: {
				auto e = (ElementBuffer*)d.data;
				auto v = (ElementView*)d.view;

				if (d.view == nullptr) {
					element_binds_.emplace_back(e->vertex_array_id_, 0, e->element_size_, e->element_attrib_, e->element_byte_size_);
				}
				else {
					element_binds_.emplace_back(e->vertex_array_id_, v->start, v->size, e->element_attrib_, e->element_byte_size_);
				}

				break;
			}

			case DESCRIPTOR_TYPE::INSTANCED_VERTEX_BUFFER_IN: {
				auto e = (InstancedVertexBuffer*)d.data;
				auto v = (InstancedVertexView*)d.view;

				if (d.view == nullptr) { // if nullptr we set to the default
					vertex_binds_.emplace_back(e->vertex_array_id_, 0, e->size_);
				}
				else {
					vertex_binds_.emplace_back(e->vertex_array_id_, v->start, v->size);
				}

				break;
			}

			case DESCRIPTOR_TYPE::INSTANCED_ELEMENT_BUFFER_IN: {
				auto e = (InstancedElementBuffer*)d.data;
				auto v = (InstancedElementView*)d.view;

				if (d.view == nullptr) {
					element_binds_.emplace_back(e->vertex_array_id_, 0, e->element_size_, e->element_attrib_, e->element_byte_size_);
				}
				else {
					element_binds_.emplace_back(e->vertex_array_id_, v->start, v->size, e->element_attrib_, e->element_byte_size_);
				}

				break;
			}

			case DESCRIPTOR_TYPE::TEXTURE_BUFFER_IN: {
				auto e = (TextureBuffer*)d.data;
				
				if (d.view == nullptr) {
					binds_.emplace_back(BINDIND_TYPE::TEXTURE_BUFFER, e->texture_id_, texture_slots++);
				}
				else {
					assert(false && "No texture view supported for in textures");
				}

				break;
			}

			case DESCRIPTOR_TYPE::TEXTURE_2D_BUFFER_IN: {
				auto e = (Texture2DBuffer*)d.data;
				
				if (d.view == nullptr) {
					binds_.emplace_back(BINDIND_TYPE::TEXTURE_2D, e->texture_id_, texture_slots++);
				}
				else {
					assert(false && "No texture view supported for in textures");
				}

				break;
			}

			case DESCRIPTOR_TYPE::TEXTURE_3D_BUFFER_IN: {
				auto e = (Texture3DBuffer*)d.data;

				if (d.view == nullptr) {
					binds_.emplace_back(BINDIND_TYPE::TEXTURE_3D, e->texture_id_, texture_slots++);
				}
				else {
					assert(false && "No texture view supported for in textures");
				}

				break;
			}
			
			case DESCRIPTOR_TYPE::TEXTURE_CUBE_MAP_BUFFER_IN: {
				auto e = (TextureCubeMapBuffer*)d.data;

				if (d.view == nullptr) {
					binds_.emplace_back(BINDIND_TYPE::TEXTURE_CUBE_MAP, e->texture_id_, texture_slots++);
				}
				else {
					assert(false && "No texture view supported for in textures");
				}

				break;
			}

			case DESCRIPTOR_TYPE::TEXTURE_ARRAY_BUFFER_IN: {
				auto e = (Texture2DArrayBuffer*)d.data;

				if (d.view == nullptr) {
					binds_.emplace_back(BINDIND_TYPE::TEXTURE_ARRAY, e->texture_id_, texture_slots++);
				}
				else {
					assert(false && "No texture view supported for in textures");
				}
	
				break;
			}

			case DESCRIPTOR_TYPE::UNIFORM_BUFFER: {
				auto e = (UniformBuffer*)d.data;
				auto v = (UniformView*)d.view;

				if (d.view == nullptr) {
					uniform_binds_.emplace_back(uniform_slots++, e->uniform_id_, 0, e->size_);
				}
				else {
					uniform_binds_.emplace_back(uniform_slots++, e->uniform_id_, v->start, e->size_);
				}

				break;
			}

			case DESCRIPTOR_TYPE::VERTEX_BUFFER_OUT:
			case DESCRIPTOR_TYPE::ELEMENT_BUFFER_OUT:
			case DESCRIPTOR_TYPE::INSTANCED_VERTEX_BUFFER_OUT:
			case DESCRIPTOR_TYPE::INSTANCED_ELEMENT_BUFFER_OUT:
				is_transform_feedback_ = true;
				break;

			case DESCRIPTOR_TYPE::TEXTURE_BUFFER_OUT:
			case DESCRIPTOR_TYPE::TEXTURE_2D_BUFFER_OUT:
			case DESCRIPTOR_TYPE::TEXTURE_3D_BUFFER_OUT:
			case DESCRIPTOR_TYPE::TEXTURE_CUBE_MAP_BUFFER_OUT:
				is_frame_buffer_ = true;
				break;

			default:
				break;
			}
		}
	}

	void addTransformFeedBackDescriptor(const Descriptor* descriptor_set, const uint count) {
		glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, transform_feedback_id_);

		GLuint binding_index = 0;
		for (unsigned i = 0; i < count; i++) {
			const Descriptor& d = descriptor_set[i];

			switch (d.type) {
			case DESCRIPTOR_TYPE::VERTEX_BUFFER_OUT: {
				VertexBuffer* vb = (VertexBuffer*)d.data;
				auto v = (VertexView*)d.view;

				if (d.view == nullptr) { // bind whole buffer
					glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, binding_index++, vb->vertex_buffer_id_);
				}
				else { // bind only a range
					uint bytes_per_element = vb->vertex_byte_size_;
					glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, binding_index++,
						vb->vertex_buffer_id_,
						v->start * bytes_per_element,
						v->size * bytes_per_element);
				}
				break;
			}

			case DESCRIPTOR_TYPE::ELEMENT_BUFFER_OUT: {
				ElementBuffer* eb = (ElementBuffer*)d.data;
				auto v = (ElementView*)d.view;

				if (d.view == nullptr) {
					glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, binding_index++, eb->vertex_buffer_id_);
				}
				else {
					uint bytes_per_element = eb->vertex_byte_size_;
					glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, binding_index++,
						eb->vertex_buffer_id_,
						v->start * bytes_per_element,
						v->size * bytes_per_element);
				}
				break;
			}

			case DESCRIPTOR_TYPE::INSTANCED_VERTEX_BUFFER_OUT: {
				InstancedVertexBuffer* ivb = (InstancedVertexBuffer*)d.data;
				auto v = (InstancedVertexView*)d.view;

				if (d.view == nullptr) {
					glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, binding_index++, ivb->vertex_buffer_id_);
				}
				else {
					uint bytes_per_element = ivb->vertex_byte_size_;
					glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, binding_index++,
						ivb->vertex_buffer_id_,
						v->start * bytes_per_element,
						v->size * bytes_per_element);
				}
				break;
			}

			case DESCRIPTOR_TYPE::INSTANCED_ELEMENT_BUFFER_OUT: {
				InstancedElementBuffer* ieb = (InstancedElementBuffer*)d.data;
				auto v = (InstancedElementView*)d.view;

				if (d.view == nullptr) {
					glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, binding_index++, ieb->vertex_buffer_id_);
				}
				else {
					uint bytes_per_element = ieb->vertex_byte_size_;
					glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, binding_index++,
						ieb->vertex_buffer_id_,
						v->start * bytes_per_element,
						v->size * bytes_per_element);
				}
				break;
			}

			default:
				break;
			}
		}

		binds_.emplace_back(BINDIND_TYPE::TRANSFORM_FEEDBACK, transform_feedback_id_, 0);
	}
	
	void addFrameBufferDescriptor(const Descriptor* descriptor_set, const uint count) {
		glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_id_);

		GLuint color_attachment_index = 0;

		for (unsigned i = 0; i < count; i++) {
			const Descriptor& d = descriptor_set[i];

			switch (d.type) {

			case DESCRIPTOR_TYPE::TEXTURE_BUFFER_OUT: {
				TextureBuffer* tb = (TextureBuffer*)d.data;

				if (d.view == nullptr) { // Bind full texture
					glFramebufferTexture(GL_FRAMEBUFFER,
						GL_COLOR_ATTACHMENT0 + color_attachment_index++,
						tb->texture_id_,
						0);
				}
				else {
					assert(false && "No texture view supported for texture buffers");
				}

				break;
			}

			case DESCRIPTOR_TYPE::TEXTURE_2D_BUFFER_OUT: {
				Texture2DBuffer* tex2D = (Texture2DBuffer*)d.data;
				auto v = (Texture2DView*)d.view;

				if (d.view == nullptr) {
					glFramebufferTexture2D(GL_FRAMEBUFFER,
						GL_COLOR_ATTACHMENT0 + color_attachment_index++,
						GL_TEXTURE_2D,
						tex2D->texture_id_,
						0);
				}
				else {
					glFramebufferTexture(GL_FRAMEBUFFER,
						GL_COLOR_ATTACHMENT0 + color_attachment_index++,
						tex2D->texture_id_,
						v->mip_level);
				}

				break;
			}

			case DESCRIPTOR_TYPE::TEXTURE_3D_BUFFER_OUT: {
				Texture3DBuffer* tex3D = (Texture3DBuffer*)d.data;
				auto v = (Texture3DView*)d.view;

				if (d.view == nullptr) {
					glFramebufferTextureLayer(GL_FRAMEBUFFER,
						GL_COLOR_ATTACHMENT0 + color_attachment_index++,
						tex3D->texture_id_,
						0,
						0);
				}
				else {
					glFramebufferTextureLayer(GL_FRAMEBUFFER,
						GL_COLOR_ATTACHMENT0 + color_attachment_index++,
						tex3D->texture_id_,
						v->mip_level,                 
						v->slice);
				}

				break;
			}

			case DESCRIPTOR_TYPE::TEXTURE_CUBE_MAP_BUFFER_OUT: {
				TextureCubeMapBuffer* cubeTex = (TextureCubeMapBuffer*)d.data;
				auto v = (TextureCubeMapView*)d.view;

				if (d.view == nullptr) {
					GLenum target = GL_TEXTURE_CUBE_MAP_POSITIVE_X;
					glFramebufferTexture2D(GL_FRAMEBUFFER,
						GL_COLOR_ATTACHMENT0 + color_attachment_index++,
						target,
						cubeTex->texture_id_,
						0);
				}
				else {
					GLenum target = GL_TEXTURE_CUBE_MAP_POSITIVE_X + v->face;
					glFramebufferTexture2D(GL_FRAMEBUFFER,
						GL_COLOR_ATTACHMENT0 + color_attachment_index++,
						target,
						cubeTex->texture_id_,
						v->mip_level);
				}

				break;
			}

			case DESCRIPTOR_TYPE::TEXTURE_ARRAY_BUFFER_OUT: {
				Texture2DArrayBuffer* texArray = (Texture2DArrayBuffer*)d.data;
				auto v = (Texture2DArrayView*)d.view;

				if (d.view == nullptr) {
					glFramebufferTextureLayer(GL_FRAMEBUFFER,
						GL_COLOR_ATTACHMENT0 + color_attachment_index++,
						texArray->texture_id_,
						0,
						0);
				}
				else {
					glFramebufferTextureLayer(GL_FRAMEBUFFER,
						GL_COLOR_ATTACHMENT0 + color_attachment_index++,
						texArray->texture_id_,
						v->mip_level,
						v->slice);
				}
				
				break;
			}

			default:
				break;
			}
		}

		binds_.emplace_back(BINDIND_TYPE::FRAME_BUFFER, frame_buffer_id_, 0);
	}

};

}