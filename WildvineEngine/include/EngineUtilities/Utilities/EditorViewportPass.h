#pragma once
#include "Prerequisites.h"
#include "Texture.h"
#include "RenderTargetView.h"
#include "DepthStencilView.h"

class Device;
class DeviceContext;

/**
 * @class EditorViewportPass
 * @brief Pase de renderizado que redirige la escena a un Render Target para el editor.
 * * En lugar de renderizar la escena directamente al backbuffer de la pantalla, esta clase
 * crea texturas de color y profundidad para atrapar el renderizado. Luego, esa textura de color
 * se expone como un Shader Resource View (SRV) para que ImGui la dibuje dentro de una ventana
 * (el Viewport del editor).
 */
class 
EditorViewportPass {
public:
	EditorViewportPass() = default;
	~EditorViewportPass() = default;

	/**
	 * @brief Inicializa los recursos del pase de renderizado (texturas, RTV, DSV).
	 * @param device Dispositivo gráfico de DirectX.
	 * @param width Ancho inicial del render target.
	 * @param height Alto inicial del render target.
	 * @return HRESULT S_OK si la inicialización fue exitosa, o código de error en fallo.
	 */
	HRESULT init(Device& device, unsigned int width, unsigned int height);

	/**
	 * @brief Redimensiona los recursos cuando la ventana del viewport en ImGui cambia de tamaño.
	 * @param device Dispositivo gráfico de DirectX.
	 * @param width Nuevo ancho.
	 * @param height Nuevo alto.
	 * @return HRESULT S_OK si el redimensionamiento fue exitoso.
	 */
	HRESULT resize(Device& device, unsigned int width, unsigned int height);

	/**
	 * @brief Prepara el pipeline gráfico vinculando el Render Target y limpiando su color de fondo.
	 * @param deviceContext Contexto del dispositivo para enviar los comandos de dibujo.
	 * @param clearColor Arreglo de 4 flotantes (RGBA) para el color de limpieza del fondo.
	 */
	void
	begin(DeviceContext& deviceContext, const float clearColor[4]);

	/**
	 * @brief Intercambia eficientemente los recursos internos con otro objeto EditorViewportPass.
	 * @param other El objeto con el cual se intercambiarán los recursos.
	 */
	void 
	swap(EditorViewportPass& other);

	/**
	 * @brief Limpia unicamente el buffer de profundidad (Depth/Stencil buffer).
	 * @param deviceContext Contexto del dispositivo gráfico.
	 */
	void 
	clearDepth(DeviceContext& deviceContext);

	/**
	 * @brief Configura el Viewport de DirectX11 para que coincida con las dimensiones de las texturas de este pase.
	 * @param deviceContext Contexto del dispositivo gráfico.
	 */
	void 
	setViewport(DeviceContext& deviceContext);

	/**
	 * @brief Libera todas las texturas, vistas y recursos asociados de la memoria de video.
	 */
	void 
	destroy();

	/**
	 * @brief Obtiene el recurso de textura para ser leído por shaders o interfaces de usuario (ImGui).
	 * @return Puntero a la interfaz ID3D11ShaderResourceView que representa el color de la escena.
	 */
	ID3D11ShaderResourceView* getSRV() const { return m_colorSRV.m_textureFromImg; }

	/** @brief Retorna el ancho actual del render target en píxeles. */
	unsigned int getWidth() const { return m_width; }
	/** @brief Retorna el alto actual del render target en píxeles. */
	unsigned int getHeight() const { return m_height; }

	/**
	 * @brief Verifica si todos los recursos esenciales del pase fueron creados correctamente.
	 * @return true si el RTV, SRV y textura de profundidad son válidos.
	 */
	bool isValid() const
	{
		return m_colorTexture.m_texture != nullptr &&
			m_colorSRV.m_textureFromImg != nullptr &&
			m_depthTexture.m_texture != nullptr;
	}

private:
	/**
	 * @brief Función auxiliar interna para la creación de los recursos de hardware (Texturas y Vistas).
	 * @param device Dispositivo gráfico de DirectX.
	 * @param width Ancho deseado.
	 * @param height Alto deseado.
	 * @return HRESULT Código de resultado de DirectX.
	 */
	HRESULT createResources(Device& device, unsigned int width, unsigned int height);

private:
	Texture           m_colorTexture;      /**< Textura principal que almacena el color de los píxeles renderizados. */
	Texture           m_colorSRV;          /**< Vista de la textura de color optimizada para ser leída por un Shader (Shader Resource View). */
	RenderTargetView  m_rtv;               /**< Vista para permitir que el pipeline de DirectX escriba en la textura de color. */

	Texture           m_depthTexture;      /**< Textura que almacena los valores de profundidad de la escena (Z-Buffer). */
	DepthStencilView  m_dsv;               /**< Vista que permite que el pipeline escriba en el buffer de profundidad y stencil. */

	unsigned int      m_width = 1;         /**< Ancho actual guardado en píxeles de los buffers de renderizado. */
	unsigned int      m_height = 1;        /**< Alto actual guardado en píxeles de los buffers de renderizado. */
};