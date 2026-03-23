#pragma once
#include "Prerequisites.h"
#include "ShaderProgram.h"
#include "Texture.h"
#include "Buffer.h"
#include "SamplerState.h"
#include "Model3D.h"
#include "RasterizerState.h"
#include "DepthStencilState.h"
#include "EngineUtilities\Utilities\Camera.h"
#include "ECS\Actor.h"

class Device;
class DeviceContext;

/**
 * @class Skybox
 * @brief Gestiona y dibuja una caja de entorno panorámica (cubemap) que envuelve la escena.
 * * Configura los shaders específicos, carga el modelo 3D de un cubo y manipula estados 
 * del renderizador (como ignorar la escritura de profundidad) para simular un fondo infinito.
 */
class 
Skybox {
public:
	Skybox()  = default;
	~Skybox() = default;

	/**
	 * @brief Inicializa los recursos necesarios para pintar el skybox.
	 * * @param device Dispositivo de hardware para crear recursos de la GPU.
	 * @param deviceContext Contexto del dispositivo para interactuar con la pipeline.
	 * @param cubemap Referencia a la textura del tipo Cubemap pre-cargada.
	 * @return HRESULT Retorna S_OK en caso de éxito o códigos de error nativos de DirectX11.
	 */
	HRESULT 
	init(Device& device, DeviceContext* deviceContext, Texture& cubemap);
	
	/**
	 * @brief Actualiza la posición del Skybox para centrarse siempre en la cámara.
	 * * Se encarga de mapear y actualizar los buffers constantes que se envían al shader
	 * con las matrices View y Projection actuales. El Skybox no debe sufrir translación, 
	 * solo seguir la rotación de la cámara.
	 * * @param deviceContext Contexto para mapear el constant buffer.
	 * @param camera Cámara de la que se extraerán las matrices de proyección y vista.
	 */
	void 
	update(DeviceContext& deviceContext, Camera& camera);

	/**
	 * @brief Vincula los recursos a la pipeline gráfica y ejecuta el comando de dibujado (Draw).
	 * * Configura el Rasterizer para evitar el "culling" de la caras traseras, ajusta el buffer
	 * de profundidad para renderizarse con "Less/Equal", y dibuja la malla cúbica interna.
	 * * @param deviceContext Contexto del dispositivo para enviar los comandos de render.
	 */
	void
	render(DeviceContext& deviceContext);

	/**
	 * @brief Limpia y libera los recursos. (A implementar o actualmente manejado por destrucción inteligente de clases internas).
	 */
	void
	destroy() {}

private:
	ShaderProgram m_shaderProgram;                /**< Encapsula el Vertex y Pixel Shader específicos para renderizar el cubemap. */
	Buffer m_constantBuffer;                      /**< Buffer de GPU donde se suben las matrices World, View y Projection al shader. */
	SamplerState m_samplerState;                  /**< Estado de muestreo que define cómo se filtran los píxeles de la textura cúbica. */
	RasterizerState m_rasterizerState;            /**< Estado de rasterización para desactivar culling, ya que la cámara está dentro del cubo. */
	DepthStencilState m_depthStencilState;        /**< Estado que permite dibujar el skybox en el fondo lejano asegurando que pasa el Test de Profundidad en Z=1.0. */
	Texture m_skyboxTexture;                      /**< Recurso de la textura cúbica enviada al Pixel Shader. */
	Model3D* m_cubeModel = nullptr;               /**< Puntero al modelo geométrico del cubo sobre el cual se proyecta el entorno. */
	EU::TSharedPointer<Actor> m_skybox;           /**< Entidad ECS virtual del actor de la caja de cielo. */

};