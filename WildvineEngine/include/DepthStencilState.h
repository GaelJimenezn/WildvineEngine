#pragma once
#include "Prerequisites.h"

class Device;
class DeviceContext;

/**
 * @class DepthStencilState
 * @brief Encapsula un @c ID3D11DepthStencilState y su ciclo de vida para la etapa Output-Merger.
 *
 * Administra la creación, configuración, aplicación y destrucción de un estado de
 * profundidad/esténcil en Direct3D 11. Permite activar/desactivar pruebas de profundidad y
 * funcionalidades de esténcil según se requiera.
 *
 * @note La clase no posee ni administra la vida de @c Device ni de @c DeviceContext.
 */
class
    DepthStencilState {
public:
    /**
     * @brief Constructor por defecto (no crea recursos).
     */
    DepthStencilState() = default;

    /**
     * @brief Destructor por defecto.
     * @details No libera automáticamente el recurso COM; llamar a destroy().
     */
    ~DepthStencilState() = default;

    /**
     * @brief Crea el objeto @c ID3D11DepthStencilState a partir de flags comunes.
     *
     * Genera y almacena internamente un estado de profundidad/esténcil. La configuración
     * concreta (función de comparación, máscaras, operaciones de esténcil, etc.) debe definirse
     * en la implementación de este método, condicionada por @p enableDepth y @p enableStencil.
     *
     * @param device         Dispositivo con el que se crea el recurso.
     * @param enableDepth    Habilita la prueba de profundidad (por defecto @c true).
     * @param enableStencil  Habilita el buffer de esténcil y sus pruebas (por defecto @c false).
     * @return @c S_OK si la creación fue exitosa; código @c HRESULT en caso contrario.
     *
     * @post Si retorna @c S_OK, @c m_depthStencilState != nullptr.
     * @sa render(), destroy()
     */
    HRESULT
        init(Device& device,
            bool depthEnable,
            D3D11_DEPTH_WRITE_MASK writeMask,
            D3D11_COMPARISON_FUNC depthFunc);

    /**
     * @brief Actualiza parámetros internos si la implementación lo requiere.
     *
     * Método placeholder para futuros cambios dinámicos del descriptor de profundidad/esténcil
     * (p. ej., recrear el recurso con otros flags).
     *
     * @note Actualmente no realiza ninguna operación.
     */
    void
        update();

    /**
     * @brief Aplica el estado de profundidad/esténcil al contexto (OMSetDepthStencilState).
     *
     * Vincula el @c ID3D11DepthStencilState al @c DeviceContext. Puede además restablecer el estado
     * a @c nullptr si @p reset es @c true.
     *
     * @param deviceContext  Contexto donde se aplicará el estado.
     * @param stencilRef     Referencia de esténcil usada por las operaciones de esténcil (por defecto 0).
     * @param reset          Si es @c true, desvincula el estado después de aplicarlo (setea @c nullptr).
     *
     * @pre @c m_depthStencilState debe haber sido creado con init().
     * @note Internamente invoca @c ID3D11DeviceContext::OMSetDepthStencilState.
     * @sa init(), destroy()
     */
    void
        render(DeviceContext& deviceContext, unsigned int stencilRef = 0, bool reset = false);

    /**
     * @brief Libera el recurso @c ID3D11DepthStencilState y deja la instancia en estado no inicializado.
     *
     * Idempotente: puede llamarse múltiples veces de forma segura.
     *
     * @post @c m_depthStencilState == nullptr.
     */
    void
        destroy();

private:
    /**
     * @brief Recurso COM de Direct3D 11 para el estado de profundidad/esténcil.
     * @details Válido tras @c init(); @c nullptr después de @c destroy().
     */
    ID3D11DepthStencilState* m_depthStencilState = nullptr;
};