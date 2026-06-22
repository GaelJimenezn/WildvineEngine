#pragma once
#include "Prerequisites.h"
#include "Texture.h"
#include "RenderTargetView.h"
#include "DepthStencilView.h"

class Device;
class DeviceContext;

/**
 * @class EditorViewportPass
 * @brief Representa un render pass utilizado para visualizar contenido en el viewport del editor.
 *
 * Este pass maneja sus propios recursos de render:
 * - Render target (color)
 * - Depth stencil
 * - Viewport
 *
 * Permite renderizar la escena a una textura que posteriormente puede ser mostrada en el editor.
 */
class
EditorViewportPass {
public:
	/** @brief Constructor por defecto. */
	EditorViewportPass() = default;

	/** @brief Destructor por defecto. */
	~EditorViewportPass() = default;

	/**
	 * @brief Inicializa los recursos del viewport.
	 * @param device Dispositivo gráfico.
	 * @param width Ancho del viewport.
	 * @param height Alto del viewport.
	 * @return HRESULT indicando éxito o fallo.
	 */
	HRESULT 
  init(Device& device, unsigned int width, unsigned int height);

	/**
	 * @brief Redimensiona los recursos del viewport.
	 * @param device Dispositivo gráfico.
	 * @param width Nuevo ancho.
	 * @param height Nuevo alto.
	 * @return HRESULT indicando éxito o fallo.
	 */
	HRESULT 
	resize(Device& device, unsigned int width, unsigned int height);

	/**
	 * @brief Inicia el pass de render.
	 * @param deviceContext Contexto del dispositivo.
	 * @param clearColor Color de limpieza del render target.
	 */
	void 
	begin(DeviceContext& deviceContext, const float clearColor[4]);

	/**
	 * @brief Intercambia los recursos con otro viewport pass.
	 * @param other Otro EditorViewportPass.
	 */
	void 
	swap(EditorViewportPass& other);

	/**
	 * @brief Limpia el buffer de profundidad.
	 * @param deviceContext Contexto del dispositivo.
	 */
	void
	clearDepth(DeviceContext& deviceContext);

	/**
	 * @brief Configura el viewport en el pipeline.
	 * @param deviceContext Contexto del dispositivo.
	 */
	void 
	setViewport(DeviceContext& deviceContext);

	/**
	 * @brief Libera todos los recursos.
	 */
	void
	destroy();

	/**
	 * @brief Obtiene el Shader Resource View del render target.
	 * @return Puntero a ID3D11ShaderResourceView.
	 */
	ID3D11ShaderResourceView* getSRV() const { return m_colorSRV.m_textureFromImg; }

	/**
	 * @brief Obtiene el ancho del viewport.
	 * @return Ancho en píxeles.
	 */
	unsigned 
	int getWidth() const { return m_width; }

	/**
	 * @brief Obtiene el alto del viewport.
	 * @return Alto en píxeles.
	 */
	unsigned
	int getHeight() const { return m_height; }

	/**
	 * @brief Verifica si los recursos son válidos.
	 * @return true si todos los recursos están correctamente inicializados.
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
	 * @brief Crea los recursos internos del viewport.
	 * @param device Dispositivo gráfico.
	 * @param width Ancho.
	 * @param height Alto.
	 * @return HRESULT indicando éxito o fallo.
	 */
	HRESULT 
	createResources(Device& device, unsigned int width, unsigned int height);

private:
	/** @brief Textura de color. */
	Texture           m_colorTexture;

	/** @brief Shader Resource View del color. */
	Texture           m_colorSRV;

	/** @brief Render Target View. */
	RenderTargetView  m_rtv;

	/** @brief Textura de profundidad. */
	Texture           m_depthTexture;

	/** @brief Depth Stencil View. */
	DepthStencilView  m_dsv;

	/** @brief Ancho del viewport. */
	unsigned int      m_width = 1;

	/** @brief Alto del viewport. */
	unsigned int      m_height = 1;

};