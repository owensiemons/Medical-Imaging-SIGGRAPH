#pragma once
#include <glad/glad.h>
#include "../Info.hpp"
#include "../FrameGraph.hpp"
#include "DescriptorSetBuffer.hpp"

namespace MBG {

    class ShaderStorageBuffer {
    public:
        ShaderStorageBuffer(const ShaderStorageBufferParams& params)
            : size_(params.size), binding_(params.binding) {
            assert(size_ > 0);
            glGenBuffers(1, &ssbo_id_);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_id_);
            glBufferData(GL_SHADER_STORAGE_BUFFER, params.size, params.data, GLenum(params.buffer_usage));
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding_, ssbo_id_);
        }

        inline void bind() {
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding_, ssbo_id_);
        }

        inline const void* mapPtr(size_t byte_start, size_t byte_size) {
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_id_);
            return glMapBufferRange(GL_SHADER_STORAGE_BUFFER, byte_start, byte_size,
                GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
        }

        inline const void unmapPtr() {
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_id_);
            glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
        }

        inline const void remapData(size_t write_size, void* data, GLintptr offset) {
            size_ = write_size;
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_id_);
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, write_size, data);
        }

        size_t getSize() const { return size_; }

    protected:
        GLuint ssbo_id_;
        GLuint binding_;
        size_t size_;
        friend class FrameGraph;
        friend class DescriptorSetBuffer;
    };
}