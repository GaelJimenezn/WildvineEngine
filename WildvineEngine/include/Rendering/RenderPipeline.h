/**
 * @file RenderPipeline.h
 * @brief Declara el orquestador de renderers de escena del motor.
 * @ingroup rendering
 */
#pragma once
#include "Rendering/ForwardRenderer.h"
#include "Rendering/DeferredRenderer.h"

/**
 * @enum RendererType
 * @brief Tipos de renderer soportados por el pipeline.
 */
enum
/** @brief Declara class RendererType. */
class
RendererType { Forward = 0, Deferred = 1 };

/**
 * @class RenderPipeline
 * @brief Selecciona y ejecuta el renderer activo para el frame actual.
 */
class
RenderPipeline {
public:
	/**
	 * @brief Inicializa el pipeline con el renderer indicado.
	 * @param device           Dispositivo D3D11.
	 * @param initialRenderer  Tipo de renderer inicial (Forward o Deferred).
	 * @return S_OK en éxito.
	 */
	HRESULT
	init(Device& device, RendererType initialRenderer = RendererType::Deferred);

	/**
	 * @brief Cambia el renderer activo en tiempo de ejecución.
	 * @param rendererType Nuevo tipo de renderer.
	 * @param device       Dispositivo D3D11 (para inicializar si es necesario).
	 * @return S_OK en éxito.
	 */
	HRESULT
	setRendererType(RendererType rendererType, Device& device);

	/**
	 * @brief Redimensiona los recursos internos del renderer activo.
	 * @param device Dispositivo D3D11.
	 * @param width  Nuevo ancho en píxeles.
	 * @param height Nuevo alto en píxeles.
	 */
	void
	resize(Device& device, unsigned int width, unsigned int height);

	/**
	 * @brief Ejecuta el render del frame completo usando el renderer activo.
	 * @param deviceContext Contexto de comandos D3D11.
	 * @param camera        Cámara del editor.
	 * @param scene         Datos de la escena para este frame.
	 * @param viewportPass  Pass de render del viewport del editor.
	 */
	void
	render(DeviceContext& deviceContext,
		const Camera& camera,
		RenderScene& scene,
		EditorViewportPass& viewportPass);

	/** @brief Libera todos los recursos de los renderers. */
	void
	destroy();

	/**
	 * @brief Devuelve el tipo de renderer activo.
	 * @return RendererType actual (Forward o Deferred).
	 */
	RendererType
	getRendererType() const { return m_activeRendererType; }

	/**
	 * @brief Devuelve el nombre del renderer activo como cadena.
	 * @return Cadena literal con el nombre del renderer.
	 */
	const char*
	getActiveRendererName() const;

	/** @brief @return SRV del shadow map generado en el shadow pass. */
	ID3D11ShaderResourceView*
	getShadowMapSRV() const;
	/** @brief @return SRV del pre-shadow debug pass. */
	ID3D11ShaderResourceView*
	getPreShadowSRV() const;
	/** @brief @return SRV del GBuffer Albedo+Metallic. */
	ID3D11ShaderResourceView*
	getGBufferAlbedoMetallicSRV() const;
	/** @brief @return SRV del GBuffer Normal+Roughness. */
	ID3D11ShaderResourceView*
	getGBufferNormalRoughnessSRV() const;
	/** @brief @return SRV del GBuffer WorldPos+AO. */
	ID3D11ShaderResourceView*
	getGBufferWorldAoSRV() const;
	/** @brief @return SRV del GBuffer Emissive+Alpha. */
	ID3D11ShaderResourceView*
	getGBufferEmissiveAlphaSRV() const;

	/**
	 * @brief Activa o desactiva el modo debug de shadow factor.
	 * @param enabled true para activar la visualización de shadow factor.
	 */
	void
	setShadowFactorDebugEnabled(bool enabled);

private:
	/** @brief Declara o ejecuta ensureRendererInitialized. */
	HRESULT
	ensureRendererInitialized(RendererType rendererType, Device& device);
	/** @brief Declara o ejecuta resolveRenderer. */
	ISceneRenderer*
	resolveRenderer(RendererType rendererType);
	/** @brief Declara o ejecuta resolveRenderer. */
	const ISceneRenderer*
	resolveRenderer(RendererType rendererType) const;

private:
	ForwardRenderer m_forwardRenderer;
	DeferredRenderer m_deferredRenderer;
	ISceneRenderer* m_activeRenderer = nullptr;
	RendererType m_activeRendererType = RendererType::Deferred;
	bool m_forwardInitialized = false;
	bool m_deferredInitialized = false;
};
