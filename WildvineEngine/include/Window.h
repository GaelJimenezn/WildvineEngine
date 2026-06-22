#pragma once
#include "Prerequisites.h"

class
BaseApp;

/**
 * @class Window
 * @brief Representa una ventana de aplicación en Windows.
 *
 * Esta clase encapsula la creación, gestión, actualización y destrucción
 * de una ventana Win32, utilizada como superficie de renderizado para DirectX.
 */
class
Window {
public:
  /**
   * @brief Constructor por defecto.
   */
  Window() = default;

  /**
   * @brief Destructor por defecto.
   */
  ~Window() = default;


  /**
   * @brief Inicializa y crea la ventana de la aplicación.
   *
   * @param hInstance Manejador de la instancia de la aplicación.
   * @param nCmdShow Parámetro que indica cómo se mostrará la ventana.
   * @param wndproc Función de procedimiento de ventana (callback de mensajes).
   * @return HRESULT Código de resultado (S_OK si se creó correctamente).
   */
  HRESULT
  init(HINSTANCE hInstance, int nCmdShow, WNDPROC wndproc, BaseApp* app);

  /**
   * @brief Actualiza el estado de la ventana.
   *
   * Normalmente procesa eventos o lógica asociada al ciclo de vida de la ventana.
   */
  void
  update();

  /**
   * @brief Renderiza el contenido de la ventana.
   *
   * Generalmente se usa junto con el contexto gráfico (DirectX/OpenGL).
   */
  void
  render();

  /**
   * @brief Libera los recursos y destruye la ventana.
   */
  void
  destroy();

public:
  /**
   * @brief Handle de la ventana Win32.
   */
  HWND m_hWnd = nullptr;

  /**
   * @brief Ancho actual de la ventana.
   */
  unsigned int m_width;

  /**
   * @brief Alto actual de la ventana.
   */
  unsigned int m_height;

private:
  /**
   * @brief Handle de la instancia de la aplicación.
   */
  HINSTANCE m_hInst = nullptr;

  /**
   * @brief Rectángulo que define las dimensiones de la ventana.
   */
  RECT m_rect;

  /**
   * @brief Nombre de la ventana (por defecto "Navi Engine").
   */
  std::string m_windowName = "Navi Engine";
};
