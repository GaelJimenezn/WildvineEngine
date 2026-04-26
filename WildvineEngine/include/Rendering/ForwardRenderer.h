#pragma once
#include "Prerequisites.h"
#include "Buffer.h"
#include "DepthStencilState.h"
#include "Rendering/RenderScene.h"
#include "Rendering/RenderTypes.h"

class Device;
class DeviceContext;
class Camera;
class EditorViewportPass;
class Material;

/**
 * @class ForwardRenderer
 * @brief Sistema de renderizado principal del motor usando un enfoque Forward Rendering.
 */
class 
ForwardRenderer {

public:

	/**
	 * @brief Inicializa los buffers globales, estados y configuraciones iniciales del renderer.
	 * @param device Referencia al dispositivo principal de la GPU (Direct3D Device).
	 * @return HRESULT indicando éxito (S_OK) o el código de error correspondiente.
	 */
    HRESULT 
    init(Device& device);

	/**
	 * @brief Actualiza los tamaños de las texturas internas o pases si cambia la resolución de pantalla.
	 * @param device Referencia al dispositivo de la GPU.
	 * @param width Nueva anchura en píxeles.
	 * @param height Nueva altura en píxeles.
	 */
    void 
    resize(Device& device, unsigned int width, unsigned int height){}

	/**
	 * @brief Actualiza los Constant Buffers globales (PerFrame) y prepara los datos para el renderizado.
	 * @param camera Cámara desde la cual se está viendo la escena (aporta Vista y Proyección).
	 * @param scene Escena recolectada que contiene información de la iluminación y fondo.
	 * @param deviceContext Contexto del dispositivo usado para actualizar buffers en memoria GPU.
	 */
    void 
    updatePerFrame(const Camera& camera, const RenderScene& scene, DeviceContext& deviceContext);

	/**
	 * @brief Ejecuta de principio a fin el pipeline visual, dibujando toda la escena y enviándola al viewport.
	 * @param deviceContext Contexto del dispositivo para emitir comandos de dibujado.
	 * @param camera La cámara activa.
	 * @param scene La escena con los objetos clasificados a dibujar.
	 * @param viewportPass Manejador del pase final del editor/pantalla.
	 */
    void 
    render(DeviceContext& deviceContext,
        const Camera& camera,
        RenderScene& scene,
        EditorViewportPass& viewportPass);

	/**
	 * @brief Destruye e invalida todos los recursos en GPU alojados por el renderer (Buffers y Estados de mezcla).
	 */
    void 
    destroy(){}

private:
	/**
	 * @brief Clasifica y ordena los objetos de la RenderScene en colas locales (opacos y transparentes ordenados por distancia).
	 * @param scene La escena a analizar.
	 * @param camera Cámara utilizada para calcular las distancias en los objetos transparentes.
	 */
    void 
    buildQueues(RenderScene& scene, const Camera& camera);

	/**
	 * @brief Recorre y dibuja secuencialmente la cola de objetos opacos (sin blending).
	 * @param deviceContext Contexto para el envío a GPU.
	 */
    void
    renderOpaquePass(DeviceContext& deviceContext);

	/**
	 * @brief Recorre y dibuja secuencialmente la cola de objetos transparentes (aplicando back-to-front y blending).
	 * @param deviceContext Contexto para el envío a GPU.
	 */
    void 
    renderTransparentPass(DeviceContext& deviceContext);

	/**
	 * @brief Dibuja el skybox de fondo garantizando que solo pinte en áreas vacías (depth = 1.0).
	 * @param deviceContext Contexto para el envío a GPU.
	 * @param scene Escena actual que contiene el puntero al Skybox.
	 */
    void 
    renderSkyboxPass(DeviceContext& deviceContext, RenderScene& scene);

	/**
	 * @brief Vincula recursos y emite la llamada DrawIndexed para un objeto específico.
	 * @param deviceContext Contexto de dibujado de la GPU.
	 * @param object Objeto a renderizar (malla y materiales).
	 * @param passType El pase que está invocando el dibujado (determina qué shaders y estados forzar).
	 */
    void 
    renderObject(DeviceContext& deviceContext, 
								 const RenderObject& object, RenderPassType passType);

	/**
	 * @brief Crea e inicializa en la GPU todos los perfiles de estados de mezcla (Blend States) declarados.
	 * @param device Dispositivo creador de DirectX.
	 * @return HRESULT indicando éxito o fracaso en la creación de los estados.
	 */
    HRESULT 
    createBlendStates(Device& device);

	/**
	 * @brief Traduce la propiedad BlendMode de un Material al objeto BlendState correspondiente en DirectX.
	 * @param material Puntero constante al material a consultar.
	 * @return Puntero al ID3D11BlendState pre-configurado para el modo que solicita el material.
	 */
		ID3D11BlendState* resolveBlendState(const Material* material) 
			const { return nullptr;}

private:
    Buffer m_perFrameBuffer;                              /**< Buffer constante para datos compartidos globalmente en el frame (Matrices y Luz). */
    Buffer m_perObjectBuffer;                             /**< Buffer constante para transformaciones locales del modelo. */
    Buffer m_perMaterialBuffer;                           /**< Buffer constante para las variables PBR del shader. */
    DepthStencilState m_transparentDepthStencil;          /**< Estado especial del Z-Buffer usado durante el pase transparente (lectura sin escritura). */
    ID3D11BlendState* m_alphaBlendState = nullptr;        /**< Estado de mezcla para transparencias por canal alfa. */
    ID3D11BlendState* m_opaqueBlendState = nullptr;       /**< Estado base sin mezcla de color. */
    ID3D11BlendState* m_additiveBlendState = nullptr;     /**< Estado de mezcla aditiva. */
    ID3D11BlendState* m_premultipliedBlendState = nullptr;/**< Estado de mezcla para color que tiene el alfa integrado. */
    float m_blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };  /**< Vector factor de la operación de mezcla (si aplica). */

    CBPerFrame m_cbPerFrame{};                            /**< Memoria en CPU de los datos del frame. */
    CBPerObject m_cbPerObject{};                          /**< Memoria en CPU de los datos de mundo del objeto. */
    CBPerMaterial m_cbPerMaterial{};                      /**< Memoria en CPU de los datos de material. */

    std::vector<const RenderObject*> m_opaqueQueue;       /**< Cola interna ordenada de dibujado de objetos opacos. */
    std::vector<const RenderObject*> m_transparentQueue;  /**< Cola interna ordenada de dibujado de objetos transparentes. */
};