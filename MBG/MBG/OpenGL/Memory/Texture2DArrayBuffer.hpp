#pragma once

#include <glad/glad.h>

#include <glm/glm.hpp>
using namespace glm;

#include "../Info.hpp"
#include "../FrameGraph.hpp"
#include "DescriptorSetBuffer.hpp"

namespace MBG {

class Texture2DArrayBuffer {
public:
    Texture2DArrayBuffer(const Texture2DArrayBufferParams& params) 
        : size_(params.size), layers_(params.layers), format_(params.format)
    {
        glGenTextures(1, &texture_id_);
        glBindTexture(GL_TEXTURE_2D_ARRAY, texture_id_);
        assert(texture_id_ != 0);

        // Set texture parameters
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GLenum(params.min_filter));
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GLenum(params.mag_filter));
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GLenum(params.wrap_s));
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GLenum(params.wrap_t));

        auto info = texture_formats[uint(params.format)];

        setUnpackAlignment(params.format, params.size.x);

        // Allocate layered texture
        glTexImage3D(GL_TEXTURE_2D_ARRAY,
            0,
            info.internal_format,
            size_.x, size_.y,
            layers_,
            0,
            info.format,
            info.type,
            params.data
        );

        if (params.generate_mipmaps)
            glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
    }

    ~Texture2DArrayBuffer() {
        glDeleteTextures(1, &texture_id_);
    }

    inline void subImage(uint layer, const uvec2& start, const uvec2& size, const void* data, uint mip_level = 0) const {
        glBindTexture(GL_TEXTURE_2D_ARRAY, texture_id_);

        auto info = texture_formats[uint(format_)];

        glTexSubImage3D(
            GL_TEXTURE_2D_ARRAY,
            mip_level,
            start.x, start.y, layer,
            size.x, size.y, 1,
            info.format,
            info.type,
            data
        );
    }

    uvec2 getSize() const { return size_; }
    uint getLayerSize() const { return layers_; }

protected:
    GLuint texture_id_ = 0;

    friend class FrameGraph;
    friend class DescriptorSetBuffer;
private:
    uvec2 size_;
    uint layers_;
    TEXTURE_TYPE format_;
};

}