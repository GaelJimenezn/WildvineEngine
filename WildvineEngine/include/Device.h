#pragma once
#include "Prerequisites.h"

/**
 * @class Device
 * @brief Encapsula un @c ID3D11Device y facilita la creación de recursos gráficos en Direct3D 11.
 *
 * Esta clase actúa como un contenedor del objeto @c ID3D11Device. Expone métodos
 * que encapsulan la creación de vistas, texturas, shaders, estados y buffers
 * para simplificar el manejo del ciclo de vida de los recursos gráficos.
 *
 * @note No administra automáticamente @c ID3D11DeviceContext.
 * @warning Asegurarse de inicializar la instancia con init() antes de crear recursos.
 */
class
    Device {
public:
    Device() = default;
    ~Device() = default;

    void
        init();

    void
        update();

    void
        render();

    void
        destroy();

    /**
   * @brief Crea una Render Target View.
   *
   * @param pResource Recurso de origen (usualmente una textura).
   * @param pDesc     Descriptor de la RTV (puede ser @c nullptr para usar valores por defecto).
   * @param ppRTView  Puntero de salida donde se guarda la RTV creada.
   * @return @c S_OK si fue exitoso; código @c HRESULT en caso de error.
   */
    HRESULT
        CreateRenderTargetView(ID3D11Resource* pResource,
            const D3D11_RENDER_TARGET_VIEW_DESC* pDesc,
            ID3D11RenderTargetView** ppRTView);

    /**
     * @brief Crea una textura 2D.
     *
     * @param pDesc        Descriptor de la textura.
     * @param pInitialData Datos iniciales opcionales (puede ser @c nullptr).
     * @param ppTexture2D  Puntero de salida a la textura creada.
     */
    HRESULT
        CreateTexture2D(const D3D11_TEXTURE2D_DESC* pDesc,
            const D3D11_SUBRESOURCE_DATA* pInitialData,
            ID3D11Texture2D** ppTexture2D);

    /**
     * @brief Crea una Depth Stencil View.
     *
     * @param pResource Recurso asociado (textura con flag @c D3D11_BIND_DEPTH_STENCIL).
     * @param pDesc     Descriptor de la DSV (puede ser @c nullptr).
     * @param ppDepthStencilView Puntero de salida.
     */
    HRESULT
        CreateDepthStencilView(ID3D11Resource* pResource,
            const D3D11_DEPTH_STENCIL_VIEW_DESC* pDesc,
            ID3D11DepthStencilView** ppDepthStencilView);

    /**
     * @brief Crea un Vertex Shader.
     *
     * @param pShaderBytecode Bytecode compilado del shader.
     * @param BytecodeLength  Tamaño del bytecode.
     * @param pClassLinkage   Enlace de clases (opcional).
     * @param ppVertexShader  Puntero de salida al vertex shader.
     */
    HRESULT
        CreateVertexShader(const void* pShaderBytecode,
            unsigned int BytecodeLength,
            ID3D11ClassLinkage* pClassLinkage,
            ID3D11VertexShader** ppVertexShader);

    /**
     * @brief Crea un Input Layout.
     *
     * @param pInputElementDescs Descriptores de entrada.
     * @param NumElements        Número de elementos en el arreglo.
     * @param pShaderBytecodeWithInputSignature Bytecode del shader que define la firma.
     * @param BytecodeLength     Tamaño del bytecode.
     * @param ppInputLayout      Puntero de salida al input layout.
     */
    HRESULT
        CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC* pInputElementDescs,
            unsigned int NumElements,
            const void* pShaderBytecodeWithInputSignature,
            unsigned int BytecodeLength,
            ID3D11InputLayout** ppInputLayout);

    /**
     * @brief Crea un Pixel Shader.
     *
     * @param pShaderBytecode Bytecode compilado del shader.
     * @param BytecodeLength  Tamaño del bytecode.
     * @param pClassLinkage   Enlace de clases (opcional).
     * @param ppPixelShader   Puntero de salida al pixel shader.
     */
    HRESULT
        CreatePixelShader(const void* pShaderBytecode,
            unsigned int BytecodeLength,
            ID3D11ClassLinkage* pClassLinkage,
            ID3D11PixelShader** ppPixelShader);

    /**
     * @brief Crea un Buffer genérico (vertex, index, constant, etc.).
     *
     * @param pDesc        Descriptor del buffer.
     * @param pInitialData Datos iniciales opcionales (puede ser @c nullptr).
     * @param ppBuffer     Puntero de salida al buffer creado.
     */
    HRESULT
        CreateBuffer(const D3D11_BUFFER_DESC* pDesc,
            const D3D11_SUBRESOURCE_DATA* pInitialData,
            ID3D11Buffer** ppBuffer);

    /**
     * @brief Crea un Sampler State.
     *
     * @param pSamplerDesc  Descriptor del sampler.
     * @param ppSamplerState Puntero de salida al estado de muestreo.
     */
    HRESULT
        CreateSamplerState(const D3D11_SAMPLER_DESC* pSamplerDesc,
            ID3D11SamplerState** ppSamplerState);

public:
    /**
     * @brief Puntero al dispositivo Direct3D 11.
     * @details Creado en init(), liberado en destroy().
     */
    ID3D11Device* m_device = nullptr;
};