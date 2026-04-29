#pragma once

#include <iostream>
#include <vector>
#include <cstring>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>

#include "Info.hpp"

namespace MBG {

class CommandBuffer {
private:
    enum class COMMAND_TYPE {
        ACTIVE_TEXTURE,
        BIND_TEXTURE,
        BIND_BUFFER_BASE,
        BIND_BUFFER_RANGE,
        BIND_FRAMEBUFFER,
        BIND_TRANSFORM_FEEDBACK,
        BIND_VERTEX_ARRAY,
        DRAW_ARRAYS_INSTANCED,
        DRAW_ELEMENTS_INSTANCED,
        ENABLE,
        DISABLE,
        DEPTH_MASK,
        DEPTH_FUNC,
        CULL_FACE,
        FRONT_FACE,
        STENCIL_OP_SEPARATE,
        STENCIL_FUNC_SEPARATE,
        STENCIL_MASK,
        BLEND_FUNC_SEPARATE,
        BLEND_EQUATION_SEPARATE,
        MIN_SAMPLE_SHADING,
        VIEWPORT,
        SCISSOR,
        USE_PROGRAM,
        CLEAR_COLOR,
        CLEAR,

        // Non-native opengl commands
        DEFAULT_VIEWPORT,
        MEMORY_COPY,
        IMGUI_RENDER,
        DISPLAY
    };

    // All Command Parameters
    struct ActiveTexture { GLenum slot; };
    struct BindTexture { GLenum type; GLuint id; };
    struct BindBufferBase { GLenum type; GLuint slot; GLuint id; };
    struct BindBufferRange { GLenum target; GLuint index; GLuint buffer; GLintptr offset; GLsizeiptr size; };
    struct BindFramebuffer { GLenum type; GLuint id; };
    struct BindTransformFB { GLenum type; GLuint id; };
    struct BindVertexArray { GLuint id; };

    struct DrawArraysInst { GLenum mode; GLint first; GLsizei count; GLsizei instancecount; };
    struct DrawElementsInst { GLenum mode; GLsizei count; GLenum type; const void* indices; GLsizei instancecount; };

    struct EnableParam { GLenum type; };
    struct DisableParam { GLenum type; };
    struct DepthMaskParam { GLboolean flag; };
    struct DepthFuncParam { GLenum func; };
    struct CullFaceParam { GLenum mode; };
    struct FrontFaceParam { GLenum mode; };

    struct StencilOpSep { GLenum face, sfail, dpfail, dppass; };
    struct StencilFuncSep { GLenum face, func; GLint ref; GLuint mask; };
    struct StencilMaskParam { GLuint mask; };

    struct BlendFuncSep { GLenum srcRGB, dstRGB, srcAlpha, dstAlpha; };
    struct BlendEqSep { GLenum modeRGB, modeAlpha; };

    struct MinSampleShadingP { GLfloat value; };

    struct ViewportParam { GLint x, y; GLsizei w, h; };
    struct ScissorParam { GLint x, y; GLsizei w, h; };

    struct UseProgramParam { GLuint program; };

    struct ClearColor { GLfloat r; GLfloat g; GLfloat b; GLfloat a; };
    struct Clear { GLbitfield bits; };

    struct MemoryCopy { GLenum target; GLuint buffer; GLsizei byte_start; GLsizei byte_size; void* data; };

    struct DefaultViewport { GLFWwindow* window; };
    struct DisplayParam { GLFWwindow* window; };

public:
    CommandBuffer() {
        // Start by allocating a decent size block of memory
        types_.reserve(128);
        data_.reserve(4096);
    }

    ~CommandBuffer() = default;

    // Command Recording Operations
    inline void activeTexture(GLenum slot) {
        addCommand(COMMAND_TYPE::ACTIVE_TEXTURE, ActiveTexture{ slot });
    }

    inline void bindTexture(GLenum type, GLuint id) {
        addCommand(COMMAND_TYPE::BIND_TEXTURE, BindTexture{ type, id });
    }

    inline void bindBufferBase(GLenum type, GLuint slot, GLuint id) {
        addCommand(COMMAND_TYPE::BIND_BUFFER_BASE, BindBufferBase{ type, slot, id });
    }

    inline void bindBufferRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size) {
        addCommand(COMMAND_TYPE::BIND_BUFFER_RANGE, BindBufferRange{ target, index, buffer, offset, size });
    }

