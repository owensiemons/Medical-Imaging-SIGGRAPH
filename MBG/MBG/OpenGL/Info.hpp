#pragma once

#include <glad/glad.h>

#include <glm/glm.hpp>
using namespace glm;

#include <string>

namespace MBG {

//------------------------------------------------------------
// RENDERING STATES
//	These are all of the rendering states we support
//------------------------------------------------------------

enum class RENDER_TYPE {
	TRIANGLES = GL_TRIANGLES,
	TRIANGLE_STRIPS = GL_TRIANGLE_STRIP,
	LINES = GL_LINES,
	POINTS = GL_POINTS,
};

enum class DEPTH_COMPARE_OP {
	NEVER = GL_NEVER,
	LESS = GL_LESS,
	EQUAL = GL_EQUAL,
	LEQUAL = GL_LEQUAL,
	GREATER = GL_GREATER,
	NOTEQUAL = GL_NOTEQUAL,
	GEQUAL = GL_GEQUAL,
	ALWAYS = GL_ALWAYS,
};
enum class CULL_MODE {
	CULL_NONE = 0,
	CULL_FRONT = GL_FRONT,
	CULL_BACK = GL_BACK,
	CULL_FRONT_AND_BACK = GL_FRONT_AND_BACK
};

enum class FRONT_FACE {
	CCW = GL_CCW,
	CW = GL_CW
};

enum class STENCIL_OP {
	KEEP = GL_KEEP,
	ZERO = GL_ZERO,
	REPLACE = GL_REPLACE,
	INCR = GL_INCR,
	INCR_WRAP = GL_INCR_WRAP,
	DECR = GL_DECR,
	DECR_WRAP = GL_DECR_WRAP,
	INVERT = GL_INVERT
};

enum class BLEND_FACTOR {
	ZERO = GL_ZERO,
	ONE = GL_ONE,
	SRC_COLOR = GL_SRC_COLOR,
	ONE_MINUS_SRC_COLOR = GL_ONE_MINUS_SRC_COLOR,
	DST_COLOR = GL_DST_COLOR,
	ONE_MINUS_DST_COLOR = GL_ONE_MINUS_DST_COLOR,
	SRC_ALPHA = GL_SRC_ALPHA,
	ONE_MINUS_SRC_ALPHA = GL_ONE_MINUS_SRC_ALPHA,
	DST_ALPHA = GL_DST_ALPHA,
	ONE_MINUS_DST_ALPHA = GL_ONE_MINUS_DST_ALPHA
};

enum class BLEND_OP {
	ADD = GL_FUNC_ADD,
	SUBTRACT = GL_FUNC_SUBTRACT,
	REVERSE_SUBTRACT = GL_FUNC_REVERSE_SUBTRACT,
	MIN_OP = GL_MIN,
	MAX_OP = GL_MAX
};

struct RenderStates {
	// Primitive type
	RENDER_TYPE render_type = RENDER_TYPE::TRIANGLES;

	// Depth
	bool depth_test = false;
	bool depth_writes = false;
	DEPTH_COMPARE_OP depth_compare = DEPTH_COMPARE_OP::LEQUAL;

	// Culling
	CULL_MODE cull_mode = CULL_MODE::CULL_NONE;
	FRONT_FACE front_face = FRONT_FACE::CCW;

	// Stencil
	bool stencil_test = false;
	STENCIL_OP stencil_fail_op = STENCIL_OP::KEEP;
	STENCIL_OP stencil_depth_fail_op = STENCIL_OP::KEEP;
	STENCIL_OP stencil_pass_op = STENCIL_OP:: KEEP;
	DEPTH_COMPARE_OP stencil_compare_op = DEPTH_COMPARE_OP::ALWAYS;
	int stencil_ref = 0;
	unsigned stencil_read_mask = 0xFF;
	unsigned stencil_write_mask = 0xFF;

