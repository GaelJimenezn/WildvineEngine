#pragma once
#include "Prerequisites.h"

/**
 * @class Window
 * @brief Abstracción de una ventana de Windows (Win32) para el motor.
 */

class BaseApp;
class 
Window {
public:
	Window()  = default;
	~Window() = default;

	/**
     * @brief Inicializa la ventana y la registra en el sistema operativo.
     * @param hInstance Instancia de la aplicación.
     * @param nCmdShow Estado de visualización de la ventana.
     * @param wndproc Puntero a la función de procedimiento de ventana.
     * @return HRESULT Estándar de Windows para indicar éxito o error.
     */

	HRESULT 
	init(HINSTANCE hInstance, int nCmdShow, WNDPROC wndproc, BaseApp* app);

	/**
     * @brief Ejecuta la lógica de actualización de la ventana.
     */

	void 
	update();

	/**
     * @brief Gestiona el renderizado directo sobre el contexto de la ventana.
     */
	
	void 
	render();

	/**
     * @brief Libera los recursos de la ventana y destruye el HWND.
     */
	
	void 
	destroy();

public:
    HWND m_hWnd = nullptr;      ///< Manejador de la ventana (Handle).
    unsigned int m_width;       ///< Ancho actual de la ventana.
    unsigned int m_height;      ///< Alto actual de la ventana.

private:
    HINSTANCE m_hInst = nullptr; ///< Instancia del proceso.
    RECT m_rect;                 ///< Rectángulo que define las dimensiones.
    std::string m_windowName = "Wildvine Engine"; ///< Nombre de la ventana.
};

