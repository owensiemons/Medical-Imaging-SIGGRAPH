#pragma once

#include <glad/glad.h>

#include <glm/glm.hpp>
using namespace glm;

#include <cassert>

#include "../Info.hpp"
#include "../FrameGraph.hpp"
#include "DescriptorSetBuffer.hpp"

namespace MBG {

class TextureCubeMapBuffer {
public:
    TextureCubeMapBuffer(const TextureCubeMapBufferParams& params) : size_(params.size), format_(params.format) {
        glGenTextures(1, &texture_id_);
        glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id_);
        assert(texture_id_ != 0);

        // Set texture parameters
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GLenum(params.min_filter));
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GLenum(params.mag_filter));
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GLenum(params.wrap_s));
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GLenum(params.wrap_t));
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GLenum(params.wrap_r));

        auto info = texture_formats[uint(params.format)];
    
        setUnpackAlignment(params.format, params.size);

        // Allocate and upload each cube face
        for (unsigned i = 0; i < 6; ++i) {
            glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0,                          // mip level
                info.internal_format,       // internal format
                size_, size_,               // width and height
                0,                          // border
                info.format,                // format
                info.type,                  // type
                params.data[i]              // initial data for this face
            );
        }

        if (params.generate_mipmaps)
            glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    }

    ~TextureCubeMapBuffer() {
        glDeleteTextures(1, &texture_id_);
    }
    
    inline void subImage(CUBE_FACE face, const uvec2& start, const uvec2& size, const void* data, uint mip_level = 0) const {
        glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id_);

        auto info = texture_formats[uint(format_)];

        glTexSubImage2D(
            GLenum(face),
            mip_level,
            start.x, start.y,
            size.x, size.y,
            info.format,
            info.type,
            data
        );
    }

    unsigned int getSize() const { return size_; }

protected:
    GLuint texture_id_ = 0;
    unsigned int size_;
    TEXTURE_TYPE format_;

    friend class FrameGraph;
    friend class DescriptorSetBuffer;
};

}