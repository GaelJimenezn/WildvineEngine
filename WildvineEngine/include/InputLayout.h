/**
 * @file InputLayout.h
 * @brief Declara la API pÃºblica de InputLayout dentro de WildvineEngine.
 */
#pragma once
#include "Prerequisites.h"

/**
 * @brief DeclaraciÃ³n adelantada de la clase Device.
 */
class Device;

/**
 * @brief DeclaraciÃ³n adelantada de la clase DeviceContext.
 */
class DeviceContext;

/**
 * @class InputLayout
 * @brief Clase encargada de gestionar la creaciÃ³n, actualizaciÃ³n, renderizado
 *        y destrucciÃ³n del Input Layout en DirectX. Define cÃ³mo los datos de
 *        los vÃ©rtices se envÃ­an al pipeline grÃ¡fico.
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
   * @brief Inicializa el Input Layout a partir de la descripciÃ³n de entrada y los datos
   * del Vertex Shader.
   * @param device Referencia al dispositivo de renderizado.
   * @param layoutDesc Descripción de los elementos de entrada.
   * @param layoutCount Número de elementos de entrada.
   * @param vertexShaderData Datos compilados del Vertex Shader.
   * @return HRESULT que indica el resultado de la creaciÃ³n.
   */
  HRESULT
  init(Device &device,
       const D3D11_INPUT_ELEMENT_DESC *layoutDesc,
       UINT layoutCount,
       ID3DBlob *vertexShaderData);

  /**
   * @brief Actualiza la informaciÃ³n o el estado del Input Layout si es necesario.
   */
  void update();

  /**
   * @brief Aplica el Input Layout al contexto del dispositivo para el renderizado.
   * @param deviceContext Contexto del dispositivo utilizado para establecer el layout.
   */
  void render(DeviceContext &deviceContext);

  /**
   * @brief Libera los recursos asociados al Input Layout.
   */
  void destroy();

public:
  /** @brief Puntero al objeto Input Layout de Direct3D. */
  ID3D11InputLayout *m_inputLayout = nullptr;
};
