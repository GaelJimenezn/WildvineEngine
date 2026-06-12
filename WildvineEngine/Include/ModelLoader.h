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
class
	ModelLoader {
public:
  /** @brief Constructor por defecto. */
  ModelLoader() = default;

  /** @brief Destructor por defecto. */
  ~ModelLoader() = default;

  /**
   * @brief Loads an OBJ file and writes parsed geometry into @p mesh.
   * @param mesh Destination mesh component that receives vertices and indices.
   * @param fileName Source OBJ file path.
   * @return @c S_OK on success; failing @c HRESULT otherwise.
   */
  HRESULT
  	init(MeshComponent& mesh, const std::string& fileName);

  /** @brief Reserved update hook for future streaming/import progress. */
  void
  	update();
  /** @brief Reserved render hook; model loading does not draw directly. */
  void
  	render();
  /** @brief Releases transient loader state, if any. */
  void
  	destroy();
};