    inline void bindFramebuffer(GLenum type, GLuint id) {
        addCommand(COMMAND_TYPE::BIND_FRAMEBUFFER, BindFramebuffer{ type, id });
    }

    inline void bindTransformFeedback(GLenum type, GLuint id) {
        addCommand(COMMAND_TYPE::BIND_TRANSFORM_FEEDBACK, BindTransformFB{ type, id });
    }

    inline void bindVertexArray(GLuint id) {
        addCommand(COMMAND_TYPE::BIND_VERTEX_ARRAY, BindVertexArray{ id });
    }

    inline void drawArraysInstanced(GLenum mode, GLint first, GLsizei count, GLsizei instancecount) {
        addCommand(COMMAND_TYPE::DRAW_ARRAYS_INSTANCED, DrawArraysInst{ mode, first, count, instancecount });
    }

    inline void drawElementsInstanced(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount) {
        addCommand(COMMAND_TYPE::DRAW_ELEMENTS_INSTANCED, DrawElementsInst{ mode, count, type, indices, instancecount });
    }

    inline void enable(GLenum type) {
        addCommand(COMMAND_TYPE::ENABLE, EnableParam{ type });
    }

    inline void disable(GLenum type) {
        addCommand(COMMAND_TYPE::DISABLE, DisableParam{ type });
    }

    inline void depthMask(GLboolean flag) {
        addCommand(COMMAND_TYPE::DEPTH_MASK, DepthMaskParam{ flag });
    }

    inline void depthFunc(GLenum func) {
        addCommand(COMMAND_TYPE::DEPTH_FUNC, DepthFuncParam{ func });
    }

    inline void cullFace(GLenum mode) {
        addCommand(COMMAND_TYPE::CULL_FACE, CullFaceParam{ mode });
    }

    inline void frontFace(GLenum mode) {
        addCommand(COMMAND_TYPE::FRONT_FACE, FrontFaceParam{ mode });
    }

    inline void stencilOpSeparate(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass) {
        addCommand(COMMAND_TYPE::STENCIL_OP_SEPARATE, StencilOpSep{ face, sfail, dpfail, dppass });
    }

