#pragma once

#include <glad/glad.h>
#include <vector>
#include <string>

std::vector<std::string> GetSortedSliceFiles(const std::string& folder);
std::vector<unsigned char> Load3DTexture(const std::string& folder, uint32_t& width, uint32_t& height, int& depth);
void Load3DTextureBinary(const std::string& folder, unsigned char*& buffer, uint32_t& width, uint32_t& height, uint32_t& depth);