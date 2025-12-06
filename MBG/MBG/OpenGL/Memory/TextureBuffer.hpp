#pragma once

#include <glad/glad.h>

#include <glm/glm.hpp>
using namespace glm;

#include <cassert>

#include "../Info.hpp"
#include "../FrameGraph.hpp"
#include "DescriptorSetBuffer.hpp"

namespace MBG {

class TextureBuffer {
public:
    TextureBuffer(const TextureBufferParams& params) : size_(params.size) {
        // Generate buffer
        glGenBuffers(1, &buffer_id_);
        glBindBuffer(GL_TEXTURE_BUFFER, buffer_id_);

        uint format_byte_size = texture_format_byte_size[uint(params.format)];
        glBufferData(GL_TEXTURE_BUFFER, size_ * format_byte_size, params.data, GL_STATIC_DRAW);

        // Generate texture
        glGenTextures(1, &texture_id_);
        glBindTexture(GL_TEXTURE_BUFFER, texture_id_);
        assert(texture_id_ != 0);

        auto info = texture_formats[uint(params.format)];
        glTexBuffer(GL_TEXTURE_BUFFER, info.internal_format, buffer_id_);
    }

    ~TextureBuffer() {
        glDeleteTextures(1, &texture_id_);
        glDeleteBuffers(1, &buffer_id_);
    }

    // This mapped pointer is unsynchronized and must be mapped to memory the GPU is not using
    inline const void* mapInstancePtr(size_t byte_start, size_t byte_size) {
        glBindBuffer(GL_TEXTURE_BUFFER, texture_id_);
        glMapBufferRange(GL_TEXTURE_BUFFER, byte_start, byte_size,
            GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
    }

    inline const void unmapInstancePtr() {
        glBindBuffer(GL_TEXTURE_BUFFER, texture_id_);
        glUnmapBuffer(GL_TEXTURE_BUFFER);
    }

    uint getSize() const { return size_; }

protected:
    GLuint texture_id_ = 0;
    uint size_ = 0;

    friend class FrameGraph;
    friend class DescriptorSetBuffer;
private:
    GLuint buffer_id_ = 0;
};

}