#pragma once
#include "Prerequisites.h"

/**
 * @brief Declaración adelantada de la clase Window.
 */
class
Window;

/**
 * @brief Declaración adelantada de la clase DeviceContext.
 */
class
DeviceContext;

/**
 * @class Viewport
 * @brief Clase encargada de administrar el área visible de renderizado (viewport)
 *        dentro de una ventana o superficie de dibujo en DirectX.
 */
class
Viewport {
public:

  /**
   * @brief Constructor por defecto de Viewport.
   */
  Viewport() = default;

  /**
   * @brief Destructor por defecto de Viewport.
   */
  ~Viewport() = default;

  /**
   * @brief Inicializa el viewport utilizando las dimensiones de una ventana.
   * @param window Referencia a la ventana desde la cual se obtienen las dimensiones.
   * @return HRESULT que indica el resultado de la operación.
   */
  HRESULT
  init(const Window& window);

  /**
   * @brief Inicializa el viewport con dimensiones personalizadas.
   * @param width Ancho del área de renderizado.
   * @param height Alto del área de renderizado.
   * @return HRESULT que indica el resultado de la operación.
   */
  HRESULT
  init(unsigned int width, unsigned int height);

  /**
   * @brief Actualiza los parámetros del viewport si es necesario.
   */
  void
  update();

  /**
   * @brief Establece el viewport en el contexto del dispositivo para el renderizado.
   * @param deviceContext Contexto del dispositivo utilizado para aplicar el viewport.
   */
  void
  render(DeviceContext& deviceContext);

  /**
   * @brief Libera los recursos asociados al viewport (si aplica).
   */
  void
  destroy() {}

public:
  /** @brief Estructura que almacena la configuración del viewport de Direct3D. */
  D3D11_VIEWPORT m_viewport;
};
