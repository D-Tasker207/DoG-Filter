#ifndef FILE_PATH_H
#define FILE_PATH_H

#include <string>
#include <iostream>
#include <filesystem>
#include <unordered_map>
using namespace std;

static std::filesystem::path getExtensionPath(const std::string& filePath) {
    static const std::unordered_map<std::string, std::filesystem::path> extensionMap = {
        {".glsl", "shaders"},
        {".vert", "shaders"},
        {".vs", "shaders"},
        {".geom", "shaders"},
        {".gs", "shaders"},
        {".tesc", "shaders"},
        {".tcs", "shaders"},
        {".tese", "shaders"},
        {".tes", "shaders"},
        {".frag", "shaders"},
        {".fs", "shaders"},
        {".comp", "shaders"},
        {".cs", "shaders"},
        {".obj", "models"},
        {".off", "models"},
        {".gltf", "models"},
        {".glb", "models"},
        {".fbx", "models"},
        {".dae", "models"},
        {".3ds", "models"},
        {".blend", "models"},
        {".ply", "models"},
        {".stl", "models"},
        {".x", "models"},
        {".3d", "models"},
        {".3mf", "models"},
        {".amf", "models"},
        {".tga", "textures"},
        {".png", "textures"},
        {".jpg", "textures"},
        {".jpeg", "textures"},
        {".bmp", "textures"},
        {".gif", "textures"},
        {".hdr", "textures"},
        {".dds", "textures"},
    };

    std::string extension = std::filesystem::path(filePath).extension().string();

    auto it = extensionMap.find(extension);
    if(it != extensionMap.end()){
        if(it->second != "shaders"){
            return "resources" / it->second;
        }
        return it->second;
    }
    return "";
}

inline std::string getFilePath(const std::string& fileName){
    std::filesystem::path currentFilePath = std::filesystem::absolute(std::filesystem::path(__FILE__));
    std::filesystem::path baseDir = currentFilePath.parent_path().parent_path();

    std::string extensionPath = getExtensionPath(fileName);
    if(extensionPath.empty()){
        cout << "Error :: " << std::filesystem::path(fileName).extension().string() << " Type Unsupported" << endl;
        return "";
    }

    return baseDir / extensionPath / fileName;
}

#endif