#pragma once
#include "Prerequisites.h"
#include "Texture.h"
#include "RenderTargetView.h"
#include "DepthStencilView.h"

class Device;
class DeviceContext;

/**
 * @class EditorViewportPass
 * @brief Off-screen color/depth target used to render the scene inside the editor viewport.
 *
 * The pass owns a color texture/SRV, an RTV, a depth texture/DSV, and the dimensions needed
 * by the GUI viewport panel. Renderers call @c begin() before drawing scene content and the
 * GUI consumes @c getSRV() to show the finished image.
 */
class
	EditorViewportPass {
public:
	/** @brief Creates an empty viewport pass with no GPU resources. */
	EditorViewportPass() = default;
	/** @brief Does not automatically release GPU resources; call destroy() during shutdown. */
	~EditorViewportPass() = default;

	/** @brief Creates color/depth resources for the requested editor viewport size. */
	HRESULT init(Device& device, unsigned int width, unsigned int height);
	/** @brief Recreates size-dependent resources for a new editor viewport size. */
	HRESULT resize(Device& device, unsigned int width, unsigned int height);

	/** @brief Binds the pass render targets and clears them for a new scene render. */
	void begin(DeviceContext& deviceContext, const float clearColor[4]);
	/** @brief Swaps GPU resources and dimensions with another viewport pass. */
	void swap(EditorViewportPass& other);
	/** @brief Clears only the depth target associated with this pass. */
	void clearDepth(DeviceContext& deviceContext);
	/** @brief Applies a viewport matching this pass dimensions. */
	void setViewport(DeviceContext& deviceContext);
	/** @brief Releases all GPU resources owned by the pass. */
	void destroy();

	/** @brief Returns the color shader-resource view consumed by ImGui. */
	ID3D11ShaderResourceView* getSRV() const { return m_colorSRV.m_textureFromImg; }

	/** @brief Returns the current color/depth width in pixels. */
	unsigned int getWidth() const { return m_width; }
	/** @brief Returns the current color/depth height in pixels. */
	unsigned int getHeight() const { return m_height; }

	/** @brief Returns true when all required color and depth resources exist. */
	bool isValid() const
	{
		return m_colorTexture.m_texture != nullptr &&
			m_colorSRV.m_textureFromImg != nullptr &&
			m_depthTexture.m_texture != nullptr;
	}

private:
	/** @brief Allocates the color/depth textures and views for @p width x @p height. */
	HRESULT createResources(Device& device, unsigned int width, unsigned int height);

private:
	/** @brief Off-screen color render texture. */
	Texture           m_colorTexture;
	/** @brief Shader-resource wrapper for the color texture. */
	Texture           m_colorSRV;
	/** @brief Render-target view bound when drawing into the pass. */
	RenderTargetView  m_rtv;

	/** @brief Off-screen depth texture. */
	Texture           m_depthTexture;
	/** @brief Depth-stencil view bound when drawing into the pass. */
	DepthStencilView  m_dsv;

	/** @brief Current pass width in pixels. */
	unsigned int      m_width = 1;
	/** @brief Current pass height in pixels. */
	unsigned int      m_height = 1;
};