    inline void stencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask) {
        addCommand(COMMAND_TYPE::STENCIL_FUNC_SEPARATE, StencilFuncSep{ face, func, ref, mask });
    }

    inline void stencilMask(GLuint mask) {
        addCommand(COMMAND_TYPE::STENCIL_MASK, StencilMaskParam{ mask });
    }

    inline void blendFuncSeparate(GLenum sRGB, GLenum dRGB, GLenum sA, GLenum dA) {
        addCommand(COMMAND_TYPE::BLEND_FUNC_SEPARATE, BlendFuncSep{ sRGB, dRGB, sA, dA });
    }

    inline void blendEquationSeparate(GLenum mRGB, GLenum mA) {
        addCommand(COMMAND_TYPE::BLEND_EQUATION_SEPARATE, BlendEqSep{ mRGB, mA });
    }

    inline void minSampleShading(GLfloat value) {
        addCommand(COMMAND_TYPE::MIN_SAMPLE_SHADING, MinSampleShadingP{ value });
    }

    inline void viewport(GLint x, GLint y, GLsizei w, GLsizei h) {
        addCommand(COMMAND_TYPE::VIEWPORT, ViewportParam{ x, y, w, h });
    }

    inline void scissor(GLint x, GLint y, GLsizei w, GLsizei h) {
        addCommand(COMMAND_TYPE::SCISSOR, ScissorParam{ x, y, w, h });
    }

    inline void useProgram(GLuint program) {
        addCommand(COMMAND_TYPE::USE_PROGRAM, UseProgramParam{ program });
    }

    inline void clearColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
        addCommand(COMMAND_TYPE::CLEAR_COLOR, ClearColor{ r, g, b, a });
    }    
    
    inline void clear(GLbitfield bits) {
        addCommand(COMMAND_TYPE::CLEAR, Clear{ bits });
    }

    inline void memoryCopy(GLenum target, GLuint buffer, GLsizei byte_start, GLsizei byte_size, void* data) {
        addCommand(COMMAND_TYPE::MEMORY_COPY, MemoryCopy{ target, buffer, byte_start, byte_size, data });
    }

    inline void defaultViewport(GLFWwindow* window) {
        addCommand(COMMAND_TYPE::DEFAULT_VIEWPORT, DefaultViewport{ window });
    }

    inline void imguiRender() {
        addCommand(COMMAND_TYPE::IMGUI_RENDER, uint8_t{ 0 });
    }

    inline void display(GLFWwindow* window) {
        addCommand(COMMAND_TYPE::DISPLAY, DisplayParam{ window });
    }

    inline void clearCommands() {
        types_.clear();
        data_.clear();
        current_type_ = current_data_ = 0;
    }

    // Execute all commands
    void runCommands() {
        while (true) {

            COMMAND_TYPE cmd = types_[current_type_++];
            void* ptr = data_.data() + current_data_;

            switch (cmd) {
            case COMMAND_TYPE::ACTIVE_TEXTURE: {
                auto* p = (ActiveTexture*)ptr;
                current_data_ += sizeof(*p);
                glActiveTexture(p->slot);
                break;
            }
            case COMMAND_TYPE::BIND_TEXTURE: {
                auto* p = (BindTexture*)ptr;
                current_data_ += sizeof(*p);
                glBindTexture(p->type, p->id);
                break;
            }
            case COMMAND_TYPE::BIND_BUFFER_BASE: {
                auto* p = (BindBufferBase*)ptr;
                current_data_ += sizeof(*p);
                glBindBufferBase(p->type, p->slot, p->id);
                break;
            }
            case COMMAND_TYPE::BIND_BUFFER_RANGE: {
                auto* p = (BindBufferRange*)ptr;
                current_data_ += sizeof(*p);
                glBindBufferRange(p->target, p->index, p->buffer, p->offset, p->size);
                break;
            }
            case COMMAND_TYPE::BIND_FRAMEBUFFER: {
                auto* p = (BindFramebuffer*)ptr;
                current_data_ += sizeof(*p);
                glBindFramebuffer(p->type, p->id);
                break;
            }
            case COMMAND_TYPE::BIND_TRANSFORM_FEEDBACK: {
                auto* p = (BindTransformFB*)ptr;
                current_data_ += sizeof(*p);
                glBindTransformFeedback(p->type, p->id);
                break;
            }
            case COMMAND_TYPE::BIND_VERTEX_ARRAY: {
                auto* p = (BindVertexArray*)ptr;
                current_data_ += sizeof(*p);
                glBindVertexArray(p->id);
                break;
            }
            case COMMAND_TYPE::DRAW_ARRAYS_INSTANCED: {
                auto* p = (DrawArraysInst*)ptr;
                current_data_ += sizeof(*p);
                glDrawArraysInstanced(p->mode, p->first, p->count, p->instancecount);
                break;
            }
            case COMMAND_TYPE::DRAW_ELEMENTS_INSTANCED: {
                auto* p = (DrawElementsInst*)ptr;
                current_data_ += sizeof(*p);
                glDrawElementsInstanced(p->mode, p->count, p->type, p->indices, p->instancecount);
                break;
            }
            case COMMAND_TYPE::ENABLE: {
                auto* p = (EnableParam*)ptr;
                current_data_ += sizeof(*p);
                glEnable(p->type);
                break;
            }
            case COMMAND_TYPE::DISABLE: {
                auto* p = (DisableParam*)ptr;
                current_data_ += sizeof(*p);
                glDisable(p->type);
                break;
            }
            case COMMAND_TYPE::DEPTH_MASK: {
                auto* p = (DepthMaskParam*)ptr;
                current_data_ += sizeof(*p);
                glDepthMask(p->flag);
                break;
            }
            case COMMAND_TYPE::DEPTH_FUNC: {
                auto* p = (DepthFuncParam*)ptr;
                current_data_ += sizeof(*p);
                glDepthFunc(p->func);
                break;
            }
            case COMMAND_TYPE::CULL_FACE: {
                auto* p = (CullFaceParam*)ptr;
                current_data_ += sizeof(*p);
                glCullFace(p->mode);
                break;
            }
            case COMMAND_TYPE::FRONT_FACE: {
                auto* p = (FrontFaceParam*)ptr;
                current_data_ += sizeof(*p);
                glFrontFace(p->mode);
                break;
            }
            case COMMAND_TYPE::STENCIL_OP_SEPARATE: {
                auto* p = (StencilOpSep*)ptr;
                current_data_ += sizeof(*p);
                glStencilOpSeparate(p->face, p->sfail, p->dpfail, p->dppass);
                break;
            }
            case COMMAND_TYPE::STENCIL_FUNC_SEPARATE: {
                auto* p = (StencilFuncSep*)ptr;
                current_data_ += sizeof(*p);
                glStencilFuncSeparate(p->face, p->func, p->ref, p->mask);
                break;
            }
            case COMMAND_TYPE::STENCIL_MASK: {
                auto* p = (StencilMaskParam*)ptr;
                current_data_ += sizeof(*p);
                glStencilMask(p->mask);
                break;
            }
            case COMMAND_TYPE::BLEND_FUNC_SEPARATE: {
                auto* p = (BlendFuncSep*)ptr;
                current_data_ += sizeof(*p);
                glBlendFuncSeparate(p->srcRGB, p->dstRGB, p->srcAlpha, p->dstAlpha);
                break;
            }
            case COMMAND_TYPE::BLEND_EQUATION_SEPARATE: {
                auto* p = (BlendEqSep*)ptr;
                current_data_ += sizeof(*p);
                glBlendEquationSeparate(p->modeRGB, p->modeAlpha);
                break;
            }
            case COMMAND_TYPE::MIN_SAMPLE_SHADING: {
                auto* p = (MinSampleShadingP*)ptr;
                current_data_ += sizeof(*p);
                glMinSampleShading(p->value);
                break;
            }
            case COMMAND_TYPE::VIEWPORT: {
                auto* p = (ViewportParam*)ptr;
                current_data_ += sizeof(*p);
                glViewport(p->x, p->y, p->w, p->h);
                break;
            }
            case COMMAND_TYPE::SCISSOR: {
                auto* p = (ScissorParam*)ptr;
                current_data_ += sizeof(*p);
                glScissor(p->x, p->y, p->w, p->h);
                break;
            }
            case COMMAND_TYPE::USE_PROGRAM: {
                auto* p = (UseProgramParam*)ptr;
                current_data_ += sizeof(*p);
                glUseProgram(p->program);
                break;
            }
            case COMMAND_TYPE::CLEAR_COLOR: {
                auto* p = (ClearColor*)ptr;
                current_data_ += sizeof(*p);
                glClearColor(p->r, p->g, p->b, p->a);
                break;
            }
            case COMMAND_TYPE::CLEAR: {
                auto* p = (Clear*)ptr;
                current_data_ += sizeof(*p);
                glClear(p->bits);
                break;
            }
            case COMMAND_TYPE::MEMORY_COPY: {
                auto* p = (MemoryCopy*)ptr;
                current_data_ += sizeof(*p);
                
                glBindBuffer(p->target, p->buffer);
                void* ptr = glMapBufferRange(
                    p->target,
                    p->byte_start,
                    p->byte_size,
                    GL_MAP_WRITE_BIT
                );

                assert(ptr != nullptr && "Failed to map vertex buffer!");

                std::memcpy(ptr, p->data, p->byte_size);

                glUnmapBuffer(p->target);
                break;
            }
            case COMMAND_TYPE::DEFAULT_VIEWPORT: {
                auto* p = (DefaultViewport*)ptr;
                current_data_ += sizeof(*p);

                int width, height;
                glfwGetFramebufferSize(p->window, &width, &height);
                glViewport(0, 0, width, height);
                break;
            }
            case COMMAND_TYPE::IMGUI_RENDER: {
                current_data_ += sizeof(uint8_t);
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
                break;
            }
            case COMMAND_TYPE::DISPLAY: {
                auto* p = (DisplayParam*)ptr;
                current_data_ += sizeof(*p);
                glfwSwapBuffers(p->window);
                glfwPollEvents();
                current_type_ = 0;
                current_data_ = 0;
                types_.clear();
                data_.clear();
                return; // Return once we have drawn ONE frame (Yes, there can be multiple frames in the command queue)
            }
            }
        }
        // No DISPLAY command — reset for next frame
        current_type_ = 0;
        current_data_ = 0;
    }

private:
    std::vector<COMMAND_TYPE> types_;
    std::vector<uint8_t> data_;
    size_t data_size_ = 0;

    size_t current_type_ = 0;
    size_t current_data_ = 0;

    template <typename T>
    inline void addCommand(COMMAND_TYPE type, const T& obj) {
        types_.push_back(type);

        size_t obj_size = sizeof(T);
        size_t old_size = data_.size();
        data_.resize(old_size + obj_size);

        std::memcpy(data_.data() + old_size, &obj, obj_size);
    }
};

}
