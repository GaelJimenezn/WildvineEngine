#include "ParserOBJ.h" 
#include <fstream>     // Para leer archivos (std::ifstream)
#include <sstream>     // Para procesar líneas (std::stringstream)
#include <map>         // Para el caché de vértices (std::map)


bool 
objl::Loader::LoadFile(std::string fileName)
{
  // Limpia los vectores miembro por si acaso
  LoadedVertices.clear();
  LoadedIndices.clear();

  // Llama a nuestro parser interno personalizado
  Parse(fileName);

  // ModelLoader.cpp necesita saber si la carga falló.
  // Si no se cargó nada, devolvemos false.
  if (LoadedVertices.empty() || LoadedIndices.empty()) {
    return false;
  }
  return true;
}

/**
 * Esta es tu función de parsing original, ahora como un método privado.
 * En lugar de devolver un LoadData, llena los miembros de la clase:
 * LoadedVertices y LoadedIndices.
 */
void 
objl::Loader::Parse(std::string fileName)
{
  //Almacenamiento temporal ara los datos leidos el .obj
  std::vector<XMFLOAT3> temp_positions;
  std::vector<XMFLOAT2> temp_texcoords;
  std::vector<XMFLOAT3> temp_normals;

  // Cache de vértices para la indexación
  std::map<std::string, unsigned int> vertexCache;

  std::ifstream file(fileName);
  if (!file.is_open()) {
    ERROR("ParserOBJ", "Parse", "No se pudo abrir el archivo .obj");
    return; // Salimos de la función si no se puede abrir
  }

  std::string line;
  while (std::getline(file, line))
  {
    // Usamos stringstream para separar cada palabra de la línea
    std::stringstream ss(line);
    std::string prefix;
    ss >> prefix; //Lee la primera palabra

    //Vertices de posicion
    if (prefix == "v") {
      XMFLOAT3 pos;
      ss >> pos.x >> pos.y >> pos.z;
      temp_positions.push_back(pos);
    }
    //Leer las Coordenadas de Textura
    else if (prefix == "vt")
    {
      XMFLOAT2 tex;
      ss >> tex.x >> tex.y;
      //El formato .obj invierte 'v' (la textura .y)
      tex.y = 1.0f - tex.y;
      temp_texcoords.push_back(tex);
    }
    // Leer las Normales
    else if (prefix == "vn") {
      XMFLOAT3 norm;
      ss >> norm.x >> norm.y >> norm.z;
      temp_normals.push_back(norm);
    }
    // Leer Caras (la parte de indexación)
    else if (prefix == "f") {
      std::string faceVeretxStr;
      std::vector<std::string> faceVertices; //Almacenamiento "1/1/1", "2/2/2" ...

      //Lee todos los vertices de la cara (ya sean 3 = triangulos, 4 = quad, etc)
      while (ss >> faceVeretxStr) {
        faceVertices.push_back(faceVeretxStr);
      }

      //Triangulacion (manejo de quads y n-gons simples)
      // Un quad se divide en dos trianguls : (0, 1, 2) y (0, 2, 3)
      for (size_t i = 1; i < faceVertices.size() - 1; ++i)
      {
        //Vertices que forman este triangulo
        std::string triangle_indices[3]{
          faceVertices[0],
          faceVertices[i],
          faceVertices[i + 1]
        };

        //Procesar cada uno de los 3 vertices del triangulo
        for (int j = 0; j < 3; ++j) {
          std::string vertexKey = triangle_indices[j];

          // Ya se proceso el vertice unico? (Cache Hit)
          auto it = vertexCache.find(vertexKey);
          if (it != vertexCache.end()) {
            //Si solo reusamos su indice
            // Escribimos en el miembro de la clase
            LoadedIndices.push_back(it->second);
          }
          else {
            //Vertice nuevo (Cache miss)
            std::stringstream ss_face(vertexKey);
            std::string segment;
            std::vector<int> indices; // v, vt, vn

            //Parser "v/vt/vn" (o "v//vn", o "v/vt")
            while (std::getline(ss_face, segment, '/')) {
              if (segment.empty()) {
                indices.push_back(0); //Para "v//vn"
              }
              else {
                indices.push_back(std::stoi(segment));
              }
            }

            //Los indices .obj empiezan en 1, no en 0. Se resta 1
            int v_idx = indices[0] - 1;
            int vt_idx = (indices.size() > 1 && indices[1] != 0) ? indices[1] - 1 : -1;
            int vn_idx = (indices.size() > 2 && indices[2] != 0) ? indices[2] - 1 : -1;

            // Creamos el vértice en el formato que ModelLoader espera (objl::Vertex)
            objl::Vertex new_vertex;

            XMFLOAT3 pos = temp_positions[v_idx];
            new_vertex.Position.X = pos.x;
            new_vertex.Position.Y = pos.y;
            new_vertex.Position.Z = pos.z;

            //Asignar textura (si es que existe)
            if (vt_idx >= 0 && vt_idx < temp_texcoords.size()) {
              XMFLOAT2 tex = temp_texcoords[vt_idx];
              new_vertex.TextureCoordinate.X = tex.x;
              new_vertex.TextureCoordinate.Y = tex.y;
            }
            else {
              new_vertex.TextureCoordinate.X = 0.0f;
              new_vertex.TextureCoordinate.Y = 0.0f;
            }

            //Asignacion de normales (si es que existe)
            if (vn_idx >= 0 && vn_idx < temp_normals.size()) {
              XMFLOAT3 norm = temp_normals[vn_idx];
              new_vertex.Normal.X = norm.x;
              new_vertex.Normal.Y = norm.y;
              new_vertex.Normal.Z = norm.z;
            }
            else {
              new_vertex.Normal.X = 0.0f;
              new_vertex.Normal.Y = 0.0f;
              new_vertex.Normal.Z = 0.0f;
            }

            // Agregarr el nuevo vertice al miembro de la clase
            LoadedVertices.push_back(new_vertex);
            unsigned int new_index = LoadedVertices.size() - 1;

            // Agregar el nuevo indice al miembro de la clase
            LoadedIndices.push_back(new_index);

            //Guardar en el cache para futura reutilizacion
            vertexCache[vertexKey] = new_index;
          }
        }
      }
    }
  }

  file.close();

  // No se devuelve nada. La función LoadFile() se encarga
  // de revisar los miembros LoadedVertices y LoadedIndices que se llamen aquí.
}