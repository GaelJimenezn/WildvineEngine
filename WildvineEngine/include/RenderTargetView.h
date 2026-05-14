#pragma once
#include "Prerequisites.h"

// Forward Declarations
class Device;
class DeviceContext;
class Texture;
class DepthStencilView;

class
    RenderTargetView {
public:
    /**
     * @brief Constructor por defecto.
     */
    RenderTargetView() = default;

    /**
     * @brief Destructor por defecto.
     * @details No libera automáticamente el recurso COM; llamar a destroy().
     */
    ~RenderTargetView() = default;

    /**
     * @brief Inicializa el Render Target View desde el back buffer.
     *
     * @param device     Dispositivo con el que se crea el recurso.
     * @param backBuffer Textura que representa el back buffer (swap chain).
     * @param Format     Formato del RTV (ej. @c DXGI_FORMAT_R8G8B8A8_UNORM).
     * @return @c S_OK si fue exitoso; código @c HRESULT en caso contrario.
     *
     * @post Si retorna @c S_OK, @c m_renderTargetView != nullptr.
     */
    HRESULT
        init(Device& device, Texture& backBuffer, DXGI_FORMAT Format);

    /**
     * @brief Inicializa el Render Target View desde una textura genérica.
     *
     * @param device        Dispositivo con el que se crea el recurso.
     * @param inTex         Textura que será usada como destino de renderizado.
     * @param ViewDimension Dimensión de la vista (ej. @c D3D11_RTV_DIMENSION_TEXTURE2D).
     * @param Format        Formato del RTV.
     * @return @c S_OK si fue exitoso; código @c HRESULT en caso contrario.
     *
     * @note Útil para render targets auxiliares (G-Buffer, mapas de sombra, etc.).
     */
    HRESULT
        init(Device& device,
            Texture& inTex,
            D3D11_RTV_DIMENSION ViewDimension,
            DXGI_FORMAT Format);

    /**
     * @brief Actualiza parámetros internos del RTV.
     *
     * Método de marcador para futuras extensiones (por ejemplo, cambiar dinámicamente
     * la configuración del RTV o recrearlo).
     *
     * @note Actualmente no realiza ninguna operación.
     */
    void
        update();

    /**
     * @brief Limpia y asigna el RTV junto con un Depth Stencil View.
     *
     * Llama a @c OMSetRenderTargets y limpia el RTV con un color dado.
     *
     * @param deviceContext    Contexto de dispositivo donde se aplicará.
     * @param depthStencilView Depth Stencil View a asociar.
     * @param numViews         Número de vistas de render (típicamente 1).
     * @param ClearColor       Color RGBA usado para limpiar el RTV.
     *
     * @pre @c m_renderTargetView debe estar creado con init().
     */
    void
        render(DeviceContext& deviceContext,
            DepthStencilView& depthStencilView,
            unsigned int numViews,
            const float ClearColor[4]);

    /**
     * @brief Asigna el RTV al contexto sin limpiar ni usar Depth Stencil.
     *
     * Llama a @c OMSetRenderTargets solo con el RTV.
     *
     * @param deviceContext Contexto de dispositivo donde se aplicará.
     * @param numViews      Número de vistas de render (típicamente 1).
     *
     * @pre @c m_renderTargetView debe estar creado con init().
     */
    void
        render(DeviceContext& deviceContext,
            unsigned int numViews);

    /**
     * @brief Libera el recurso @c ID3D11RenderTargetView.
     *
     * Idempotente: puede llamarse múltiples veces de forma segura.
     *
     * @post @c m_renderTargetView == nullptr.
     */
    void
        destroy();
private:
    /**
     * @brief Recurso COM de Direct3D 11 para la vista de Render Target.
     * @details Válido tras init(); @c nullptr después de destroy().
     */
    ID3D11RenderTargetView* m_renderTargetView = nullptr;
};