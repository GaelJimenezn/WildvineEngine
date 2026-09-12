/**
 * @file Window.h
 * @brief Declara la API pÃºblica de Window dentro de WildvineEngine.
 */
#pragma once
#include "Prerequisites.h"

/** @brief Declara class BaseApp. */
class BaseApp;

/**
 * @class Window
 * @brief Representa una ventana de aplicaciÃ³n en Windows.
 *
 * Esta clase encapsula la creaciÃ³n, gestiÃ³n, actualizaciÃ³n y destrucciÃ³n
 * de una ventana Win32, utilizada como superficie de renderizado para DirectX.
 */
class Window {
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
   * @brief Inicializa y crea la ventana de la aplicaciÃ³n.
   *
   * @param hInstance Manejador de la instancia de la aplicaciÃ³n.
   * @param nCmdShow ParÃ¡metro que indica cÃ³mo se mostrarÃ¡ la ventana.
   * @param wndproc FunciÃ³n de procedimiento de ventana (callback de mensajes).
   *
   * @param app AplicaciÃ³n propietaria que recibe los eventos de ventana.
   * @return
   * HRESULT CÃ³digo de resultado (S_OK si se creÃ³ correctamente).
   */
  HRESULT
  init(HINSTANCE hInstance, int nCmdShow, WNDPROC wndproc, BaseApp *app);

  /**
   * @brief Actualiza el estado de la ventana.
   *
   * Normalmente procesa eventos o lÃ³gica asociada al ciclo de vida de la ventana.
   */
  void update();

  /**
   * @brief Renderiza el contenido de la ventana.
   *
   * Generalmente se usa junto con el contexto grÃ¡fico (DirectX/OpenGL).
   */
  void render();

  /**
   * @brief Libera los recursos y destruye la ventana.
   */
  void destroy();

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
   * @brief Handle de la instancia de la aplicaciÃ³n.
   */
  HINSTANCE m_hInst = nullptr;

  /**
   * @brief RectÃ¡ngulo que define las dimensiones de la ventana.
   */
  RECT m_rect;

  /**
   * @brief Nombre de la ventana (por defecto "Navi Engine").
   */
  std::string m_windowName = "Navi Engine";
};