	// Blending
	bool enable_blend = false;
	BLEND_FACTOR src_color = BLEND_FACTOR::ONE;
	BLEND_FACTOR dst_color = BLEND_FACTOR::ZERO;
	BLEND_FACTOR src_alpha = BLEND_FACTOR::ONE;
	BLEND_FACTOR dst_alpha = BLEND_FACTOR::ZERO;
	BLEND_OP color_op = BLEND_OP::ADD;
	BLEND_OP alpha_op = BLEND_OP::ADD;

	// Multisampling
	bool multisample_enabled = false;
	bool sample_shading = false;
	float min_sample_shading = 1.0f;

	// Viewport + Scissor
	bool enable_viewport = false;
	uvec2 viewport_start = uvec2(0);
	uvec2 viewport_size = uvec2(0);
	bool enable_scissor = false;
	uvec2 scissor_start = uvec2(0);
	uvec2 scissor_size = uvec2(0);
};

//------------------------------------------------------------
// DESCRIPTOR
//	These are all of the assets needed for the shader and should be bound
//------------------------------------------------------------

enum class DESCRIPTOR_TYPE {
	VERTEX_BUFFER_IN,
	VERTEX_BUFFER_OUT,

	ELEMENT_BUFFER_IN,
	ELEMENT_BUFFER_OUT,

	INSTANCED_VERTEX_BUFFER_IN,
	INSTANCED_VERTEX_BUFFER_OUT,

	INSTANCED_ELEMENT_BUFFER_IN,
	INSTANCED_ELEMENT_BUFFER_OUT,

	TEXTURE_BUFFER_IN,
	TEXTURE_BUFFER_OUT,

	TEXTURE_2D_BUFFER_IN,
	TEXTURE_2D_BUFFER_OUT,

	TEXTURE_3D_BUFFER_IN,
	TEXTURE_3D_BUFFER_OUT,

	TEXTURE_CUBE_MAP_BUFFER_IN,
	TEXTURE_CUBE_MAP_BUFFER_OUT,

	TEXTURE_ARRAY_BUFFER_IN,
	TEXTURE_ARRAY_BUFFER_OUT,

