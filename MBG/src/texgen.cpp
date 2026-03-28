#include "texgen.hpp"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <filesystem>
#include <iostream>
#include <algorithm>
#include <tiffio.h>

std::vector<std::string> GetSortedSliceFiles(const std::string& folder) {
    std::vector<std::string> images;
    for (const auto& entry : std::filesystem::directory_iterator(folder)) {
        if (entry.is_regular_file()) {
            images.push_back(entry.path().string());
        }
    }
    std::sort(images.begin(), images.end());
    return images;
}

std::vector<unsigned char> Load3DTexture(const std::string& folder, uint32_t& width, uint32_t& height, int& depth) {
    std::vector<unsigned char> empty;
    auto files = GetSortedSliceFiles(folder);
    if (files.empty()) return empty;

    // Open first TIFF
    TIFF* tif = TIFFOpen(files[0].c_str(), "r");
    if (!tif) {
        std::cout << "Failed to open TIFF" << std::endl;
        return empty;
    }

    TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
    TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
    TIFFClose(tif);

    depth = files.size();
    std::vector<unsigned char> volumeData(width * height * depth);

    std::cout << "Loading " << width << "x" << height << "x" << depth << "..." << std::endl;

    for (int z = 0; z < depth; z++) {
        TIFF* slice_tif = TIFFOpen(files[z].c_str(), "r");
        if (!slice_tif) continue;

        // Read scanlines
        for (uint32_t row = 0; row < height; row++) {
            uint32_t flipped_row = height - 1 - row;
            TIFFReadScanline(slice_tif, &volumeData[z * width * height + flipped_row * width], row);
        }

        TIFFClose(slice_tif);

        if (z % 100 == 0) {
            std::cout << "Loaded " << z << "/" << depth << std::endl;
        }
    }

    return volumeData;
}