#pragma once
#include "Prerequisites.h"


class BaseApp;
/**
 * @class Window
 * @brief Owns the Win32 window handle and presentation dimensions for the engine.
 *
 * The window wrapper registers/creates the native window, stores its dimensions, and keeps
 * the application instance/rectangle data needed by resize and message handling code.
 */
class
	Window {
public:
	/** @brief Creates an empty window wrapper. */
	Window() = default;
	/** @brief Does not destroy the native window automatically; use destroy() in shutdown flow. */
	~Window() = default;

	/**
	 * @brief Creates the Win32 window and connects it to the application message procedure.
	 * @param hInstance Current process instance.
	 * @param nCmdShow Initial Win32 show command.
	 * @param wndproc Window procedure used to dispatch messages.
	 * @param app Application object stored in window user data.
	 * @return @c S_OK on success; failing @c HRESULT otherwise.
	 */
	HRESULT
		init(HINSTANCE hInstance, int nCmdShow, WNDPROC wndproc, BaseApp* app);

	/** @brief Updates cached window state if the implementation requires it. */
	void
		update();

	/** @brief Placeholder render hook for window-level presentation work. */
	void
		render();

	/** @brief Releases or unregisters native window resources owned by this wrapper. */
	void
		destroy();

public:
	/** @brief Native Win32 window handle. */
	HWND m_hWnd = nullptr;
	/** @brief Current client width in pixels. */
	unsigned int m_width;
	/** @brief Current client height in pixels. */
	unsigned int m_height;
private:
	/** @brief Win32 application instance used during window creation. */
	HINSTANCE m_hInst = nullptr;
	/** @brief Cached window/client rectangle used for sizing. */
	RECT m_rect;
	/** @brief Title displayed in the Win32 window caption. */
	std::string m_windowName = "Wildvine Engine";
};