	UNIFORM_BUFFER,
};

struct VertexView {
	uint start;
	uint size;
};

struct ElementView {
	uint start;
	uint size;
};

struct InstancedVertexView {
	uint start;
	uint size;
};

struct InstancedElementView {
	uint start;
	uint size;
};

struct TextureCubeMapView {
	uint face; // In openGL's assending directions
	uint mip_level;
};

struct Texture2DArrayView {
	uint slice;
	uint mip_level;
};

struct Texture2DView {
	uint mip_level;
};

struct Texture3DView {
	uint slice;
	uint mip_level;
};

struct UniformView {
	uint start;
	uint size;
};

struct Descriptor {
	DESCRIPTOR_TYPE type = DESCRIPTOR_TYPE::UNIFORM_BUFFER;		// Type of the descriptor
	void* data = nullptr;										// Refrence to the object
	void* view = nullptr;										// Set the view the GPU sees/uese [nullptr is default]
};

struct DescriptorSetBufferParams {
	Descriptor* descriptors = nullptr;
	uint count = 0;
};

//------------------------------------------------------------
// TEXTURES
//------------------------------------------------------------

enum class TEXTURE_WRAP {
	CLAMP_TO_EDGE = GL_CLAMP_TO_EDGE,
	CLAMP_TO_BORDER = GL_CLAMP_TO_BORDER,
	MIRRORED_REPEAT = GL_MIRRORED_REPEAT,
	REPEAT = GL_REPEAT,
	MIRROR_CLAMP_TO_EDGE = GL_MIRROR_CLAMP_TO_EDGE
};

enum class TEXTURE_FILTER {
	NEAREST = GL_NEAREST,
	LINEAR = GL_LINEAR,
	NEAREST_MIPMAP_NEAREST = GL_NEAREST_MIPMAP_NEAREST,
	LINEAR_MIPMAP_NEAREST = GL_LINEAR_MIPMAP_NEAREST,
	NEAREST_MIPMAP_LINEAR = GL_NEAREST_MIPMAP_LINEAR,
	LINEAR_MIPMAP_LINEAR = GL_LINEAR_MIPMAP_LINEAR,
};

// These are all of the texture color types which are exactly the same as the opengl types
enum class TEXTURE_TYPE {
	R8,
	R8_SNORM,
	R16,
	R16_SNORM,
	RG8_SNORM,
	RG16,
	RG16_SNORM,
	R3_G3_B2,
	RGB4,
	RGB5,
	RGB8,
	RGB8_SNORM,
	RGB10,
	RGB12,
	RGB16_SNORM,
	RGBA2,
	RGBA4,
	RGB5_A1,
	RGBA8,
	RGBA8_SNORM,
	RGB10_A2,
	RGB10_A2UI,
	RGBA12,
	RGBA16,
	SRGB8,
	SRGB8_ALPHA8,
	R16F,
	RG16F,
	RGB16F,
	RGBA16F,
	R32F,
	RG32F,
	RGB32F,
	RGBA32F,
	R11F_G11F_B10F,
	RGB9_E5,
	R8I,
	R8UI,
	R16I,
	R16UI,
	R32I,
	R32UI,
	RG8I,
	RG8UI,
	RG16I,
	RG16UI,
	RG32I,
	RG32UI,
	RGB8I,
	RGB8UI,
	RGB16I,
	RGB16UI,
	RGB32I,
	RGB32UI,
	RGBA8I,
	RGBA8UI,
	RGBA16I,
	RGBA16UI,
	RGBA32I,
	RGBA32UI
};

struct TextureFormatInfo {
	GLint internal_format;
	GLenum format;
	GLenum type;
};

// This directly gives me the internal format, base format and default type
constexpr TextureFormatInfo texture_formats[] = {
	{GL_R8, GL_RED, GL_UNSIGNED_BYTE},
	{GL_R8_SNORM, GL_RED, GL_BYTE},
	{GL_R16, GL_RED, GL_UNSIGNED_SHORT},
	{GL_R16_SNORM, GL_RED, GL_SHORT},
	{GL_RG8_SNORM, GL_RG, GL_BYTE},
	{GL_RG16, GL_RG, GL_UNSIGNED_SHORT},
	{GL_RG16_SNORM, GL_RG, GL_SHORT},
	{GL_R3_G3_B2, GL_RGB, GL_UNSIGNED_BYTE},
	{GL_RGB4, GL_RGB, GL_UNSIGNED_BYTE},
	{GL_RGB5, GL_RGB, GL_UNSIGNED_SHORT},
	{GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE},
	{GL_RGB8_SNORM, GL_RGB, GL_BYTE},
	{GL_RGB10, GL_RGB, GL_UNSIGNED_INT},
	{GL_RGB12, GL_RGB, GL_UNSIGNED_INT},
	{GL_RGB16_SNORM, GL_RGB, GL_SHORT},
	{GL_RGBA2, GL_RGBA, GL_UNSIGNED_BYTE},
	{GL_RGBA4, GL_RGBA, GL_UNSIGNED_SHORT},
	{GL_RGB5_A1, GL_RGBA, GL_UNSIGNED_SHORT},
	{GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE},
	{GL_RGBA8_SNORM, GL_RGBA, GL_BYTE},
	{GL_RGB10_A2, GL_RGBA, GL_UNSIGNED_INT},
	{GL_RGB10_A2UI, GL_RGBA, GL_UNSIGNED_INT},
	{GL_RGBA12, GL_RGBA, GL_UNSIGNED_INT},
	{GL_RGBA16, GL_RGBA, GL_UNSIGNED_SHORT},
	{GL_SRGB8, GL_RGB, GL_UNSIGNED_BYTE},
	{GL_SRGB8_ALPHA8, GL_RGBA, GL_UNSIGNED_BYTE},
	{GL_R16F, GL_RED, GL_HALF_FLOAT},
	{GL_RG16F, GL_RG, GL_HALF_FLOAT},
	{GL_RGB16F, GL_RGB, GL_HALF_FLOAT},
	{GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT},
	{GL_R32F, GL_RED, GL_FLOAT},
	{GL_RG32F, GL_RG, GL_FLOAT},
	{GL_RGB32F, GL_RGB, GL_FLOAT},
	{GL_RGBA32F, GL_RGBA, GL_FLOAT},
	{GL_R11F_G11F_B10F, GL_RGB, GL_UNSIGNED_INT},
	{GL_RGB9_E5, GL_RGB, GL_UNSIGNED_INT},
	{GL_R8I, GL_RED, GL_BYTE},
	{GL_R8UI, GL_RED, GL_UNSIGNED_BYTE},
	{GL_R16I, GL_RED, GL_SHORT},
	{GL_R16UI, GL_RED, GL_UNSIGNED_SHORT},
	{GL_R32I, GL_RED, GL_INT},
	{GL_R32UI, GL_RED, GL_UNSIGNED_INT},
	{GL_RG8I, GL_RG, GL_BYTE},
	{GL_RG8UI, GL_RG, GL_UNSIGNED_BYTE},
	{GL_RG16I, GL_RG, GL_SHORT},
	{GL_RG16UI, GL_RG, GL_UNSIGNED_SHORT},
	{GL_RG32I, GL_RG, GL_INT},
	{GL_RG32UI, GL_RG, GL_UNSIGNED_INT},
	{GL_RGB8I, GL_RGB, GL_BYTE},
	{GL_RGB8UI, GL_RGB, GL_UNSIGNED_BYTE},
	{GL_RGB16I, GL_RGB, GL_SHORT},
	{GL_RGB16UI, GL_RGB, GL_UNSIGNED_SHORT},
	{GL_RGB32I, GL_RGB, GL_INT},
	{GL_RGB32UI, GL_RGB, GL_UNSIGNED_INT},
	{GL_RGBA8I, GL_RGBA, GL_BYTE},
	{GL_RGBA8UI, GL_RGBA, GL_UNSIGNED_BYTE},
	{GL_RGBA16I, GL_RGBA, GL_SHORT},
	{GL_RGBA16UI, GL_RGBA, GL_UNSIGNED_SHORT},
	{GL_RGBA32I, GL_RGBA, GL_INT},
	{GL_RGBA32UI, GL_RGBA, GL_UNSIGNED_INT},
};

constexpr uint texture_format_byte_size[] = {
	1,  // R8
	1,  // R8_SNORM
	2,  // R16
	2,  // R16_SNORM
	2,  // RG8_SNORM
	4,  // RG16
	4,  // RG16_SNORM
	1,  // R3_G3_B2
	2,  // RGB4
	2,  // RGB5
	3,  // RGB8
	3,  // RGB8_SNORM
	4,  // RGB10
	4,  // RGB12
	6,  // RGB16_SNORM
	1,  // RGBA2
	2,  // RGBA4
	2,  // RGB5_A1
	4,  // RGBA8
	4,  // RGBA8_SNORM
	4,  // RGB10_A2
	4,  // RGB10_A2UI
	6,  // RGBA12
	8,  // RGBA16
	3,  // SRGB8
	4,  // SRGB8_ALPHA8
	2,  // R16F
	4,  // RG16F
	6,  // RGB16F
	8,  // RGBA16F
	4,  // R32F
	8,  // RG32F
	12, // RGB32F
	16, // RGBA32F
	4,  // R11F_G11F_B10F
	4,  // RGB9_E5
	1,  // R8I
	1,  // R8UI
	2,  // R16I
	2,  // R16UI
	4,  // R32I
	4,  // R32UI
	2,  // RG8I
	2,  // RG8UI
	4,  // RG16I
	4,  // RG16UI
	8,  // RG32I
	8,  // RG32UI
	3,  // RGB8I
	3,  // RGB8UI
	6,  // RGB16I
	6,  // RGB16UI
	12, // RGB32I
	12, // RGB32UI
	4,  // RGBA8I
	4,  // RGBA8UI
	8,  // RGBA16I
	8,  // RGBA16UI
	16, // RGBA32I
	16  // RGBA32UI
};

inline void setUnpackAlignment(const TEXTURE_TYPE format, const uint width) {
	size_t bytesPerPixel = texture_format_byte_size[uint(format)];
	size_t rowSize = width * bytesPerPixel;

	// OpenGL unpack alignment must be 1, 2, 4, or 8
	if (rowSize % 8 == 0)        glPixelStorei(GL_UNPACK_ALIGNMENT, 8);
	else if (rowSize % 4 == 0)   glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	else if (rowSize % 2 == 0)   glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
	else                         glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}

//------------------------------------------------------------
// TEXTURE BUFFER
//------------------------------------------------------------

struct TextureBufferParams {
	uint size = 0;								// Number of elements
	TEXTURE_TYPE format = TEXTURE_TYPE::R32F;  // default format
	void* data = nullptr;						// Initial data
};

//------------------------------------------------------------
// TEXTURE CUBE MAP
//------------------------------------------------------------

enum class CUBE_FACE {
	POSITIVE_X = GL_TEXTURE_CUBE_MAP_POSITIVE_X,
	NEGATIVE_X = GL_TEXTURE_CUBE_MAP_NEGATIVE_X,

