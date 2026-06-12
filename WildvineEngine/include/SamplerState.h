#pragma once
#include "Prerequisites.h"

class Device;
class DeviceContext;

/**
 * @class SamplerState
 * @brief Encapsula un @c ID3D11SamplerState para la etapa de muestreo de texturas en
 * Direct3D 11.
 *
 * Un Sampler State define cómo se leen las texturas en los shaders:
 * - Filtrado (point, linear, anisotropic).
 * - Direccionamiento (wrap, mirror, clamp, border).
 * - Nivel de mipmapping.
 *
 * Esta clase administra la creación, aplicación y destrucción de un @c SamplerState.
 */
class
	SamplerState {
public:
    /**
     * @brief Constructor por defecto.
     */
    SamplerState() = default;

    /**
     * @brief Destructor por defecto.
     * @details No libera automáticamente el recurso COM; llamar a destroy().
     */
    ~SamplerState() = default;

    /**
     * @brief Inicializa el Sampler State con una configuración predeterminada.
     *
     * Crea un @c ID3D11SamplerState configurado según la implementación (ejemplo:
     * filtrado lineal, wrap en UV, LOD completo).
     *
     * @param device Dispositivo con el que se creará el recurso.
     * @return @c S_OK si fue exitoso; código @c HRESULT en caso de error.
     *
     * @post Si retorna @c S_OK, @c m_sampler != nullptr.
     * @sa render(), destroy()
     */
    HRESULT
    	init(Device& device);

    /**
     * @brief Actualiza parámetros internos del Sampler.
     *
     * Método de marcador para recrear o mutar dinámicamente la configuración
     * (por ejemplo, cambiar de filtrado linear a anisotrópico).
     *
     * @note Actualmente no realiza ninguna operación.
     */
    void
    	update();

    /**
     * @brief Asigna el Sampler State a la etapa de Pixel Shader.
     *
     * Llama a @c ID3D11DeviceContext::PSSetSamplers para establecer el sampler.
     *
     * @param deviceContext Contexto donde se aplicará el sampler.
     * @param StartSlot     Slot inicial en el que se vinculará el sampler.
     * @param NumSamplers   Número de samplers a enlazar (normalmente 1).
     *
     * @pre @c m_sampler debe haberse creado con init().
     */
    void
    	render(DeviceContext& deviceContext,
            unsigned int StartSlot,
            unsigned int NumSamplers);

    /**
     * @brief Libera el recurso @c ID3D11SamplerState.
     *
     * Idempotente: puede llamarse múltiples veces de forma segura.
     *
     * @post @c m_sampler == nullptr.
     */
    void
    	destroy();

public:
    /**
     * @brief Recurso COM de Direct3D 11 para el Sampler State.
     * @details Válido tras init(); @c nullptr después de destroy().
     */
    ID3D11SamplerState* m_sampler = nullptr;
};
