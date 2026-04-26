/**
 * @file EditorViewportPass.h
 * @brief Define el pase de renderizado dedicado al viewport del editor.
 * Contiene y administra su propio Render Target y Depth Stencil para dibujar la escena fuera de la pantalla principal.
 */

#pragma once
#include "Prerequisites.h"
#include "Texture.h"
#include "RenderTargetView.h"
#include "DepthStencilView.h"
#include <utility> // Necesario para std::swap

class Device;
class DeviceContext;

/**
 * @class EditorViewportPass
 * @brief Gestiona los recursos gráficos necesarios para renderizar una escena dentro de un panel/ventana del editor (como ImGui).
 */
class 
EditorViewportPass {
public:
	/**
	 * @brief Constructor por defecto.
	 */
	EditorViewportPass() = default;

	/**
	 * @brief Destructor por defecto.
	 */
	~EditorViewportPass() = default;

	/**
	 * @brief Inicializa los recursos iniciales del pase del viewport.
	 * @param device Referencia al dispositivo de DirectX para crear las texturas.
	 * @param width Anchura inicial de la ventana del viewport.
	 * @param height Altura inicial de la ventana del viewport.
	 * @return HRESULT indicando si la creación fue exitosa.
	 */
	HRESULT
	init(Device& device, unsigned int width, unsigned int height);

	/**
	 * @brief Redimensiona los recursos (RTV y DSV) si el tamaño de la ventana del editor cambia.
	 * @param device Referencia al dispositivo de DirectX.
	 * @param width Nueva anchura del viewport.
	 * @param height Nueva altura del viewport.
	 * @return HRESULT indicando el resultado de la recreación de recursos.
	 */
	HRESULT 
	resize(Device& device, unsigned int width, unsigned int height);

	/**
	 * @brief Prepara este pase para empezar a dibujar en él, limpiando su Color Buffer.
	 * @param deviceContext Contexto del dispositivo para emitir los comandos gráficos.
	 * @param clearColor Arreglo de 4 flotantes (RGBA) con el color de fondo para limpiar el RTV.
	 */
	void 
	begin(DeviceContext& deviceContext, const float clearColor[4]);

	/**
	 * @brief Intercambia los datos y recursos de este pase con los de otro. Implementación inline por defecto.
	 * @param other Referencia al otro EditorViewportPass con el que se intercambiarán los recursos.
	 */
	void 
	swap(EditorViewportPass& other) {
		std::swap(m_colorTexture, other.m_colorTexture);
		std::swap(m_colorSRV, other.m_colorSRV);
		std::swap(m_rtv, other.m_rtv);
		std::swap(m_depthTexture, other.m_depthTexture);
		std::swap(m_dsv, other.m_dsv);
		std::swap(m_width, other.m_width);
		std::swap(m_height, other.m_height);
	}

	/**
	 * @brief Limpia el búfer de profundidad (Z-Buffer) a su valor máximo (1.0).
	 * @param deviceContext Contexto de la GPU.
	 */
	void 
	clearDepth(DeviceContext& deviceContext);

	/**
	 * @brief Establece el área de dibujado (Viewport) en el pipeline según las dimensiones actuales.
	 * @param deviceContext Contexto de la GPU.
	 */
	void 
	setViewport(DeviceContext& deviceContext);

	/**
	 * @brief Libera la memoria y destruye los recursos alojados en la GPU. Implementación inline básica por defecto.
	 */
	void 
	destroy() {
		m_colorTexture.destroy();
		m_colorSRV.destroy();
		m_rtv.destroy();
		m_depthTexture.destroy();
		m_dsv.destroy();
		m_width = 1;
		m_height = 1;
	}

	/**
	 * @brief Obtiene la vista de recurso del shader (SRV) para poder mostrar el resultado final como textura.
	 * @return Puntero al ID3D11ShaderResourceView de la textura de color resultante.
	 */
	ID3D11ShaderResourceView* getSRV() const { return m_colorSRV.m_textureFromImg; }

	/**
	 * @brief Obtiene el ancho actual del pase de renderizado.
	 * @return Ancho en píxeles.
	 */
	unsigned int
	getWidth() const { return m_width; }
	
	/**
	 * @brief Obtiene el alto actual del pase de renderizado.
	 * @return Alto en píxeles.
	 */
	unsigned int
	getHeight() const { return m_height; }

	/**
	 * @brief Comprueba si los recursos de textura y vistas fueron creados correctamente.
	 * @return true si los recursos principales no son nulos, false en caso contrario.
	 */
	bool 
	isValid() const
	{
		return m_colorTexture.m_texture != nullptr &&
			m_colorSRV.m_textureFromImg != nullptr &&
			m_depthTexture.m_texture != nullptr;
	}

private:
	/**
	 * @brief Función interna para instanciar las texturas y sus vistas (RTV y DSV).
	 * @param device Referencia al dispositivo.
	 * @param width Anchura a crear.
	 * @param height Altura a crear.
	 * @return HRESULT con el estado de las llamadas internas.
	 */
	HRESULT 
	createResources(Device& device, unsigned int width, unsigned int height);

private:
	Texture           m_colorTexture;   /**< Textura que actúa como búfer de color principal. */
	Texture           m_colorSRV;       /**< Envoltorio para mantener el Shader Resource View de la textura de color. */
	RenderTargetView  m_rtv;            /**< Vista de Render Target para escribir en la textura de color. */

	Texture           m_depthTexture;   /**< Textura que contiene la información del búfer de profundidad/stencil. */
	DepthStencilView  m_dsv;            /**< Vista de Depth Stencil para usar la textura de profundidad. */

	unsigned int      m_width = 1;      /**< Ancho interno almacenado del viewport. */
	unsigned int      m_height = 1;     /**< Alto interno almacenado del viewport. */
};