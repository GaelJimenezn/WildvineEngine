#pragma once
#include "Prerequisites.h"

class Device;
class DeviceContext;

/**
 * @class Rasterizer
 * @brief Encapsula un @c ID3D11RasterizerState para configurar la etapa de rasterización
 * en el pipeline gráfico.
 *
 * La etapa de rasterización en Direct3D 11 define cómo se transforman las primitivas
 * (triángulos, líneas, puntos) en fragmentos antes de pasar al Pixel Shader.
 *
 * Esta clase administra la creación, aplicación y destrucción de un @c RasterizerState,
 * permitiendo configurar opciones como el modo de relleno (wireframe/solid), el culling
 * y la profundidad del clipping.
 */
class
	RasterizerState {
public:
    /**
     * @brief Constructor por defecto.
     */
    RasterizerState() = default;

    /**
     * @brief Destructor por defecto.
     * @details No libera automáticamente el recurso COM; llamar a destroy().
     */
    ~RasterizerState() = default;

    /**
     * @brief Inicializa el Rasterizer State.
     *
     * Crea un @c ID3D11RasterizerState con una configuración determinada (por ejemplo,
     * @c D3D11_FILL_SOLID y @c D3D11_CULL_BACK).
     * La configuración exacta debe definirse en la implementación del método.
     *
     * @param device Dispositivo con el que se creará el recurso.
     * @return @c S_OK si la creación fue exitosa; código @c HRESULT en caso de error.
     *
     * @post Si retorna @c S_OK, @c m_rasterizerState != nullptr.
     * @sa render(), destroy()
     */
    HRESULT
    	init(Device device);

    /**
     * @brief Inicializa la instancia de la clase.
     *
     * @return Retorna el resultado de la operación.
     */
    HRESULT
    	init(Device& device,
            D3D11_FILL_MODE fill,
            D3D11_CULL_MODE cull,
            bool frontCCW,
            bool depthClip);

    /**
     * @brief Actualiza parámetros internos del Rasterizer.
     *
     * Método de marcador para recrear o modificar dinámicamente la configuración
     * del rasterizador.
     *
     * @note Actualmente no realiza ninguna operación.
     */
    void
    	update();

    /**
     * @brief Aplica el Rasterizer State al contexto de dispositivo.
     *
     * Llama a @c ID3D11DeviceContext::RSSetState para establecer el rasterizador activo.
     *
     * @param deviceContext Contexto donde se aplicará el rasterizer state.
     *
     * @pre @c m_rasterizerState debe haberse creado con init().
     */
    void
    	render(DeviceContext& deviceContext);

    /**
     * @brief Libera el recurso @c ID3D11RasterizerState.
     *
     * Idempotente: puede llamarse múltiples veces de forma segura.
     *
     * @post @c m_rasterizerState == nullptr.
     */
    void
    	destroy();

private:
    /**
     * @brief Estado de rasterización de Direct3D 11.
     * @details Válido después de init(); @c nullptr tras destroy().
     */
    ID3D11RasterizerState* m_rasterizerState = nullptr;
};