	POSITIVE_Y = GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
	NEGATIVE_Y = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,

	POSITIVE_Z = GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
	NEGATIVE_Z = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
};

struct TextureCubeMapBufferParams {
	// Size of each face (cube is always square)
	unsigned int size = 0;
	TEXTURE_TYPE format = TEXTURE_TYPE::RGBA8;

	// Mip Maps
	bool generate_mipmaps = false;
	TEXTURE_FILTER min_filter = TEXTURE_FILTER::LINEAR;
	TEXTURE_FILTER mag_filter = TEXTURE_FILTER::LINEAR;

	// Wraps (cube maps usually use CLAMP_TO_EDGE)
	TEXTURE_WRAP wrap_s = TEXTURE_WRAP::CLAMP_TO_EDGE;
	TEXTURE_WRAP wrap_t = TEXTURE_WRAP::CLAMP_TO_EDGE;
	TEXTURE_WRAP wrap_r = TEXTURE_WRAP::CLAMP_TO_EDGE;

	// Initial data for 6 faces, ordered +X, -X, +Y, -Y, +Z, -Z
	void* data[6] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
};

//------------------------------------------------------------
// TEXTURE ARRAY BUFFER
//------------------------------------------------------------

struct Texture2DArrayBufferParams {
	// Actual texture parameters
	uvec2 size = uvec2(0);						// Width and height of each layer
	uint layers = 0;							// Number of layers
	TEXTURE_TYPE format = TEXTURE_TYPE::RGBA8;

