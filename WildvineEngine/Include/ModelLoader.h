#pragma once
#include "Prerequisites.h"
#include "MeshComponent.h"

// Declaraciones adelantadas
class MeshComponent;

/**
 * @class ModelLoader
 * @brief Clase encargada de cargar modelos 3D desde archivos (OBJ Parser manual).
 * @details
 * Esta clase es responsable de la lectura, el parseo y la triangulación de
 * archivos de modelos OBJ para extraer la geometría y poblar un objeto MeshComponent
 * con datos de vértices e índices re-indexados.
 */
class ModelLoader {
public:
  /** @brief Constructor por defecto. */
  ModelLoader() = default;

  /** @brief Destructor por defecto. */
  ~ModelLoader() = default;

  HRESULT init(MeshComponent& mesh, const std::string& fileName);

  void update();
  void render();
  void destroy();
};
