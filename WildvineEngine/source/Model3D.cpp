#include "Model3D.h"

bool
Model3D::load(const std::string& path) {
    SetPath(path);
    SetState(ResourceState::Loading);

    init();

    bool success = true; // Cambia esto según el resultado real.

    SetState(success ? ResourceState::Loaded : ResourceState::Failed);
    return success;
}

bool Model3D::init()
{
    // Inicializar recursos GPU, buffers, etc.
    LoadFBXModel(m_filePath);
    return false;
}

void Model3D::unload()
{
    // Liberar buffers, memoria en CPU/GPU, etc.
    SetState(ResourceState::Unloaded);
}

size_t Model3D::getSizeInBytes() const
{
    return 0;
}

bool
Model3D::InitializeFBXManager() {
    // Initialize the FBX SDK manager
    lSdkManager = FbxManager::Create();
    if (!lSdkManager) {
        ERROR("ModelLoader", "FbxManager::Create()", "Unable to create FBX Manager!");
        return false;
    }
    else {
        MESSAGE("ModelLoader", "ModelLoader", "Autodesk FBX SDK version " << lSdkManager->GetVersion())
    }

    // Create an IOSettings object
    FbxIOSettings* ios = FbxIOSettings::Create(lSdkManager, IOSROOT);
    lSdkManager->SetIOSettings(ios);

    // Create an FBX Scene
    lScene = FbxScene::Create(lSdkManager, "MyScene");
    if (!lScene) {
        ERROR("ModelLoader", "FbxScene::Create()", "Unable to create FBX Scene!");
        return false;
    }
    else {
        MESSAGE("ModelLoader", "ModelLoader", "FBX Scene created successfully.")
    }
    return true;
}

std::vector<MeshComponent>
Model3D::LoadFBXModel(const std::string& filePath) {
    // 01. Initialize the SDK from FBX Manager
    if (InitializeFBXManager()) {
        // 02. Create an importer using the SDK manager
        FbxImporter* lImporter = FbxImporter::Create(lSdkManager, "");
        if (!lImporter) {
            ERROR("ModelLoader", "FbxImporter::Create()", "Unable to create FBX Importer!");
            return std::vector<MeshComponent>();
        }
        else {
            MESSAGE("ModelLoader", "ModelLoader", "FBX Importer created successfully.");
        }

        // 03. Use the first argument as the filename for the importer
        if (!lImporter->Initialize(filePath.c_str(), -1, lSdkManager->GetIOSettings())) {
            ERROR("ModelLoader", "FbxImporter::Initialize()",
                "Unable to initialize FBX Importer! Error: " << lImporter->GetStatus().GetErrorString());
            lImporter->Destroy();
            return std::vector<MeshComponent>();
        }
        else {
            MESSAGE("ModelLoader", "ModelLoader", "FBX Importer initialized successfully.");
        }

        // 04. Import the scene from the file into the scene
        if (!lImporter->Import(lScene)) {
            ERROR("ModelLoader", "FbxImporter::Import()",
                "Unable to import FBX Scene! Error: " << lImporter->GetStatus().GetErrorString());
            lImporter->Destroy();
            return std::vector<MeshComponent>();
        }
        else {
            MESSAGE("ModelLoader", "ModelLoader", "FBX Scene imported successfully.");
            m_name = lImporter->GetFileName();
        }

        FbxAxisSystem::DirectX.ConvertScene(lScene);
        FbxSystemUnit::m.ConvertScene(lScene);
        FbxGeometryConverter gc(lSdkManager);
        gc.Triangulate(lScene, /*replace*/ true);

        // 05. Destroy the importer
        lImporter->Destroy();
        MESSAGE("ModelLoader", "ModelLoader", "FBX Importer destroyed successfully.");

        // 06. Process the model from the scene
        FbxNode* lRootNode = lScene->GetRootNode();

        if (lRootNode) {
            MESSAGE("ModelLoader", "ModelLoader", "Processing model from the scene root node.");
            for (int i = 0; i < lRootNode->GetChildCount(); i++) {
                ProcessFBXNode(lRootNode->GetChild(i));
            }
            return m_meshes;
        }
        else {
            ERROR("ModelLoader", "FbxScene::GetRootNode()",
                "Unable to get root node from FBX Scene!");
            return std::vector<MeshComponent>();
        }
    }
    return m_meshes;
}

