#pragma once
#include "Prerequisites.h"
#include "fbxsdk.h"

/**
 * @class ModelLoader
 * @brief Clase encargada de gestionar la carga, actualización, renderizado
 *        y destrucción de modelos 3D, permitiendo importar datos desde archivos externos.
 */
class
ModelLoader{
public:

  /**
   * @brief Constructor por defecto de ModelLoader.
   */
  ModelLoader() = default;

  /**
   * @brief Destructor por defecto de ModelLoader.
   */
  ~ModelLoader() = default;

  /**
   * @brief Inicializa los recursos necesarios para el cargador de modelos.
   */
  void
  init();

  /**
   * @brief Actualiza el estado del cargador de modelos, si es necesario.
   */
  void
  update();

  /**
   * @brief Renderiza el modelo cargado (si aplica).
   */
  void
  render();

  /**
   * @brief Libera los recursos asociados al cargador de modelos.
   */
  void
  destroy();

  /**
   * @brief Carga un archivo de modelo 3D (por ejemplo, formato OBJ) y devuelve su información.
   * @param objFileName Nombre o ruta del archivo OBJ a cargar.
   * @return Estructura LoadData que contiene los datos del modelo cargado.
   */
  LoadData
  Load(std::string objFileName);

private:
  FbxManager* lSdkManager;
  FbxScene* lScene;
  std::vector<std::string > textureFileName;
public:
  std::string modelName;
 
};

