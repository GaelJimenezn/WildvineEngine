#pragma once
#include "Prerequisites.h"

/**
 * @brief Encapsula el contexto de dispositivo de DirectX 11.
 *
 * La clase DeviceContext se encarga de administrar los estados,
 * buffers, shaders y recursos asociados al pipeline de renderizado.
 */
class
DeviceContext {
public:
  /**
   * @brief Constructor por defecto.
   */
  DeviceContext() = default;

  /**
   * @brief Destructor por defecto.
   */
  ~DeviceContext() = default;

  /**
   * @brief Inicializa el contexto de dispositivo.
   */
  void
  init();

  /**
   * @brief Actualiza el estado del contexto de dispositivo.
   */
  void
  update();

  /**
   * @brief Ejecuta las operaciones de renderizado con el contexto.
   */
  void
  render();

  /**
   * @brief Libera los recursos asociados al contexto.
   */
  void
  destroy();

  
  /**
   * @brief Define los viewports activos en el rasterizador.
   *
   * @param NumViewports N�mero de viewports.
   * @param pViewports Array de viewports.
   */
  void
  RSSetViewports(unsigned int NumViewports,
                 const D3D11_VIEWPORT* pViewports);

  /**
   * @brief Asigna recursos de textura a la etapa de pixel shader.
   *
   * @param StartSlot Slot inicial.
   * @param NumViews N�mero de vistas.
   * @param ppShaderResourceViews Array de vistas de recursos.
   */
  void
  PSSetShaderResources(unsigned int StartSlot,
                      unsigned int NumViews,
                      ID3D11ShaderResourceView* const* ppShaderResourceViews);

  /**
   * @brief Establece el Input Layout para la etapa de entrada de ensamblaje.
   *
   * @param pInputLayout Puntero al input layout.
   */
  void
  IASetInputLayout(ID3D11InputLayout* pInputLayout);


  /**
   * @brief Asigna un shader de v�rtices al pipeline.
   *
   * @param pVertexShader Shader de v�rtices.
   * @param ppClassInstances Array de instancias de clase.
   * @param NumClassInstances N�mero de instancias de clase.
   */
  void
  VSSetShader(ID3D11VertexShader* pVertexShader,
              ID3D11ClassInstance* const* ppClassInstances,
             unsigned int NumClassInstances);
  /**
   * @brief Asigna un shader de p�xeles al pipeline.
   *
   * @param pPixelShader Shader de p�xeles.
   * @param ppClassInstances Array de instancias de clase.
   * @param NumClassInstances N�mero de instancias de clase.
   */
  void
  PSSetShader(ID3D11PixelShader* pPixelShader,
              ID3D11ClassInstance* const* ppClassInstances,
              unsigned int NumClassInstances);

   /**
   * @brief Copia datos desde memoria de CPU a un recurso de GPU.
   *
   * @param pDstResource Recurso de destino.
   * @param DstSubresource Subrecurso de destino.
   * @param pDstBox Regi�n a actualizar (puede ser nullptr).
   * @param pSrcData Puntero a los datos de origen.
   * @param SrcRowPitch N�mero de bytes por fila de datos.
   * @param SrcDepthPitch N�mero de bytes por capa de datos.
   */
  void
  UpdateSubresource(ID3D11Resource* pDstResource,
                    unsigned int DstSubresource,
                    const D3D11_BOX* pDstBox,
                    const void* pSrcData,
                    unsigned int SrcRowPitch,
                    unsigned int SrcDepthPitch);

  /**
   * @brief Asigna buffers de v�rtices al pipeline.
   *
   * @param StartSlot Slot inicial.
   * @param NumBuffers N�mero de buffers.
   * @param ppVertexBuffers Array de buffers de v�rtices.
   * @param pStrides Array con el tama�o de cada v�rtice.
   * @param pOffsets Array con los desplazamientos iniciales.
   */
  void
  IASetVertexBuffers(unsigned int StartSlot,
                    unsigned int NumBuffers,
                    ID3D11Buffer* const* ppVertexBuffers,
                    const unsigned int* pStrides,
                    const unsigned int* pOffsets);

  /**
   * @brief Asigna un buffer de �ndices al pipeline.
   *
   * @param pIndexBuffer Buffer de �ndices.
   * @param Format Formato de los �ndices.
   * @param Offset Desplazamiento inicial.
   */
  void
  IASetIndexBuffer(ID3D11Buffer* pIndexBuffer,
                    DXGI_FORMAT Format,
                    unsigned int Offset);

   /**
   * @brief Asigna estados de muestreo a la etapa de pixel shader.
   *
   * @param StartSlot Slot inicial.
   * @param NumSamplers N�mero de samplers.
   * @param ppSamplers Array de estados de muestreo.
   */
  void
  PSSetSamplers(unsigned int StartSlot,
                unsigned int NumSamplers,
                ID3D11SamplerState* const* ppSamplers);

  /**
   * @brief Establece el estado del rasterizador.
   *
   * @param pRasterizerState Estado del rasterizador.
   */
  void
  RSSetState(ID3D11RasterizerState* pRasterizerState);

  /**
   * @brief Establece el estado de blending para el pipeline.
   *
   * @param pBlendState Estado de blending.
   * @param BlendFactor Factores de mezcla RGBA.
   * @param SampleMask M�scara de muestras.
   */
  void
  OMSetBlendState(ID3D11BlendState* pBlendState,
                  const float BlendFactor[4],
                  unsigned int SampleMask);

  /**
   * @brief Establece los render targets y el depth-stencil en el pipeline.
   *
   * @param NumViews N�mero de vistas de render.
   * @param ppRenderTargetViews Array de vistas de render target.
   * @param pDepthStencilView Vista de profundidad/stencil.
   */
  void
  OMSetRenderTargets(unsigned int NumViews,
                      ID3D11RenderTargetView* const* ppRenderTargetViews,
                      ID3D11DepthStencilView* pDepthStencilView);

  /**
   * @brief Establece la topolog�a de primitivas para el ensamblador de entrada.
   *
   * @param Topology Tipo de primitiva (ejemplo: tri�ngulos, l�neas).
   */
  void
  IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY Topology);

  
  /**
   * @brief Limpia un render target con un color espec�fico.
   *
   * @param pRenderTargetView Vista del render target.
   * @param ColorRGBA Array de 4 floats con el color RGBA.
   */
  void
  ClearRenderTargetView(ID3D11RenderTargetView* pRenderTargetView,
                        const float ColorRGBA[4]);

  /**
   * @brief Limpia un buffer de profundidad y stencil.
   *
   * @param pDepthStencilView Vista de profundidad/stencil.
   * @param ClearFlags Banderas de limpieza (ejemplo: profundidad, stencil).
   * @param Depth Valor de profundidad inicial.
   * @param Stencil Valor de stencil inicial.
   */
  void
  ClearDepthStencilView(ID3D11DepthStencilView* pDepthStencilView,
                        unsigned int ClearFlags,
                        float Depth,
                        UINT8 Stencil);

 

  /**
   * @brief Asigna buffers constantes a la etapa de vertex shader.
   *
   * @param StartSlot Slot inicial.
   * @param NumBuffers N�mero de buffers.
   * @param ppConstantBuffers Array de buffers constantes.
   */
  void
  VSSetConstantBuffers(unsigned int StartSlot,
                        unsigned int NumBuffers,
                        ID3D11Buffer* const* ppConstantBuffers);

  

  /**
   * @brief Asigna buffers constantes a la etapa de pixel shader.
   *
   * @param StartSlot Slot inicial.
   * @param NumBuffers N�mero de buffers.
   * @param ppConstantBuffers Array de buffers constantes.
   */
  void
  PSSetConstantBuffers(unsigned int StartSlot,
                       unsigned int NumBuffers,
                        ID3D11Buffer* const* ppConstantBuffers);

  

  
  /**
   * @brief Dibuja primitivas indexadas.
   *
   * @param IndexCount N�mero de �ndices a dibujar.
   * @param StartIndexLocation �ndice inicial.
   * @param BaseVertexLocation Desplazamiento base de v�rtices.
   */
  void
  DrawIndexed(unsigned int IndexCount,
              unsigned int StartIndexLocation,
              int BaseVertexLocation);

 
  
public:
ID3D11DeviceContext* m_deviceContext = nullptr; /**< Puntero al contexto de dispositivo de DirectX. */
unsigned int m_drawCallCount = 0; /**< Contador de draw calls por frame. */
};