	// Mip Maps
	bool generate_mipmaps = false;
	TEXTURE_FILTER min_filter = TEXTURE_FILTER::LINEAR;
	TEXTURE_FILTER mag_filter = TEXTURE_FILTER::LINEAR;

	// Wraps
	TEXTURE_WRAP wrap_s = TEXTURE_WRAP::REPEAT;
	TEXTURE_WRAP wrap_t = TEXTURE_WRAP::REPEAT;

	void* data = nullptr;						// Must contain all layers consecutively
};

//------------------------------------------------------------
// TEXTURE 2D
//------------------------------------------------------------

struct Texture2DBufferParams {
	// Actual texture parameters (size, type, etc.)
	uvec2 size = uvec2(0);
	TEXTURE_TYPE format = TEXTURE_TYPE::RGBA8;

	// Mip Maps
	bool generate_mipmaps = false;
	TEXTURE_FILTER min_filter = TEXTURE_FILTER::LINEAR;
	TEXTURE_FILTER mag_filter = TEXTURE_FILTER::LINEAR;

	// Waps
	TEXTURE_WRAP wrap_s = TEXTURE_WRAP::REPEAT;
	TEXTURE_WRAP wrap_t = TEXTURE_WRAP::REPEAT;

	void* data = nullptr;
};

//------------------------------------------------------------
// TEXTURE 3D
//------------------------------------------------------------

struct Texture3DBufferParams {
	// Actual texture parameters (size, type, etc.)
	uvec3 size = uvec3(0);
	TEXTURE_TYPE format = TEXTURE_TYPE::RGBA8;

