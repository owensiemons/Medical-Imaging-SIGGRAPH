# CMakeList.txt : CMake project for MBG, include source and define
# project specific logic here.
#

find_package(Catch2 CONFIG REQUIRED)

# Add source to this project's executable.
add_executable (MBG
    "MBG/OpenGL/Memory/DescriptorSetBuffer.hpp"
    "MBG/OpenGL/Memory/ElementBuffer.hpp"
    "MBG/OpenGL/Memory/IndirectDrawBuffer.hpp"
    "MBG/OpenGL/Memory/InstancedElementBuffer.hpp"
    "MBG/OpenGL/Memory/TextureBuffer.hpp"
    "MBG/OpenGL/Memory/TextureCubeMapBuffer.hpp"
    "MBG/OpenGL/Memory/Texture2DArrayBuffer.hpp"
    "MBG/OpenGL/Memory/Texture2DBuffer.hpp"
    "MBG/OpenGL/Memory/Texture3DBuffer.hpp"
    "MBG/OpenGL/Memory/UniformBuffer.hpp"
    "MBG/OpenGL/Memory/ShaderStorageBuffer.hpp"
    "MBG/OpenGL/Memory/VertexBuffer.hpp"
    "MBG/OpenGL/CommandBuffer.hpp"
    "MBG/OpenGL/FrameGraph.hpp"
    "MBG/OpenGL/FrameGraph.cpp"
    "MBG/OpenGL/Info.hpp"
    "MBG/OpenGL/MBG.hpp"
    "MBG/OpenGL/RenderPass.hpp"
    "MBG/OpenGL/RenderPass.cpp"
    "MBG/OpenGL/Window.hpp"
    "MBG/OpenGL/Window.cpp"

    "src/main.cpp"
    "src/texgen.hpp"
    "src/texgen.cpp"
    "src/camera.hpp"
    "src/camera.cpp"
    "src/callback_funcs.hpp"
    "src/callback_funcs.cpp"
    "src/common.hpp"
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET MBG PROPERTY CXX_STANDARD 20)
endif()

# If we are in debug mode declate that
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(_DEBUG)
endif()

find_package(imgui REQUIRED)
target_link_libraries(MBG PRIVATE imgui::imgui)

find_package(implot REQUIRED)
target_link_libraries(MBG PRIVATE implot::implot)

find_package (glfw3 REQUIRED)
target_link_libraries (MBG PRIVATE glfw)

find_package (glm REQUIRED)
target_link_libraries (MBG PRIVATE glm::glm)

find_package(glad REQUIRED)
target_link_libraries(MBG PRIVATE glad::glad)

find_package(stb REQUIRED)
target_include_directories(MBG PRIVATE ${Stb_INCLUDE_DIR})

find_package(TIFF REQUIRED)
target_link_libraries(MBG PRIVATE TIFF::TIFF)



add_custom_target(copy_folders ALL
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_LIST_DIR}/copy_folders.cmake
)
add_dependencies(MBG copy_folders)

add_subdirectory(third_party/imgui_gradient)
get_target_property(IMGUI_INCLUDE imgui::imgui INTERFACE_INCLUDE_DIRECTORIES)
target_include_directories(imgui_gradient SYSTEM PRIVATE ${IMGUI_INCLUDE})
target_link_libraries(MBG PRIVATE imgui_gradient::imgui_gradient)
target_compile_definitions(imgui_gradient PUBLIC IMGUI_DEFINE_MATH_OPERATORS)