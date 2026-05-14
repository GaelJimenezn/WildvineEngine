#pragma once
#include "Prerequisites.h"
#include "InputLayout.h"

class Device;
class DeviceContext;
class LayoutBuilder;

/**
 * @class ShaderProgram
 * @brief Encapsula la creación, compilación y uso de Vertex Shader y Pixel Shader en Direct3D 11.
 *
 * Esta clase administra el ciclo de vida de un conjunto de shaders (VS y PS),
 * incluyendo su compilación desde archivo, creación en el dispositivo y vinculación
 * al pipeline. Además, maneja el Input Layout asociado al Vertex Shader.
 */
class
    ShaderProgram {
public:
    /**
     * @brief Constructor por defecto.
     */
    ShaderProgram() = default;

    /**
     * @brief Destructor por defecto.
     * @details No libera automáticamente los recursos COM; llamar a destroy().
     */
    ~ShaderProgram() = default;

    /**
     * @brief Inicializa el programa de shaders desde un archivo HLSL.
     *
     * Compila y crea los shaders (VS y PS) definidos en el archivo indicado,
     * además de crear el Input Layout con la descripción proporcionada.
     *
     * @param device   Dispositivo con el que se crearán los recursos.
     * @param fileName Nombre del archivo HLSL que contiene los shaders.
     * @param Layout   Vector con la descripción de los elementos de entrada (para VS).
     * @return @c S_OK si fue exitoso; código @c HRESULT en caso de error.
     *
     * @post Si retorna @c S_OK, los punteros a shaders y el input layout serán válidos.
     */
    HRESULT
        init(Device& device, const std::string& fileName, LayoutBuilder layoutBuilder);

    /**
     * @brief Actualiza parámetros internos de los shaders.
     *
     * Método de marcador para futuras extensiones (por ejemplo,
     * recompilar shaders en caliente).
     *
     * @note Actualmente no realiza ninguna operación.
     */
    void
        update();

    /**
     * @brief Aplica el Vertex Shader, Pixel Shader e Input Layout al pipeline.
     *
     * Llama a @c VSSetShader, @c PSSetShader y asigna el input layout
     * al contexto.
     *
     * @param deviceContext Contexto donde se aplicará el programa de shaders.
     *
     * @pre Los shaders deben haberse creado con init() o CreateShader().
     */
    void
        render(DeviceContext& deviceContext);

    /**
     * @brief Aplica únicamente un shader específico al pipeline.
     *
     * Permite vincular solo el Vertex Shader o solo el Pixel Shader,
     * según el parámetro @p type.
     *
     * @param deviceContext Contexto donde se aplicará el shader.
     * @param type          Tipo de shader a establecer (VS o PS).
     */
    void
        render(DeviceContext& deviceContext, ShaderType type);

    /**
     * @brief Libera todos los recursos asociados (shaders, blobs e input layout).
     *
     * @post @c m_VertexShader == nullptr, @c m_PixelShader == nullptr,
     *       @c m_vertexShaderData == nullptr, @c m_pixelShaderData == nullptr.
     */
    void
        destroy();

    /**
     * @brief Crea un Input Layout asociado al Vertex Shader.
     *
     * @param device Dispositivo con el que se creará el recurso.
     * @param Layout Descripción de los elementos de entrada.
     * @return @c S_OK si fue exitoso; código @c HRESULT en caso de error.
     */
    HRESULT
        CreateInputLayout(Device& device, LayoutBuilder);

    /**
     * @brief Crea un shader (Vertex o Pixel) a partir del archivo establecido en @c m_shaderFileName.
     *
     * @param device Dispositivo con el que se creará el recurso.
     * @param type   Tipo de shader a crear.
     * @return @c S_OK si fue exitoso; código @c HRESULT en caso de error.
     */
    HRESULT
        CreateShader(Device& device, ShaderType type);

    /**
     * @brief Crea un shader (Vertex o Pixel) a partir de un archivo HLSL.
     *
     * @param device   Dispositivo con el que se creará el recurso.
     * @param type     Tipo de shader a crear.
     * @param fileName Nombre del archivo HLSL.
     * @return @c S_OK si fue exitoso; código @c HRESULT en caso de error.
     */
    HRESULT
        CreateShader(Device& device, ShaderType type, const std::string& fileName);

    /**
     * @brief Compila un shader desde archivo.
     *
     * Llama internamente a @c D3DCompileFromFile para obtener el bytecode
     * de un shader en función de su punto de entrada y modelo.
     *
     * @param szFileName   Ruta del archivo HLSL.
     * @param szEntryPoint Punto de entrada de la función shader (ej. "VSMain").
     * @param szShaderModel Modelo de shader (ej. "vs_5_0", "ps_5_0").
     * @param ppBlobOut    Salida con el bytecode compilado.
     * @return @c S_OK si fue exitoso; código @c HRESULT en caso de error.
     */
    HRESULT
        CompileShaderFromFile(char* szFileName,
            LPCSTR szEntryPoint,
            LPCSTR szShaderModel,
            ID3DBlob** ppBlobOut);

public:
    /**
     * @brief Vertex Shader compilado y creado en GPU.
     */
    ID3D11VertexShader* m_VertexShader = nullptr;

    /**
     * @brief Pixel Shader compilado y creado en GPU.
     */
    ID3D11PixelShader* m_PixelShader = nullptr;

    /**
     * @brief Input Layout asociado al Vertex Shader.
     */
    InputLayout m_inputLayout;

private:
    /**
     * @brief Nombre del archivo HLSL asociado a este programa de shaders.
     */
    std::string m_shaderFileName;

    /**
     * @brief Bytecode compilado del Vertex Shader.
     */
    ID3DBlob* m_vertexShaderData = nullptr;

    /**
     * @brief Bytecode compilado del Pixel Shader.
     */
    ID3DBlob* m_pixelShaderData = nullptr;
};
