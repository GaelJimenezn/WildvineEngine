/**
 * @file Buffer.h
 * @brief Declara la API pÃºblica de Buffer dentro de WildvineEngine.
 */
#pragma once
#include "Prerequisites.h"
#include "MeshComponent.h"

/**
 * @brief DeclaraciÃ³n adelantada de la clase Device.
 */
class Device;

/**
 * @brief DeclaraciÃ³n adelantada de la clase DeviceContext.
 */
class DeviceContext;

/**
 * @class Buffer
 * @brief Clase encargada de manejar la creaciÃ³n, actualizaciÃ³n, renderizado
 *        y destrucciÃ³n de buffers utilizados en DirectX.
 */
class
Buffer {
public:
  /**
   * @brief Constructor por defecto de la clase Buffer.
   */
  Buffer() = default;

  /**
   * @brief Destructor por defecto de la clase Buffer.
   */
  ~Buffer() = default;

  /**
   * @brief Inicializa el buffer utilizando los datos de un componente de malla.
   * @param device Referencia al dispositivo de renderizado.
   * @param mesh Referencia al componente de malla que contiene los datos del modelo.
   * @param bindFlag Indicador de tipo de enlace (por ejemplo, vÃ©rtices o Ã­ndices).
   * @return HRESULT que indica el resultado de la operaciÃ³n.
   */
  HRESULT
  init(Device &device, const MeshComponent &mesh, unsigned int bindFlag);

  /**
   * @brief Inicializa un buffer vacÃ­o con un tamaÃ±o en bytes determinado.
   * @param device Referencia al dispositivo de renderizado.
   * @param ByteWidth TamaÃ±o del buffer en bytes.
   * @return HRESULT que indica el resultado de la creaciÃ³n.
   */
  HRESULT
  init(Device &device, unsigned int ByteWidth);

  /**
   * @brief Actualiza el contenido del buffer con nuevos datos.
   * @param deviceContext Contexto del dispositivo para la actualizaciÃ³n.
   * @param pDstResource Recurso de destino a actualizar.
   * @param DstSubresource Ãndice del subrecurso de destino.
   * @param pDstBox Caja que define el Ã¡rea del recurso a actualizar.
   * @param pSrcData Puntero a los datos fuente.
   * @param SrcRowPitch TamaÃ±o de la fila de datos fuente.
   * @param SrcDepthPitch TamaÃ±o de la profundidad de los datos fuente.
   */
  void update(DeviceContext &deviceContext,
              ID3D11Resource *pDstResource,
              unsigned int DstSubresource,
              const D3D11_BOX *pDstBox,
              const void *pSrcData,
              unsigned int SrcRowPitch,
              unsigned int SrcDepthPitch);

  /**
   * @brief Asocia el buffer al pipeline para su renderizado.
   * @param deviceContext Contexto del dispositivo para el renderizado.
   * @param StartSlot PosiciÃ³n inicial del buffer en el pipeline.
   * @param NumBuffers NÃºmero de buffers a establecer.
   * @param setPixelShader Indica si el buffer se usa tambiÃ©n en el pixel shader.
   * @param format Formato DXGI utilizado por el buffer (opcional).
   */
  void render(DeviceContext &deviceContext,
              unsigned int StartSlot,
              unsigned int NumBuffers,
              bool setPixelShader = false,
              DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN);

  /**
   * @brief Libera los recursos asociados al buffer.
   */
  void destroy();

  /**
   * @brief Crea un buffer con la descripciÃ³n y los datos iniciales proporcionados.
   * @param device Referencia al dispositivo de renderizado.
   * @param desc DescripciÃ³n del buffer (tipo, tamaÃ±o, uso, etc.).
   * @param initData Datos iniciales para llenar el buffer (puede ser nullptr).
   * @return HRESULT que indica el resultado de la operaciÃ³n.
   */
  HRESULT
  createBuffer(Device &device, D3D11_BUFFER_DESC &desc, D3D11_SUBRESOURCE_DATA *initData);

public:
  /** @brief Puntero al buffer de Direct3D. */
  ID3D11Buffer *m_buffer = nullptr;

private:
  /** @brief TamaÃ±o en bytes de cada elemento del buffer (stride). */
  unsigned int m_stride = 0;

  /** @brief Desplazamiento en bytes desde el inicio del buffer. */
  unsigned int m_offset = 0;

  /** @brief Bandera que indica el tipo de enlace (bind flag) del buffer. */
  unsigned int m_bindFlag = 0;
};
