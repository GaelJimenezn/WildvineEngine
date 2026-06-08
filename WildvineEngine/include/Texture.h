#pragma once
#include "Prerequisites.h"

class Device;
class DeviceContext;

/**
 * @class Texture
 * @brief Encapsula una textura 2D en Direct3D 11, incluyendo su recurso y vista como Shader Resource.
 *
 * Esta clase administra texturas que pueden provenir de:
 * - Archivos de imagen (png, jpg, etc.).
 * - Texturas creadas en memoria (RTV, DSV, UAV).
 * - Copias a partir de otra textura.
 *
 * Proporciona métodos para inicialización, actualización, uso en shaders y destrucción.
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
     * @brief Inicializa una textura cargada desde archivo.
     *
     * Crea un recurso de textura a partir de un archivo de imagen y genera su
     * @c ShaderResourceView correspondiente para ser usado en shaders.
     *
     * @param device        Dispositivo con el que se creará la textura.
     * @param textureName   Nombre o ruta del archivo de textura.
     * @param extensionType Tipo de extensión de archivo (ej. PNG, JPG, DDS).
     * @return @c S_OK si fue exitoso; código @c HRESULT en caso contrario.
     *
     * @post Si retorna @c S_OK, @c m_texture y @c m_textureFromImg != nullptr.
     */
    HRESULT
        init(Device& device,
            const std::string& textureName,
            ExtensionType extensionType);

    /**
     * @brief Inicializa una textura creada desde memoria.
     *
     * Crea un recurso de textura 2D vacío con un tamaño y formato especificados.
     * Útil para render targets, depth buffers o texturas dinámicas.
     *
     * @param device        Dispositivo con el que se creará la textura.
     * @param width         Ancho de la textura en píxeles.
     * @param height        Alto de la textura en píxeles.
     * @param Format        Formato DXGI de la textura (ej. DXGI_FORMAT_R8G8B8A8_UNORM).
     * @param BindFlags     Banderas de enlace (ej. @c D3D11_BIND_SHADER_RESOURCE, @c D3D11_BIND_RENDER_TARGET).
     * @param sampleCount   Número de muestras para MSAA (por defecto 1 = sin MSAA).
     * @param qualityLevels Niveles de calidad soportados para MSAA.
     * @return @c S_OK si fue exitoso; código @c HRESULT en caso contrario.
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
     * @brief Inicializa una textura a partir de otra existente.
     *
     * Crea una nueva textura basada en la descripción de @p textureRef,
     * con un formato diferente.
     *
     * @param device     Dispositivo con el que se creará la textura.
     * @param textureRef Referencia a otra textura existente.
     * @param format     Nuevo formato DXGI de la textura.
     * @return @c S_OK si fue exitoso; código @c HRESULT en caso contrario.
     */
    HRESULT
        init(Device& device, Texture& textureRef, DXGI_FORMAT format);

    /**
     * @brief Actualiza el contenido de la textura.
     *
     * Método de marcador, útil para soportar carga dinámica de datos o streaming
     * de texturas desde CPU hacia GPU.
     *
     * @note Actualmente no realiza ninguna operación.
     */
    void
        update();

    /**
     * @brief Asigna la textura al pipeline de render.
     *
     * Llama a @c PSSetShaderResources para establecer la textura como
     * recurso de shader en la etapa de Pixel Shader.
     *
     * @param deviceContext Contexto donde se aplicará la textura.
     * @param StartSlot     Slot inicial en el que se vinculará la textura.
     * @param NumViews      Número de vistas de recurso a asignar (normalmente 1).
     *
     * @pre @c m_textureFromImg debe haberse creado con init().
     */
    void
        render(DeviceContext& deviceContext, unsigned int StartSlot, unsigned int NumViews);

    /**
     * @brief Libera los recursos de la textura.
     *
     * Libera tanto el recurso de textura (@c ID3D11Texture2D) como su
     * @c ShaderResourceView asociado.
     *
     * @post @c m_texture == nullptr y @c m_textureFromImg == nullptr.
     */
    void
        destroy();

    /**
     * @brief Creates a cubemap texture from six image file paths.
     * @param device Device used to create texture and SRV resources.
     * @param deviceContext Context used for upload/copy operations.
     * @param facePaths Ordered image paths for the six cubemap faces.
     * @param generateMips Whether mipmaps should be generated when supported.
     * @return @c S_OK on success; failing @c HRESULT otherwise.
     */
    HRESULT
        CreateCubemap(Device& device,
            DeviceContext& deviceContext,
            const std::array<std::string, 6>& facePaths,
            bool generateMips /*= false*/);

    /**
     * @brief Creates an SRV that views one face of a cubemap as a 2D texture array slice.
     * @param device Native D3D11 device.
     * @param cubemapTex Cubemap texture resource.
     * @param format View format.
     * @param faceIndex Cubemap face index to expose.
     * @param mipLevels Number of mip levels visible through the SRV.
     * @return Created SRV, or @c nullptr on failure.
     */
    ID3D11ShaderResourceView* CreateCubemapFaceSRV(
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
        d.Texture2DArray.MipLevels = mipLevels;       // usa 1 para vista simple
        d.Texture2DArray.FirstArraySlice = faceIndex; // cara
        d.Texture2DArray.ArraySize = 1;               // solo esa cara

        ID3D11ShaderResourceView* srv = nullptr;
        if (FAILED(device->CreateShaderResourceView(cubemapTex, &d, &srv)))
            return nullptr;

        return srv;
    }

public:
    /**
     * @brief Recurso base de la textura en GPU.
     */
    ID3D11Texture2D* m_texture = nullptr;

    /**
     * @brief Vista de la textura como recurso de shader.
     *
     * Permite acceder a la textura desde Pixel Shader u otros shaders.
     */
    ID3D11ShaderResourceView* m_textureFromImg = nullptr;

    /**
     * @brief Nombre o ruta de la textura (si proviene de archivo).
     */
    std::string m_textureName;
};
