#pragma once
#include "Prerequisites.h"

class
  Device;

class
  DeviceContext;

/**
 * @class Texture
 * @brief Representa una textura en DirectX 11.
 *
 * Esta clase encapsula la creación, gestión y destrucción de texturas 2D
 * en DirectX, así como su vinculación al pipeline gráfico. Puede inicializarse
 * desde archivo, como un recurso en memoria, o copiando otra textura.
 */
class
  Texture {
public:
  /**
   * @brief Constructor por defecto.
   */
  Texture() = default;

  /**
   * @brief Destructor por defecto.
   * @details No libera automáticamente los recursos COM; llamar a destroy().
   */
  ~Texture() = default;

  /**
   * @brief Inicializa la textura desde un archivo de imagen.
   *
   * @param device Referencia al dispositivo de DirectX.
   * @param textureName Nombre o ruta del archivo de la textura.
   * @param extensionType Tipo de extensión de la textura (ej. PNG, JPG).
   * @return HRESULT Código de resultado (S_OK si se cargó correctamente).
   *
   * @post Si retorna @c S_OK, @c m_texture y
   *       @c m_textureFromImg != nullptr.
   */
  HRESULT
    init(Device& device,
      const std::string& textureName,
      ExtensionType extensionType);

  HRESULT
    initFromMemory(Device& device,
      const unsigned char* data,
      size_t size,
      const std::string& textureName);

  /**
   * @brief Inicializa la textura como un recurso vacío en memoria.
   *
   * @param device Referencia al dispositivo de DirectX.
   * @param width Ancho de la textura.
   * @param height Alto de la textura.
   * @param Format Formato de la textura (DXGI_FORMAT).
   * @param BindFlags Banderas de enlace (ej. render target, shader resource).
   * @param sampleCount Número de muestras para multisampling (default = 1).
   * @param qualityLevels Niveles de calidad para multisampling (default = 0).
   * @return HRESULT Código de resultado (S_OK si se creó correctamente).
   */
  HRESULT
    init(Device& device,
      unsigned int width,
      unsigned int height,
      DXGI_FORMAT Format,
      unsigned int BindFlags,
      unsigned int sampleCount = 1,
      unsigned int qualityLevels = 0);

  /**
   * @brief Inicializa la textura copiando desde otra textura existente.
   *
   * @param device Referencia al dispositivo de DirectX.
   * @param textureRef Textura de referencia para crear la nueva.
   * @param format Formato de la textura (DXGI_FORMAT).
   * @return HRESULT Código de resultado.
   */
  HRESULT
    init(Device& device,
      Texture& textureRef,
      DXGI_FORMAT format);

  /**
   * @brief Actualiza el estado de la textura.
   *
   * Método de marcador para lógica de actualización de texturas.
   *
   * @note Actualmente no realiza ninguna operación.
   */
  void
    update();

  /**
   * @brief Renderiza la textura en el pipeline gráfico.
   *
   * @param deviceContext Contexto del dispositivo de DirectX.
   * @param StartSlot Slot de inicio donde se asignará la textura.
   * @param NumViews Número de vistas de recurso de shader a asignar.
   *
   * @pre @c m_textureFromImg debe haberse creado con init().
   */
  void
    render(DeviceContext& deviceContext,
      unsigned int StartSlot,
      unsigned int NumViews);

  /**
   * @brief Libera los recursos asociados a la textura.
   *
   * @post @c m_texture == nullptr y
   *       @c m_textureFromImg == nullptr.
   */
  void
    destroy();

  /**
   * @brief Crea un cubemap utilizando seis imágenes.
   *
   * @param device Referencia al dispositivo de DirectX.
   * @param deviceContext Contexto del dispositivo.
   * @param facePaths Arreglo con las rutas de las seis caras del cubemap.
   * @param generateMips Indica si se deben generar mipmaps.
   * @return HRESULT Código de resultado.
   */
  HRESULT
    CreateCubemap(Device& device,
      DeviceContext& deviceContext,
      const std::array<std::string, 6>& facePaths,
      bool generateMips /*= false*/);

  /**
   * @brief Crea una Shader Resource View para una cara específica del cubemap.
   *
   * @param device Dispositivo de DirectX utilizado para crear la vista.
   * @param cubemapTex Textura del cubemap.
   * @param format Formato DXGI de la textura.
   * @param faceIndex Índice de la cara del cubemap.
   * @param mipLevels Número de mip levels.
   * @return ID3D11ShaderResourceView* Vista creada; nullptr en caso de error.
   */
  ID3D11ShaderResourceView*
    CreateCubemapFaceSRV(
      ID3D11Device* device,
      ID3D11Texture2D* cubemapTex,
      DXGI_FORMAT format,
      UINT faceIndex,
      UINT mipLevels = 1
    )
  {
    D3D11_SHADER_RESOURCE_VIEW_DESC d{};
    d.Format = format;
    d.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
    d.Texture2DArray.MostDetailedMip = 0;
    d.Texture2DArray.MipLevels = mipLevels;
    d.Texture2DArray.FirstArraySlice = faceIndex;
    d.Texture2DArray.ArraySize = 1;

    ID3D11ShaderResourceView* srv = nullptr;

    if (FAILED(device->CreateShaderResourceView(cubemapTex, &d, &srv)))
      return nullptr;

    return srv;
  }

public:
  /**
   * @brief Puntero al recurso de textura 2D en DirectX 11.
   */
  ID3D11Texture2D* m_texture = nullptr;

  /**
   * @brief Vista de recurso de shader creada a partir de la textura.
   */
  ID3D11ShaderResourceView* m_textureFromImg = nullptr;

  /**
   * @brief Nombre o ruta de la textura cargada.
   */
  std::string m_textureName;
};