#pragma once
#include "Prerequisites.h"

class Window;
class DeviceContext;

/**
 * @class Viewport
 * @brief Encapsula un @c D3D11_VIEWPORT para definir la región de renderizado en la pantalla.
 *
 * Un viewport en Direct3D 11 especifica el área rectangular del render target donde
 * se dibujarán las primitivas. Incluye dimensiones, profundidad mínima y máxima,
 * así como el origen en la superficie de render.
 *
 * Esta clase permite inicializar un viewport a partir de una ventana o de dimensiones
 * específicas, y aplicarlo al pipeline gráfico.
 */
class
	Viewport {
public:
	/**
	 * @brief Constructor por defecto.
	 */
	Viewport() = default;

	/**
	 * @brief Destructor por defecto.
	 */
	~Viewport() = default;

	/**
	 * @brief Inicializa el viewport a partir de una ventana.
	 *
	 * Utiliza el tamaño del cliente de la ventana para definir las dimensiones
	 * del viewport.
	 *
	 * @param window Referencia a la ventana que define el área de renderizado.
	 * @return @c S_OK si la inicialización fue exitosa.
	 *
	 * @post El miembro @c m_viewport contendrá las dimensiones de la ventana.
	 */
	HRESULT
		init(const Window& window);

	/**
	 * @brief Inicializa el viewport con dimensiones específicas.
	 *
	 * Define un viewport con el ancho y alto especificados.
	 * Los valores de profundidad mínima y máxima se establecen por defecto
	 * en 0.0f y 1.0f respectivamente.
	 *
	 * @param width  Ancho del viewport en píxeles.
	 * @param height Alto del viewport en píxeles.
	 * @return @c S_OK si la inicialización fue exitosa.
	 */
	HRESULT
		init(unsigned int width, unsigned int height);

	/**
	 * @brief Actualiza los parámetros del viewport.
	 *
	 * Método de marcador para futuras extensiones (por ejemplo,
	 * manejo de redimensionado dinámico de la ventana).
	 *
	 * @note Actualmente no realiza ninguna operación.
	 */
	void
		update();

	/**
	 * @brief Aplica el viewport al contexto de dispositivo.
	 *
	 * Llama a @c RSSetViewports para establecer este viewport
	 * en la etapa de rasterización del pipeline.
	 *
	 * @param deviceContext Contexto de dispositivo donde se aplicará.
	 *
	 * @pre El viewport debe haber sido inicializado con @c init().
	 */
	void
		render(DeviceContext& deviceContext);

	/**
	 * @brief Libera recursos asociados al viewport.
	 *
	 * En este caso, no hay recursos COM asociados, por lo que
	 * la implementación es vacía.
	 */
	void
		destroy() {}

public:
	/**
	 * @brief Estructura de Direct3D que define el viewport.
	 */
	D3D11_VIEWPORT m_viewport;
};
