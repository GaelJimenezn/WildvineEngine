#pragma once
#include "Prerequisites.h"

/**
 * @class DeviceContext
 * @brief Clase DeviceContext definida en el motor WildvineEngine.
 *
 * Esta clase gestiona la funcionalidad correspondiente a DeviceContext
 * dentro de la arquitectura del motor gráfico.
 */
class
	DeviceContext {
public:
    /**
     * @brief Método DeviceContext.
     */
    DeviceContext() = default;
    /**
     * @brief Método ~DeviceContext.
     */
    ~DeviceContext() = default;

    /**
   * @brief Inicializa el contexto del dispositivo.
   *
   * Este método se utiliza como punto de inicialización; la implementación
   * puede asociar el contexto inmediato creado junto con @c ID3D11Device.
   */
    void
    	init();

    /**
     * @brief Actualiza parámetros internos del contexto.
     * @note Método placeholder, útil para extender funcionalidades.
     */
    void
    	update();

    /**
     * @brief Ejecuta operaciones relacionadas con render.
     * @note Método placeholder; puede ser usado para depuración.
     */
    void
    	render();

    /**
     * @brief Libera el recurso @c ID3D11DeviceContext.
     *
     * @post @c m_deviceContext == nullptr.
     */
    void
    	destroy();

    /**
     * @brief Configura los viewports en la etapa de rasterización.
     *
     * @param NumViewports Número de viewports.
     * @param pViewports   Puntero a un arreglo de descriptores de viewport.
     */
    void
    	RSSetViewports(unsigned int NumViewports, const D3D11_VIEWPORT* pViewports);

    /**
     * @brief Asigna Shader Resource Views a la etapa de Pixel Shader.
     *
     * @param StartSlot             Slot inicial.
     * @param NumViews              Número de vistas a asignar.
     * @param ppShaderResourceViews Arreglo de vistas de recurso de shader.
     */
    void
    	PSSetShaderResources(unsigned int StartSlot,
            unsigned int NumViews,
            ID3D11ShaderResourceView* const* ppShaderResourceViews);

    /**
     * @brief Define el Input Layout activo en la etapa de ensamblado de entrada.
     *
     * @param pInputLayout Input layout a asignar.
     */
    void
    	IASetInputLayout(ID3D11InputLayout* pInputLayout);

    /**
     * @brief Asigna un Vertex Shader al pipeline.
     *
     * @param pVertexShader   Puntero al vertex shader.
     * @param ppClassInstances Instancias de clase (opcional).
     * @param NumClassInstances Número de instancias de clase.
     */
    void
    	VSSetShader(ID3D11VertexShader* pVertexShader,
            ID3D11ClassInstance* const* ppClassInstances,
            unsigned int NumClassInstances);

    /**
     * @brief Asigna un Pixel Shader al pipeline.
     *
     * @param pPixelShader     Puntero al pixel shader.
     * @param ppClassInstances Instancias de clase (opcional).
     * @param NumClassInstances Número de instancias de clase.
     */
    void
    	PSSetShader(ID3D11PixelShader* pPixelShader,
            ID3D11ClassInstance* const* ppClassInstances,
            unsigned int NumClassInstances);

    /**
     * @brief Copia datos desde CPU hacia un recurso en GPU.
     *
     * @param pDstResource   Recurso destino.
     * @param DstSubresource Índice de subrecurso.
     * @param pDstBox        Región destino (puede ser nullptr).
     * @param pSrcData       Datos fuente.
     * @param SrcRowPitch    Tamaño por fila.
     * @param SrcDepthPitch  Tamaño por capa de profundidad.
     */
    void
    	UpdateSubresource(ID3D11Resource* pDstResource,
            unsigned int DstSubresource,
            const D3D11_BOX* pDstBox,
            const void* pSrcData,
            unsigned int SrcRowPitch,
            unsigned int SrcDepthPitch);

    /**
     * @brief Asigna buffers de vértices a la etapa de ensamblado de entrada.
     *
     * @param StartSlot       Slot inicial.
     * @param NumBuffers      Número de buffers.
     * @param ppVertexBuffers Arreglo de punteros a vertex buffers.
     * @param pStrides        Arreglo con tamaños de cada vértice.
     * @param pOffsets        Arreglo con offsets iniciales.
     */
    void
    	IASetVertexBuffers(unsigned int StartSlot,
            unsigned int NumBuffers,
            ID3D11Buffer* const* ppVertexBuffers,
            const unsigned int* pStrides,
            const unsigned int* pOffsets);

    /**
     * @brief Asigna un Index Buffer a la etapa de ensamblado de entrada.
     *
     * @param pIndexBuffer Buffer de índices.
     * @param Format       Formato de índice (ej. DXGI_FORMAT_R16_UINT).
     * @param Offset       Offset inicial en bytes.
     */
    void
    	IASetIndexBuffer(ID3D11Buffer* pIndexBuffer,
            DXGI_FORMAT Format,
            unsigned int Offset);

    /**
     * @brief Asigna Sampler States a la etapa de Pixel Shader.
     *
     * @param StartSlot   Slot inicial.
     * @param NumSamplers Número de samplers.
     * @param ppSamplers  Arreglo de sampler states.
     */
    void
    	PSSetSamplers(unsigned int StartSlot,
            unsigned int NumSamplers,
            ID3D11SamplerState* const* ppSamplers);

    /**
     * @brief Configura el Rasterizer State actual.
     *
     * @param pRasterizerState Estado de rasterización.
     */
    void
    	RSSetState(ID3D11RasterizerState* pRasterizerState);

    /**
     * @brief Asigna un Blend State al Output Merger.
     *
     * @param pBlendState Estado de blending.
     * @param BlendFactor Factor de mezcla (RGBA).
     * @param SampleMask  Máscara de muestras.
     */
    void
    	OMSetBlendState(ID3D11BlendState* pBlendState,
            const float BlendFactor[4],
            unsigned int SampleMask);

    /**
     * @brief Asigna Render Targets y Depth Stencil al Output Merger.
     *
     * @param NumViews            Número de render targets.
     * @param ppRenderTargetViews Arreglo de render target views.
     * @param pDepthStencilView   Vista de profundidad/esténcil.
     */
    void
    	OMSetRenderTargets(unsigned int NumViews,
            ID3D11RenderTargetView* const* ppRenderTargetViews,
            ID3D11DepthStencilView* pDepthStencilView);

    /**
     * @brief Define la topología de primitivas a renderizar.
     *
     * @param Topology Tipo de topología (ej. D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST).
     */
    void
    	IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY Topology);

    /**
     * @brief Limpia un Render Target con un color dado.
     *
     * @param pRenderTargetView Render target a limpiar.
     * @param ColorRGBA         Color en formato RGBA.
     */
    void
    	ClearRenderTargetView(ID3D11RenderTargetView* pRenderTargetView,
            const float ColorRGBA[4]);

    /**
     * @brief Limpia un Depth Stencil View.
     *
     * @param pDepthStencilView Vista a limpiar.
     * @param ClearFlags        Flags de limpieza (D3D11_CLEAR_DEPTH /
     * D3D11_CLEAR_STENCIL).
     * @param Depth             Valor de profundidad (0.0 - 1.0).
     * @param Stencil           Valor de esténcil.
     */
    void
    	ClearDepthStencilView(ID3D11DepthStencilView* pDepthStencilView,
            unsigned int ClearFlags,
            float Depth,
            UINT8 Stencil);

    /**
     * @brief Asigna constant buffers a la etapa de Vertex Shader.
     *
     * @param StartSlot       Slot inicial.
     * @param NumBuffers      Número de buffers.
     * @param ppConstantBuffers Arreglo de constant buffers.
     */
    void
    	VSSetConstantBuffers(unsigned int StartSlot,
            unsigned int NumBuffers,
            ID3D11Buffer* const* ppConstantBuffers);

    /**
     * @brief Asigna constant buffers a la etapa de Pixel Shader.
     *
     * @param StartSlot       Slot inicial.
     * @param NumBuffers      Número de buffers.
     * @param ppConstantBuffers Arreglo de constant buffers.
     */
    void
    	PSSetConstantBuffers(unsigned int StartSlot,
            unsigned int NumBuffers,
            ID3D11Buffer* const* ppConstantBuffers);

    /**
     * @brief Envía un comando de dibujado de primitivas indexadas.
     *
     * @param IndexCount         Número de índices a renderizar.
     * @param StartIndexLocation Posición inicial en el buffer de índices.
     * @param BaseVertexLocation Offset aplicado a los vértices.
     */
    void
    	DrawIndexed(unsigned int IndexCount,
            unsigned int StartIndexLocation,
            int BaseVertexLocation);
public:
    /**
     * @brief Puntero al contexto inmediato de Direct3D 11.
     * @details Válido tras init(); liberado en destroy().
     */
    ID3D11DeviceContext* m_deviceContext = nullptr;

};