void
Model3D::ProcessFBXNode(FbxNode* node) {
    // 01. Process all the node's meshes
    if (node->GetNodeAttribute()) {
        if (node->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eMesh) {
            ProcessFBXMesh(node);
        }
    }

    // 02. Recursively process each child node
    for (int i = 0; i < node->GetChildCount(); i++) {
        ProcessFBXNode(node->GetChild(i));
    }
}

void
Model3D::ProcessFBXMesh(FbxNode* node) {
    FbxMesh* mesh = node->GetMesh();
    if (!mesh) return;

    // --- Asegura normales/tangentes en el FBX ---
    if (mesh->GetElementNormalCount() == 0)
        mesh->GenerateNormals(true, true);

    const char* uvSetName = nullptr;
    {
        FbxStringList uvSets; mesh->GetUVSetNames(uvSets);
        if (uvSets.GetCount() > 0) uvSetName = uvSets[0];
    }

    if (mesh->GetElementTangentCount() == 0 && uvSetName)
        mesh->GenerateTangentsData(uvSetName);

    const FbxGeometryElementUV* uvElem = (mesh->GetElementUVCount() > 0) ? mesh->GetElementUV(0) : nullptr;
    const FbxGeometryElementTangent* tanElem = (mesh->GetElementTangentCount() > 0) ? mesh->GetElementTangent(0) : nullptr;
    const FbxGeometryElementBinormal* binElem = (mesh->GetElementBinormalCount() > 0) ? mesh->GetElementBinormal(0) : nullptr;

    std::vector<SimpleVertex>       vertices;
    std::vector<unsigned int> indices;
    vertices.reserve(mesh->GetPolygonCount() * 3);
    indices.reserve(mesh->GetPolygonCount() * 3);

    // Helpers de lectura (control point vs. polygon-vertex)
    auto readV2 = [](const FbxGeometryElementUV* elem, int cpIdx, int pvIdx) -> FbxVector2 {
        if (!elem) return FbxVector2(0, 0);
        using E = FbxGeometryElement;
        int idx;
        if (elem->GetMappingMode() == E::eByControlPoint)
            idx = (elem->GetReferenceMode() == E::eIndexToDirect) ? elem->GetIndexArray().GetAt(cpIdx) : cpIdx;
        else
            idx = (elem->GetReferenceMode() == E::eIndexToDirect) ? elem->GetIndexArray().GetAt(pvIdx) : pvIdx;
        return elem->GetDirectArray().GetAt(idx);
        };
    auto readV4 = [](auto* elem, int cpIdx, int pvIdx) -> FbxVector4 {
        if (!elem) return FbxVector4(0, 0, 0, 0);
        using E = FbxGeometryElement;
        int idx;
        if (elem->GetMappingMode() == E::eByControlPoint)
            idx = (elem->GetReferenceMode() == E::eIndexToDirect) ? elem->GetIndexArray().GetAt(cpIdx) : cpIdx;
        else
            idx = (elem->GetReferenceMode() == E::eIndexToDirect) ? elem->GetIndexArray().GetAt(pvIdx) : pvIdx;
        return elem->GetDirectArray().GetAt(idx);
        };

    // --- Construcción por esquina (corner) ---
    for (int p = 0; p < mesh->GetPolygonCount(); ++p)
    {
        const int polySize = mesh->GetPolygonSize(p);
        std::vector<unsigned> cornerIdx; cornerIdx.reserve(polySize);

        for (int v = 0; v < polySize; ++v)
        {
            const int cpIndex = mesh->GetPolygonVertex(p, v);
            const int pvIndex = mesh->GetPolygonVertexIndex(p) + v;

            SimpleVertex out{};

            // Posición (espacio local)
            FbxVector4 P = mesh->GetControlPointAt(cpIndex);
            out.Position = { (float)P[0], (float)P[1], (float)P[2] };

            // Normal por esquina
            FbxVector4 N(0, 1, 0, 0);
            mesh->GetPolygonVertexNormal(p, v, N);
            N.Normalize();
            out.Normal = { (float)N[0], (float)N[1], (float)N[2] };

            // UV (invertir V para DX)
            if (uvElem && uvSetName) {
                int uvIdx = mesh->GetTextureUVIndex(p, v);
                FbxVector2 uv = (uvIdx >= 0) ? uvElem->GetDirectArray().GetAt(uvIdx)
                    : readV2(uvElem, cpIndex, pvIndex);
                out.TextureCoordinate = { (float)uv[0], 1.0f - (float)uv[1] };
            }
            else {
                out.TextureCoordinate = { 0.0f, 0.0f };
            }

            // Tangente / Bitangente si existen
            if (tanElem) {
                FbxVector4 T = readV4(tanElem, cpIndex, pvIndex);
                out.Tangent = { (float)T[0], (float)T[1], (float)T[2] };
            }
            else out.Tangent = { 0,0,0 };

            if (binElem) {
                FbxVector4 B = readV4(binElem, cpIndex, pvIndex);
                out.Bitangent = { (float)B[0], (float)B[1], (float)B[2] };
            }
            else out.Bitangent = { 0,0,0 };

            cornerIdx.push_back((unsigned)vertices.size());
            vertices.push_back(out);
        }

        // Triangula en “fan” (CW por defecto)
        for (int k = 1; k + 1 < polySize; ++k) {
            indices.push_back(cornerIdx[0]);
            indices.push_back(cornerIdx[k + 1]);
            indices.push_back(cornerIdx[k]);
        }
    }

    // --- Fallback: calcula T/B si faltan ---
    if (mesh->GetElementTangentCount() == 0 || mesh->GetElementBinormalCount() == 0)
    {
        auto add = [](EU::Vector3 a, const EU::Vector3& b) { a.x += b.x; a.y += b.y; a.z += b.z; return a; };
        auto sub = [](const EU::Vector3& a, const EU::Vector3& b) { return EU::Vector3(a.x - b.x, a.y - b.y, a.z - b.z); };
        auto mul = [](const EU::Vector3& a, float s) { return EU::Vector3(a.x * s, a.y * s, a.z * s); };

        for (size_t i = 0; i + 2 < indices.size(); i += 3)
        {
            SimpleVertex& v0 = vertices[indices[i + 0]];
            SimpleVertex& v1 = vertices[indices[i + 1]];
            SimpleVertex& v2 = vertices[indices[i + 2]];

            EU::Vector3 e1 = sub(v1.Position, v0.Position);
            EU::Vector3 e2 = sub(v2.Position, v0.Position);

            float du1 = v1.TextureCoordinate.x - v0.TextureCoordinate.x;
            float dv1 = v1.TextureCoordinate.y - v0.TextureCoordinate.y;
            float du2 = v2.TextureCoordinate.x - v0.TextureCoordinate.x;
            float dv2 = v2.TextureCoordinate.y - v0.TextureCoordinate.y;

            float denom = du1 * dv2 - du2 * dv1;
            float r = (std::fabs(denom) < 1e-8f) ? 0.0f : 1.0f / denom;

            EU::Vector3 T = mul(EU::Vector3(e1.x * dv2 - e2.x * dv1, e1.y * dv2 - e2.y * dv1, e1.z * dv2 - e2.z * dv1), r);
            EU::Vector3 B = mul(EU::Vector3(e2.x * du1 - e1.x * du2, e2.y * du1 - e1.y * du2, e2.z * du1 - e1.z * du2), r);

            v0.Tangent = add(v0.Tangent, T);
            v1.Tangent = add(v1.Tangent, T);
            v2.Tangent = add(v2.Tangent, T);
            v0.Bitangent = add(v0.Bitangent, B);
            v1.Bitangent = add(v1.Bitangent, B);
            v2.Bitangent = add(v2.Bitangent, B);
        }
    }

    // --- Autodetecta espejo global del nodo y corrige de forma CONSISTENTE ---
    bool autoDetectMirror = true;
    bool forceFlipWinding = true; // pon true si quieres forzar flip aunque no haya espejo

    bool mirrored = true;
    if (autoDetectMirror) {
        // world = global * geometric (aunque no lo horneamos a vértices, lo usamos para detectar espejo)
        FbxAMatrix geo;
        geo.SetT(node->GetGeometricTranslation(FbxNode::eSourcePivot));
        geo.SetR(node->GetGeometricRotation(FbxNode::eSourcePivot));
        geo.SetS(node->GetGeometricScaling(FbxNode::eSourcePivot));
        FbxAMatrix world = node->EvaluateGlobalTransform() * geo;

        // El signo del producto de escalas indica espejo
        FbxVector4 S = world.GetS();
        double detScale = S[0] * S[1] * S[2];
        mirrored = (detScale < 0.0);
    }

    if (mirrored || forceFlipWinding) {
        // 1) Flip global del winding (todas las caras)
        for (size_t i = 0; i + 2 < indices.size(); i += 3)
            std::swap(indices[i + 1], indices[i + 2]);

        // 2) Invierte TODAS las normales/tangentes/bitangentes (consistencia total)
        for (auto& v : vertices) {
            v.Normal = { v.Normal.x,    v.Normal.y,    v.Normal.z };
            v.Tangent = { v.Tangent.x,   v.Tangent.y,   v.Tangent.z };
            v.Bitangent = { v.Bitangent.x, v.Bitangent.y, v.Bitangent.z };
        }
    }

    // --- Ortonormaliza TBN por vértice ---
    auto dot3 = [](const EU::Vector3& a, const EU::Vector3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; };
    auto norm3 = [](EU::Vector3& v) { float l = std::sqrt(EU::EMax(1e-20f, v.x * v.x + v.y * v.y + v.z * v.z)); v.x /= l; v.y /= l; v.z /= l; };
    auto sub3 = [](const EU::Vector3& a, const EU::Vector3& b) { return EU::Vector3(a.x - b.x, a.y - b.y, a.z - b.z); };
    auto cross3 = [](const EU::Vector3& a, const EU::Vector3& b) {
        return EU::Vector3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
        };

    for (auto& v : vertices)
    {
        norm3(v.Normal);
        float dTN = dot3(v.Tangent, v.Normal);
        v.Tangent = sub3(v.Tangent, EU::Vector3(v.Normal.x * dTN, v.Normal.y * dTN, v.Normal.z * dTN));
        norm3(v.Tangent);

        EU::Vector3 Bcalc = cross3(v.Normal, v.Tangent);
        float hand = (dot3(Bcalc, v.Bitangent) < 0.0f) ? -1.0f : 1.0f;
        v.Bitangent = { Bcalc.x * hand, Bcalc.y * hand, Bcalc.z * hand };
        norm3(v.Bitangent);
    }

    // --- Empaqueta ---
    MeshComponent mc;
    mc.m_name = node->GetName();
    mc.m_vertex = std::move(vertices);
    mc.m_index = std::move(indices);
    mc.m_numVertex = (int)mc.m_vertex.size();
    mc.m_numIndex = (int)mc.m_index.size();
    m_meshes.push_back(std::move(mc));
}

void Model3D::ProcessFBXMaterials(FbxSurfaceMaterial* material)
{
    if (material) {
        FbxProperty prop = material->FindProperty(FbxSurfaceMaterial::sDiffuse);
        if (prop.IsValid()) {
            int textureCount = prop.GetSrcObjectCount<FbxTexture>();
            for (int i = 0; i < textureCount; ++i) {
                FbxTexture* texture = FbxCast<FbxTexture>(prop.GetSrcObject<FbxTexture>(i));
                if (texture) {
                    textureFileNames.push_back(texture->GetName());
                }
            }
        }
    }
}
