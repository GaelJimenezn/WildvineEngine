/** 
* @File : ISceneRenderer.h
* @brief : Interfaz para renderizadores de escenas.
* @ingroup Rendering
*/

#pragma once
#include "Prerequisites.h"

class Device;
class DeviceContext;
class Camera;
class RenderScene;
class EdtorViewportPass;
class Tecture;
/**
 * @enum RenderPassType
 * @brief Enumeración RenderPassType definida en el motor WildvineEngine.
 *
 * Esta enumeración gestiona la funcionalidad correspondiente a RenderPassType
 * dentro de la arquitectura del motor gráfico.
 */
enum class
	RenderPassType {
	/** @brief Legacy forward renderer selector. */
	Forward = 0,
	/** @brief Legacy deferred renderer selector. */
	Deferred,
};

/**
* @class ISceneRenderer
* @brief Contrato base para cualquier render consumido por el pipeline pricipal. 
*/

class
	IsceneRenderer {
public: 
	/** @brief Virtual destructor for legacy renderer implementations. */
	virtual
		~IsceneRenderer() = default;


	/** @brief Creates renderer-owned GPU resources. */
	virtual HRESULT
		init(Device& device) = 0;
	/** @brief Recreates size-dependent renderer resources. */
	virtual void
		resize(Device& device, unsigned int width, unsigned int height) = 0;
	/** @brief Renders a scene into the legacy editor viewport pass. */
	virtual void
		render(DeviceContext& deviceContext,
		const Camera& camera, 
		RenderScene& scene, 
		EdtorViewportPass& editorViewportPass) = 0;
	/** @brief Releases renderer-owned GPU resources. */
	virtual void
		destroy() = 0;

	/** @brief Returns a shadow-map SRV when available. */
	virtual ID3D11ShaderResourceView*
		getShadowMapSRV() const { return nullptr; }
	/** @brief Returns a pre-shadow/debug SRV when available. */
	virtual ID3D11ShaderResourceView*
		getPreShadowSRV() const { return nullptr; }
	/** @brief Returns the deferred albedo/metallic GBuffer SRV when available. */
	virtual ID3D11ShaderResourceView*
		getGBufferAlbedoMetallicSRV() const { return nullptr; }
	/** @brief Returns the deferred normal/roughness GBuffer SRV when available. */
	virtual ID3D11ShaderResourceView*
		getGBufferNormalRoughnessSRV() const { return nullptr; }
	/** @brief Returns the deferred world-position/AO GBuffer SRV when available. */
	virtual ID3D11ShaderResourceView*
		getGBufferWorldAoSRV() const { return nullptr; }
	/** @brief Returns the deferred emissive/alpha GBuffer SRV when available. */
	virtual ID3D11ShaderResourceView*
		getGBufferEmissiveAlphaSRV() const { return nullptr; }
	/** @brief Enables or disables shadow-factor debug visualization when supported. */
	virtual void
		setShadowFactorDebugEnabled(bool enabled) { (void)enabled; }
	/** @brief Selects deferred debug view mode when supported. */
	virtual void
		setDeferredDebugViewMode(int mode) { (void)mode; }
	/** @brief Returns a stable renderer name for debug UI. */
	virtual const char*
		getDebugName() const = 0;
};
