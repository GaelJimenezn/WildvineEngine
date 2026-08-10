/**
 * @file Texture.h
 * @brief Declara la API pÃºblica de Texture dentro de WildvineEngine.
 */
#pragma once
#include "Prerequisites.h"

/** @brief Declara class Device. */
class Device;

/** @brief Declara class DeviceContext. */
class DeviceContext;

/**
 * @class Texture
 * @brief Representa una textura en DirectX 11.
 *
 * Esta clase encapsula la creaciÃ³n, gestiÃ³n y destrucciÃ³n de texturas 2D
 * en DirectX, asÃ­ como su vinculaciÃ³n al pipeline grÃ¡fico. Puede inicializarse
 * desde archivo, como un recurso en memoria, o copiando otra textura.
 */
class Texture {
public:
  /**
   * @brief Constructor por defecto.
   */
  Texture() = default;

  /**
   * @brief Destructor por defecto.
   * @details No libera automÃ¡ticamente los recursos COM; llamar a destroy().
   */
  ~Texture() = default;

  /**
   * @brief Inicializa la textura desde un archivo de imagen.
   *
   * @param device Referencia al dispositivo de DirectX.
   * @param textureName Nombre o ruta del archivo de la textura.
   * @param extensionType Tipo de extensiÃ³n de la textura (ej. PNG, JPG).
   * @return HRESULT CÃ³digo de resultado (S_OK si se cargÃ³ correctamente).
   *
   * @post Si retorna @c S_OK, @c m_texture y
   *       @c m_textureFromImg != nullptr.
   */
  HRESULT
  init(Device &device, const std::string &textureName, ExtensionType extensionType);

  /** @brief Declara o ejecuta initFromFile. */
  HRESULT
  initFromFile(Device &device, const std::string &fullPath);

  /** @brief Declara o ejecuta initFromMemory. */
  HRESULT
  initFromMemory(Device &device,
                 const unsigned char *data,
                 size_t size,
                 const std::string &textureName);

  /** @brief Declara o ejecuta initSingleChannelFromMemory. */
  HRESULT
  initSingleChannelFromMemory(Device &device,
                              const unsigned char *data,
                              size_t size,
                              int channelIndex,
                              const std::string &textureName);

  /** @brief Declara o ejecuta initSolidColor. */
  HRESULT
  initSolidColor(Device &device,
                 unsigned char r,
                 unsigned char g,
                 unsigned char b,
                 unsigned char a,
                 const std::string &textureName);

  /**
   * @brief Inicializa la textura como un recurso vacÃ­o en memoria.
   *
   * @param device Referencia al dispositivo de DirectX.
   * @param width Ancho de la textura.
   * @param height Alto de la textura.
   * @param Format Formato de la textura (DXGI_FORMAT).
   * @param BindFlags Banderas de enlace (ej. render target, shader resource).
   * @param sampleCount NÃºmero de muestras para multisampling (default = 1).
   * @param qualityLevels Niveles de calidad para multisampling (default = 0).
   * @return HRESULT CÃ³digo de resultado (S_OK si se creÃ³ correctamente).
   */
  HRESULT
  init(Device &device,
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
   * @return HRESULT CÃ³digo de resultado.
   */
  HRESULT
  init(Device &device, Texture &textureRef, DXGI_FORMAT format);

  /**
   * @brief Actualiza el estado de la textura.
   *
   * MÃ©todo de marcador para lÃ³gica de actualizaciÃ³n de texturas.
   *
   * @note Actualmente no realiza ninguna operaciÃ³n.
   */
  void update();

  /**
   * @brief Renderiza la textura en el pipeline grÃ¡fico.
   *
   * @param deviceContext Contexto del dispositivo de DirectX.
   * @param StartSlot Slot de inicio donde se asignarÃ¡ la textura.
   * @param NumViews NÃºmero de vistas de recurso de shader a asignar.
   *
   * @pre @c m_textureFromImg debe haberse creado con init().
   */
  void
  render(DeviceContext &deviceContext, unsigned int StartSlot, unsigned int NumViews);

  /**
   * @brief Libera los recursos asociados a la textura.
   *
   * @post @c m_texture == nullptr y
   *       @c m_textureFromImg == nullptr.
   */
  void destroy();

  /**
   * @brief Crea un cubemap utilizando seis imÃ¡genes.
   *
   * @param device Referencia al dispositivo de DirectX.
   * @param deviceContext Contexto del dispositivo.
   * @param facePaths Arreglo con las rutas de las seis caras del cubemap.
   * @param generateMips Indica si se deben generar mipmaps.
   * @return HRESULT CÃ³digo de resultado.
   */
  HRESULT
  CreateCubemap(Device &device,
                DeviceContext &deviceContext,
                const std::array<std::string, 6> &facePaths,
                bool generateMips /*= false*/);

  /**
   * @brief Crea una Shader Resource View para una cara especÃ­fica del cubemap.
   *
   * @param device Dispositivo de DirectX utilizado para crear la vista.
   * @param cubemapTex Textura del cubemap.
   * @param format Formato DXGI de la textura.
   * @param faceIndex Ãndice de la cara del cubemap.
   * @param mipLevels NÃºmero de mip levels.
   * @return ID3D11ShaderResourceView* Vista creada; nullptr en caso de error.
   */
  ID3D11ShaderResourceView *
  CreateCubemapFaceSRV(ID3D11Device *device,
                       ID3D11Texture2D *cubemapTex,
                       DXGI_FORMAT format,
                       UINT faceIndex,
                       UINT mipLevels = 1) {
    D3D11_SHADER_RESOURCE_VIEW_DESC d{};
    d.Format = format;
    d.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
    d.Texture2DArray.MostDetailedMip = 0;
    d.Texture2DArray.MipLevels = mipLevels;
    d.Texture2DArray.FirstArraySlice = faceIndex;
    d.Texture2DArray.ArraySize = 1;

    ID3D11ShaderResourceView *srv = nullptr;

    if (FAILED(device->CreateShaderResourceView(cubemapTex, &d, &srv)))
      return nullptr;

    return srv;
  }

public:
  /**
   * @brief Puntero al recurso de textura 2D en DirectX 11.
   */
  ID3D11Texture2D *m_texture = nullptr;

  /**
   * @brief Vista de recurso de shader creada a partir de la textura.
   */
  ID3D11ShaderResourceView *m_textureFromImg = nullptr;

  /**
   * @brief Nombre o ruta de la textura cargada.
   */
  std::string m_textureName;
};
