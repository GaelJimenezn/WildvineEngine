#include "ModelLoader.h"
#include "ParserOBJ.h"

void
ModelLoader::init()
{
  // Inicializa cualquier recurso necesario antes de cargar modelos.
}

void
ModelLoader::update()
{
  // Actualiza el estado interno del cargador si es necesario.
}

void
ModelLoader::render()
{
  // Renderiza el modelo cargado (si se implementa un render directo).
}

void
ModelLoader::destroy()
{
  // Libera los recursos usados por el cargador o modelos cargados.
}

LoadData
ModelLoader::Load(std::string objFileName)
{
  LoadData LD;                       // Estructura donde se almacenarán los datos del modelo.
  objl::Loader m_loader;              // Instancia temporal del cargador OBJ.

  // Intenta cargar el archivo OBJ especificado.
  if (!m_loader.LoadFile(objFileName)) {
    ERROR("ModelLoader", "Load", "No se pudo cargar el archivo .obj");
    return LD;                       // Retorna estructura vacía si la carga falla.
  }

  LD.name = objFileName;              // Guarda el nombre del modelo cargado.

  size_t vertexCount = m_loader.LoadedVertices.size();  // Número total de vértices cargados.
  LD.vertex.resize(vertexCount);      // Redimensiona el vector de vértices.

  // Recorre todos los vértices cargados y copia su información.
  for (int i = 0; i < LD.vertex.size(); i++)
  {
    LD.vertex[i].Position.x = m_loader.LoadedVertices[i].Position.X;
    LD.vertex[i].Position.y = m_loader.LoadedVertices[i].Position.Y;
    LD.vertex[i].Position.z = m_loader.LoadedVertices[i].Position.Z;

    LD.vertex[i].TextureCoordinate.x = m_loader.LoadedVertices[i].TextureCoordinate.X;
    LD.vertex[i].TextureCoordinate.y = m_loader.LoadedVertices[i].TextureCoordinate.Y;

    LD.vertex[i].Normal.x = m_loader.LoadedVertices[i].Normal.X;
    LD.vertex[i].Normal.y = m_loader.LoadedVertices[i].Normal.Y;
    LD.vertex[i].Normal.z = m_loader.LoadedVertices[i].Normal.Z;
  }

  size_t indexCount = m_loader.LoadedIndices.size();   // Número total de índices cargados.
  LD.index.resize(indexCount);                         // Redimensiona el vector de índices.
  LD.index = m_loader.LoadedIndices;                   // Copia los índices cargados.

  LD.numVertex = (int)vertexCount;                     // Guarda la cantidad de vértices.
  LD.numIndex = (int)indexCount;                       // Guarda la cantidad de índices.

  return LD;                                           // Retorna la estructura con los datos del modelo cargado.
}
