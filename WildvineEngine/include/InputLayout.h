/**
 * @file InputLayout.h
 * @brief Declara la API pública de InputLayout dentro de WildvineEngine.
 */
#pragma once
#include "Prerequisites.h"

/**
 * @brief Declaración adelantada de la clase Device.
 */
class
Device;

/**
 * @brief Declaración adelantada de la clase DeviceContext.
 */
class
DeviceContext;

/**
 * @class InputLayout
 * @brief Clase encargada de gestionar la creación, actualización, renderizado
 *        y destrucción del Input Layout en DirectX. Define cómo los datos de
 *        los vértices se envían al pipeline gráfico.
 */
class
InputLayout {
public:
  /**
   * @brief Constructor por defecto de InputLayout.
   */
  InputLayout() = default;

  /**
   * @brief Destructor por defecto de InputLayout.
   */
  ~InputLayout() = default;

  /**
   * @brief Inicializa el Input Layout a partir de la descripción de entrada y los datos
   * del Vertex Shader.
   * @param device Referencia al dispositivo de renderizado.
   * @param Layout Vector con la descripción de los elementos de entrada (atributos de
   * vértice).
   * @param VertexShaderData Datos compilados del Vertex Shader necesarios para validar el
   * layout.
   * @return HRESULT que indica el resultado de la creación.
   */
  HRESULT
  init(Device& device,
       const D3D11_INPUT_ELEMENT_DESC* layoutDesc,
       UINT layoutCount,
       ID3DBlob* vertexShaderData);

  /**
   * @brief Actualiza la información o el estado del Input Layout si es necesario.
   */
  void
  update();

  /**
   * @brief Aplica el Input Layout al contexto del dispositivo para el renderizado.
   * @param deviceContext Contexto del dispositivo utilizado para establecer el layout.
   */
  void
  render(DeviceContext& deviceContext);

  /**
   * @brief Libera los recursos asociados al Input Layout.
   */
  void
  destroy();


public:
  /** @brief Puntero al objeto Input Layout de Direct3D. */
  ID3D11InputLayout* m_inputLayout = nullptr;
};
