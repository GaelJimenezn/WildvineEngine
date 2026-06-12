#pragma once
#include "Prerequisites.h"
#include "MeshComponent.h"

class Device;
class DeviceContext;

/**
 * @class Buffer
 * @brief Encapsula un @c ID3D11Buffer para vértices, índices o constantes, incluyendo
 * creación, actualización y enlace.
 *
 * Esta clase administra la vida de un único buffer de D3D11 y su uso en etapa de render.
 * Soporta:
 * - Creación como Vertex/Index buffer a partir de un @c MeshComponent.
 * - Creación como Constant buffer a partir de un tamaño (ByteWidth).
 * - Actualización de datos (p. ej. @c UpdateSubresource).
 * - Enlace del buffer a la etapa correspondiente del pipeline.
 *
 * @note La instancia gestiona un solo @c ID3D11Buffer a la vez; el tipo efectivo se
 * deduce de @c m_bindFlag.
 * @warning No copia el recurso; si se añade semántica de copia, manejar correctamente
 * referencias COM.
 */
class
	Buffer {
public:
    /**
     * @brief Constructor por defecto (no crea recursos).
     */
    Buffer() = default;

    /**
     * @brief Destructor por defecto.
     * @details No libera automáticamente; llamar a destroy() para liberar el recurso COM.
     */
    ~Buffer() = default;

    /**
     * @brief Inicializa el buffer como Vertex o Index Buffer usando un @c MeshComponent.
     *
     * Crea internamente un @c ID3D11Buffer con los datos del mesh (vértices/índices)
     * según @p bindFlag.
     * Debe usarse @c D3D11_BIND_VERTEX_BUFFER o @c D3D11_BIND_INDEX_BUFFER.
     *
     * @param device     Dispositivo con el que se creará el recurso.
     * @param mesh       Fuente de datos (vértices/índices) para poblar el buffer.
     * @param bindFlag   Bandera de enlace (p. ej. @c D3D11_BIND_VERTEX_BUFFER o @c
     * D3D11_BIND_INDEX_BUFFER).
     * @return @c S_OK si la creación fue exitosa; código @c HRESULT en caso contrario.
     *
     * @post Si retorna @c S_OK, @c m_buffer != nullptr y @c m_bindFlag == bindFlag.
     * @sa createBuffer(), render()
     */
    HRESULT
    	init(Device& device, const MeshComponent& mesh, unsigned int bindFlag);

    /**
     * @brief Inicializa el buffer como Constant Buffer.
     *
     * Crea un @c ID3D11Buffer con @c D3D11_BIND_CONSTANT_BUFFER y tamaño @p ByteWidth
     * (múltiplo de 16 bytes recomendado).
     *
     * @param device     Dispositivo con el que se creará el recurso.
     * @param ByteWidth  Tamaño del buffer en bytes (alinear a 16 para constantes).
     * @return @c S_OK si la creación fue exitosa; código @c HRESULT en caso contrario.
     *
     * @post Si retorna @c S_OK, @c m_buffer != nullptr y @c m_bindFlag ==
     * D3D11_BIND_CONSTANT_BUFFER.
     * @sa update(), render()
     */
    HRESULT
    	init(Device& device, unsigned int ByteWidth);

    /**
     * @brief Actualiza el contenido del buffer (típicamente mediante @c
     * UpdateSubresource).
     *
     * Útil para escribir datos de constantes por cuadro, o para subir datos de
     * vértices/índices si corresponde.
     *
     * @param deviceContext  Contexto donde se realizará la actualización.
     * @param pDstResource   Recurso destino (típicamente @c m_buffer). Se permite pasar
     * otro recurso compatible.
     * @param DstSubresource Índice de subrecurso destino (normalmente 0 para buffers).
     * @param pDstBox        Región destino (puede ser @c nullptr para sobrescribir
     * completo).
     * @param pSrcData       Puntero a los datos de origen.
     * @param SrcRowPitch    Paso por fila (no aplica a buffers; se ignora por D3D11 para
     * buffers).
     * @param SrcDepthPitch  Paso por profundidad (no aplica a buffers; se ignora por
     * D3D11 para buffers).
     *
     * @pre @c pDstResource debe ser un buffer válido creado en este dispositivo.
     * @note Para constantes dinámicas alternativamente podría usarse @c Map/@c Unmap con
     * @c D3D11_USAGE_DYNAMIC.
     */
    void
    	update(DeviceContext& deviceContext,
            ID3D11Resource* pDstResource,
            unsigned int    DstSubresource,
            const D3D11_BOX* pDstBox,
            const void* pSrcData,
            unsigned int    SrcRowPitch,
            unsigned int    SrcDepthPitch);

    /**
     * @brief Enlaza el buffer a la etapa correspondiente del pipeline para el frame de
     * render.
     *
     * El comportamiento depende de @c m_bindFlag:
     * - @c D3D11_BIND_VERTEX_BUFFER: Llama a @c IASetVertexBuffers con @p StartSlot y @p
     * NumBuffers (stride/offset internos).
     * - @c D3D11_BIND_INDEX_BUFFER:  Llama a @c IASetIndexBuffer con @p format.
     * - @c D3D11_BIND_CONSTANT_BUFFER: Enlaza a VS/PS según @p setPixelShader, usando @p
     * StartSlot y @p NumBuffers.
     *
     * @param deviceContext   Contexto donde se enlazará el buffer.
     * @param StartSlot       Primer slot de enlace (IA o VS/PS según tipo).
     * @param NumBuffers      Número de buffers a enlazar (típicamente 1 para esta clase).
     * @param setPixelShader  Si es @c true y el buffer es de constantes, también se
     * enlaza a PS (además de VS).
     * @param format          Formato del índice (@c DXGI_FORMAT_R16_UINT o @c
     * DXGI_FORMAT_R32_UINT) cuando es Index Buffer.
     *
     * @pre @c m_buffer debe estar creado y @c m_bindFlag configurado correctamente.
     * @sa init()
     */
    void
    	render(DeviceContext& deviceContext,
            unsigned int   StartSlot,
            unsigned int   NumBuffers,
            bool           setPixelShader = false,
            DXGI_FORMAT    format = DXGI_FORMAT_UNKNOWN);

    /**
     * @brief Libera el @c ID3D11Buffer y resetea los metadatos internos.
     *
     * Idempotente.
     *
     * @post @c m_buffer == nullptr, @c m_stride == 0, @c m_offset == 0 y @c m_bindFlag ==
     * 0.
     */
    void
    	destroy();

    /**
     * @brief Crea un buffer genérico con una @c D3D11_BUFFER_DESC y datos iniciales
     * opcionales.
     *
     * Método de ayuda para factorizar la creación. Normalmente invocado por @c init().
     *
     * @param device   Dispositivo con el que se creará el recurso.
     * @param desc     Descriptor del buffer (uso, bind flags, tamaño, etc.).
     * @param initData Datos iniciales (puede ser @c nullptr para buffer sin inicializar).
     * @return @c S_OK si la creación fue exitosa; código @c HRESULT en caso contrario.
     */
    HRESULT
    	createBuffer(Device& device,
            D3D11_BUFFER_DESC& desc,
            D3D11_SUBRESOURCE_DATA* initData);

public:
    ID3D11Buffer* m_buffer = nullptr;
private:
    /**
     * @brief Recurso COM de D3D11 administrado por la clase.
     */

     /**
      * @brief Tamaño de un elemento en bytes (para Vertex Buffer).
      * @details Usado en @c IASetVertexBuffers. Cero cuando no aplica.
      */
    unsigned int m_stride = 0;

    /**
     * @brief Desplazamiento inicial en bytes (para Vertex Buffer).
     * @details Usado en @c IASetVertexBuffers. Cero cuando no aplica.
     */
    unsigned int m_offset = 0;

    /**
     * @brief Bandera de enlace (@c D3D11_BIND_* ) que define el rol del buffer.
     */
    unsigned int m_bindFlag = 0;
};
