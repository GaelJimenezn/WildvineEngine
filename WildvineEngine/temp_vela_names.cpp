#include <iostream>
#include "Model3D.h"
int main() {
    Model3D model("Models/Vela.fbx", ModelType::FBX);
    if (!model.load("Models/Vela.fbx")) {
        std::cout << "load failed\n";
        return 1;
    }
    const auto& meshes = model.GetMeshes();
    for (size_t i = 0; i < meshes.size(); ++i) {
        std::cout << i << ": " << meshes[i].m_name << "\n";
    }
    return 0;
}