	// Mip Maps
	bool generate_mipmaps = false;
	TEXTURE_FILTER min_filter = TEXTURE_FILTER::LINEAR;
	TEXTURE_FILTER mag_filter = TEXTURE_FILTER::LINEAR;

	// Waps
	TEXTURE_WRAP wrap_s = TEXTURE_WRAP::REPEAT;
	TEXTURE_WRAP wrap_t = TEXTURE_WRAP::REPEAT;
	TEXTURE_WRAP wrap_r = TEXTURE_WRAP::REPEAT;

	void* data = nullptr;
};

//------------------------------------------------------------
// VERTEX ARRAY OBJECTS
//------------------------------------------------------------

enum class ATTRIBUTE {
	BYTE = GL_BYTE,
	UNSIGNED_BYTE = GL_UNSIGNED_BYTE,
	SHORT = GL_SHORT,
	UNSIGNED_SHORT = GL_UNSIGNED_SHORT,
	INT = GL_INT,
	UNSIGNED_INT = GL_UNSIGNED_INT,
	HALF_FLOAT = GL_HALF_FLOAT,
	FLOAT = GL_FLOAT,
	INT_2_10_10_10_REV = GL_INT_2_10_10_10_REV,
	UNSIGNED_INT_2_10_10_10_REV = GL_UNSIGNED_INT_2_10_10_10_REV
};

enum class BUFFER_USAGE {
	STATIC_DRAW = GL_STATIC_DRAW,
	DYNAMIC_DRAW = GL_DYNAMIC_DRAW,
	STREAM_DRAW = GL_STREAM_DRAW,
	STATIC_READ = GL_STATIC_READ,
	DYNAMIC_READ = GL_DYNAMIC_READ,
	STREAM_READ = GL_STREAM_READ,
	STATIC_COPY = GL_STATIC_COPY,
	DYNAMIC_COPY = GL_DYNAMIC_COPY,
	STREAM_COPY = GL_STREAM_COPY
};

enum class ELEMENT_ATTRUBUTE {
	BYTE = GL_UNSIGNED_BYTE,
	SHORT = GL_UNSIGNED_SHORT,
	UINT32 = GL_UNSIGNED_INT,
};

struct Attributes {
	ATTRIBUTE attribute = ATTRIBUTE::FLOAT;	// Attribute of the vertex
	int count = 1;							// Number of attribute (vec4 would be 4, ivec2 would be 2)
};

// We need to return the byte size of the attribute
constexpr unsigned getAttributeSize(ATTRIBUTE type) {
	switch (type) {
	case ATTRIBUTE::BYTE: return 1;
	case ATTRIBUTE::UNSIGNED_BYTE: return 1;
	case ATTRIBUTE::SHORT: return 2;
	case ATTRIBUTE::UNSIGNED_SHORT: return 2;
	case ATTRIBUTE::INT: return 4;
	case ATTRIBUTE::UNSIGNED_INT: return 4;
	case ATTRIBUTE::HALF_FLOAT: return 2;
	case ATTRIBUTE::FLOAT: return 4;
	case ATTRIBUTE::INT_2_10_10_10_REV: return 4;
	case ATTRIBUTE::UNSIGNED_INT_2_10_10_10_REV: return 4;
	default: return 0;
	}
}

constexpr bool isFloat(ATTRIBUTE type) {
	switch (type) {
	case ATTRIBUTE::HALF_FLOAT:
	case ATTRIBUTE::FLOAT:
		return true;

	case ATTRIBUTE::BYTE:
	case ATTRIBUTE::UNSIGNED_BYTE:
	case ATTRIBUTE::SHORT:
	case ATTRIBUTE::UNSIGNED_SHORT:
	case ATTRIBUTE::INT:
	case ATTRIBUTE::UNSIGNED_INT:
	case ATTRIBUTE::INT_2_10_10_10_REV:
	case ATTRIBUTE::UNSIGNED_INT_2_10_10_10_REV:
	default:
		return false;
	}
}

//------------------------------------------------------------
// VERTEX BUFFER
//------------------------------------------------------------

struct VertexBufferParams {
	Attributes* attributes = nullptr;							// Describes each vertex
	uint attributes_count = 0;									// Number of Attributes
	void* data = nullptr;										// Pointer to the data to upload
	uint count = 0;												// Number of vertices
	BUFFER_USAGE buffer_usage = BUFFER_USAGE::STATIC_DRAW;		// Buffer usage
};

//------------------------------------------------------------
// ELEMENT BUFFER
//------------------------------------------------------------

struct ElementBufferParams {
	Attributes* vertex_attributes = nullptr;							// Describes each vertex
	uint attributes_count = 0;											// Number of Attributes
	void* vertex_data = nullptr;										// Pointer to the vertex data
	uint vertex_count = 0;												// Number of vertices
	ELEMENT_ATTRUBUTE element_attribute = ELEMENT_ATTRUBUTE::UINT32;	// Describes each element
	void* element_data = nullptr;										// Pointer to the element data
	uint element_count = 0;												// Number of elements
	BUFFER_USAGE buffer_usage = BUFFER_USAGE::STATIC_DRAW;				// Buffer usage
};

//------------------------------------------------------------
// INSTANCED VERTEX BUFFER
//------------------------------------------------------------

struct InstancedVertexBufferParams {
	Attributes* vertex_attributes = nullptr;
	uint attributes_count = 0;
	Attributes* instance_attributes = nullptr;	// Attributes of the instanced data
	uint instance_attributes_count = 0;			// Number of attributes
	void* vertex_data = nullptr;
	uint vertex_count = 0;
	void* instance_data = nullptr;
	uint instance_count = 0;
	BUFFER_USAGE buffer_usage = BUFFER_USAGE::STATIC_DRAW;
};

//------------------------------------------------------------
// INSTANCED ELEMENT BUFFER
//------------------------------------------------------------

struct InstancedElementBufferParams {
	Attributes* vertex_attributes = nullptr;		// Vertex data
	uint attributes_count = 0;
	void* vertex_data = nullptr;
	uint vertex_count = 0;
	Attributes* instance_attributes = nullptr;		// Instance data
	uint instance_attributes_count = 0;
	void* instance_data = nullptr;
	uint instance_count = 0;
	ELEMENT_ATTRUBUTE element_attribute = ELEMENT_ATTRUBUTE::UINT32;	// Element data
	void* element_data = nullptr;
	uint element_count = 0;
	BUFFER_USAGE buffer_usage = BUFFER_USAGE::STATIC_DRAW;				// Buffer usage
};

//------------------------------------------------------------
// UNIFORM BUFFER OBJECT
//------------------------------------------------------------

struct UniformBufferParams {
	void* data = nullptr;
	uint size = 0;
	BUFFER_USAGE buffer_usage = BUFFER_USAGE::STREAM_DRAW;
};

//------------------------------------------------------------
// INDIRECT DRAW BUFFER
//------------------------------------------------------------

struct IndirectDrawBufferParams {
	int* start;		// An array of first vertex/element
	int* size;		// An array of sizes of vertex/element
	uint count;		// Number of elements in the arrays
};

// TODO: we need to add lamdba expressions for both read and write operations

// TODO: we need a multi draaw node

// TODO: add a way to render with IMGUI

// TODO: add f11 for full screen mode!

// TODO: add reading to textures

}