#pragma once
#include "Prerequisites.h"

class Device;
class DeviceContext;

/**
 * @class InputLayout
 * @brief Encapsula un @c ID3D11InputLayout que describe el formato de los vértices para el pipeline de entrada.
 *
 * Un Input Layout en Direct3D 11 define cómo se interpretan los datos de un Vertex Buffer
 * (posición, normales, UVs, colores, etc.) y cómo se asignan a las entradas de un Vertex Shader.
 *
 * Esta clase administra la creación, uso y destrucción del recurso @c ID3D11InputLayout.
 */
class
    InputLayout {
public:
    /**
     * @brief Constructor por defecto.
     */
    InputLayout() = default;

    /**
     * @brief Destructor por defecto.
     * @details No libera automáticamente el recurso COM; llamar a destroy().
     */
    ~InputLayout() = default;

    /**
     * @brief Inicializa el Input Layout a partir de una descripción y bytecode de Vertex Shader.
     *
     * Crea un @c ID3D11InputLayout utilizando un arreglo de @c D3D11_INPUT_ELEMENT_DESC
     * y el bytecode compilado de un Vertex Shader que define la firma de entrada.
     *
     * @param device            Dispositivo con el que se crea el recurso.
     * @param Layout            Vector con la descripción de los elementos de entrada (semánticas, formato, offset, etc.).
     * @param VertexShaderData  Bytecode compilado del Vertex Shader que contiene la firma de entrada.
     * @return @c S_OK si la creación fue exitosa; código @c HRESULT en caso de error.
     *
     * @post Si retorna @c S_OK, @c m_inputLayout != nullptr.
     */
    HRESULT
        init(Device& device,
            const D3D11_INPUT_ELEMENT_DESC* layoutDesc,
            UINT layoutCount,
            ID3DBlob* vertexShaderData);

    /**
     * @brief Actualiza parámetros internos del Input Layout.
     *
     * Método de marcador, útil si en el futuro se desea recrear o modificar dinámicamente
     * el Input Layout.
     *
     * @note Actualmente no realiza ninguna operación.
     */
    void
        update();

    /**
     * @brief Aplica el Input Layout al contexto de dispositivo.
     *
     * Asigna el @c ID3D11InputLayout al pipeline gráfico a través de
     * @c ID3D11DeviceContext::IASetInputLayout.
     *
     * @param deviceContext Contexto donde se establecerá el Input Layout.
     *
     * @pre @c m_inputLayout debe haberse creado con init().
     */
    void
        render(DeviceContext& deviceContext);

    /**
     * @brief Libera el recurso @c ID3D11InputLayout y deja la instancia en estado no inicializado.
     *
     * Idempotente: puede llamarse múltiples veces de forma segura.
     *
     * @post @c m_inputLayout == nullptr.
     */
    void
        destroy();

public:
    /**
     * @brief Recurso COM de Direct3D 11 que representa el Input Layout.
     * @details Válido tras init(); @c nullptr después de destroy().
     */
    ID3D11InputLayout* m_inputLayout = nullptr;
};
