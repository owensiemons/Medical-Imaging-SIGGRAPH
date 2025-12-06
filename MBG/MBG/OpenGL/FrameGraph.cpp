#include "FrameGraph.hpp"

#include <cassert>
#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
using namespace glm;

#include "Info.hpp"
#include "Memory/DescriptorSetBuffer.hpp"
#include "CommandBuffer.hpp"

namespace MBG {

void FrameGraph::bindRenderStates(const RenderStates& s, uvec2 default_frame_size) {
	// Depth Test + Depth Writes
	if (s.depth_test)
		command_buffer_.enable(GL_DEPTH_TEST);
	else
		command_buffer_.disable(GL_DEPTH_TEST);

	command_buffer_.depthMask(s.depth_writes);
	command_buffer_.depthFunc(GLenum(s.depth_compare));

	// Culling
	if (s.cull_mode == CULL_MODE::CULL_NONE) {
		command_buffer_.disable(GL_CULL_FACE);
	}
	else {
		command_buffer_.enable(GL_CULL_FACE);
		command_buffer_.cullFace(GLenum(s.cull_mode));
		command_buffer_.frontFace(GLenum(s.front_face));
	}

	// Stencil
	if (!s.stencil_test) {
		command_buffer_.disable(GL_STENCIL_TEST);
	}
	else {
		command_buffer_.enable(GL_STENCIL_TEST);

		command_buffer_.stencilOpSeparate(
			GL_FRONT_AND_BACK,
			GLenum(s.stencil_fail_op),
			GLenum(s.stencil_depth_fail_op),
			GLenum(s.stencil_pass_op)
		);

		command_buffer_.stencilFuncSeparate(
			GL_FRONT_AND_BACK,
			GLenum(s.stencil_compare_op),
			s.stencil_ref,
			s.stencil_read_mask
		);

		command_buffer_.stencilMask(s.stencil_write_mask);
	}

	// Blending
	if (!s.enable_blend) {
		command_buffer_.disable(GL_BLEND);
	}
	else {
		command_buffer_.enable(GL_BLEND);

		command_buffer_.blendFuncSeparate(
			GLenum(s.src_color),
			GLenum(s.dst_color),
			GLenum(s.src_alpha),
			GLenum(s.dst_alpha)
		);

		command_buffer_.blendEquationSeparate(
			GLenum(s.color_op),
			GLenum(s.alpha_op)
		);
	}

	// Multisampling
	if (s.multisample_enabled)
		command_buffer_.enable(GL_MULTISAMPLE);
	else
		command_buffer_.disable(GL_MULTISAMPLE);

	if (s.sample_shading) {
		command_buffer_.enable(GL_SAMPLE_SHADING);
		command_buffer_.minSampleShading(s.min_sample_shading);
	}
	else {
		command_buffer_.disable(GL_SAMPLE_SHADING);
	}

	// viewport
	if (s.enable_viewport) {
		command_buffer_.viewport(
			s.viewport_start.x,
			s.viewport_start.y,
			s.viewport_size.x,
			s.viewport_size.y
		);
	}
	else if (default_frame_size.x != 0 && default_frame_size.y != 0) {
		command_buffer_.viewport(
			0,
			0,
			default_frame_size.x,
			default_frame_size.y
			);
	}
	else {
		command_buffer_.defaultViewport(window_->getWindow());
	}

	// Scissor
	if (s.enable_scissor == 0) {
		command_buffer_.disable(GL_SCISSOR_TEST);
	}
	else {
		command_buffer_.enable(GL_SCISSOR_TEST);
		command_buffer_.scissor(
			s.scissor_start.x,
			s.scissor_start.y,
			s.scissor_size.x,
			s.scissor_size.y
		);
	}
}

void FrameGraph::addNode(const NodeVertexCopy& node) {
	VertexBuffer* v = node.vertex_buffer;

	command_buffer_.memoryCopy(
		GL_ARRAY_BUFFER,
		v->vertex_buffer_id_, 
		node.start * v->vertex_byte_size_, 
		node.size * v->vertex_byte_size_,
		node.data
	);
}

void FrameGraph::addNode(const NodeClear& node) {
	command_buffer_.clearColor(node.color.r, node.color.g, node.color.b, node.color.a);
	command_buffer_.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void FrameGraph::addNode(const NodeDraw& node) {
	// Add all bind commands
	assert(node.render_pass != nullptr);
	command_buffer_.useProgram(node.render_pass->shader_program_);

	assert(node.descriptor_set != nullptr);
	node.descriptor_set->bind(command_buffer_);
	
	bindRenderStates(node.render_states, node.descriptor_set->frame_buffer_size_);

	// Add all draw commands
	node.descriptor_set->draw(command_buffer_, GLenum(node.render_states.render_type), node.draw_count);
}

void FrameGraph::addNodeDisplay() {
	command_buffer_.display(window_->getWindow());
}

void FrameGraph::build() {
	// TODO: This doesn't need to be done yet

	// remove all duplicates find constants

	// bind all constants

	//command_buffer_.clear();
	//compiled_command_buffer_.clear();
}

void FrameGraph::run() {
	//compiled_command_buffer_.run();
	command_buffer_.runCommands();
}

void FrameGraph::renderRealTimeGraph()
{
}

}