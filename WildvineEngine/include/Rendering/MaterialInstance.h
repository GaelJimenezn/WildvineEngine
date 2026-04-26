#pragma once
#include "Prerequisites.h"
#include "Rendering/RenderTypes.h"

class Material;
class DeviceContext;
class Texture;

/**
 * @class MaterialInstance
 * @brief Instancia específica de un material que posee valores de parámetros únicos y mapas de texturas.
 */
class
MaterialInstance {

public:
	/**
	 * @brief Asigna el material base (parent) del cual hereda shaders y estados del pipeline.
	 * @param material Puntero al material base.
	 */
	void 
	setMaterial(Material* material) { m_material = material; }

	/**
	 * @brief Asigna la textura de Albedo (Color base).
	 * @param texture Puntero a la textura de albedo.
	 */
	void 
	setAlbedo(Texture* texture) { m_albedo = texture; }

	/**
	 * @brief Asigna el mapa de normales.
	 * @param texture Puntero a la textura normal.
	 */
	void 
	setNormal(Texture* texture) { m_normal = texture; }

	/**
	 * @brief Asigna el mapa de Metalicidad.
	 * @param texture Puntero a la textura metálica.
	 */
	void 
	setMetallic(Texture* texture) { m_metallic = texture; }

	/**
	 * @brief Asigna el mapa de Rugosidad (Roughness).
	 * @param texture Puntero a la textura de rugosidad.
	 */
	void 
	setRoughness(Texture* texture) { m_roughness = texture; }

	/**
	 * @brief Asigna el mapa de Oclusión Ambiental.
	 * @param texture Puntero a la textura de AO.
	 */
	void 
	setAO(Texture* texture) { m_ao = texture; }

	/**
	 * @brief Asigna el mapa emisivo.
	 * @param texture Puntero a la textura emisiva.
	 */
	void 
	setEmissive(Texture* texture) { m_emissive = texture; }

	/**
	 * @brief Obtiene el material base asociado.
	 * @return Puntero al material base.
	 */
	Material* getMaterial() const { return m_material; }

	/**
	 * @brief Obtiene la textura de albedo actual.
	 * @return Puntero a la textura de albedo.
	 */
	Texture* getAlbedo() const { return m_albedo; }

	/**
	 * @brief Obtiene el mapa de normales actual.
	 * @return Puntero al mapa de normales.
	 */
	Texture* getNormal() const { return m_normal; }

	/**
	 * @brief Obtiene la textura metálica actual.
	 * @return Puntero a la textura metálica.
	 */
	Texture* getMetallic() const { return m_metallic; }

	/**
	 * @brief Obtiene la textura de rugosidad actual.
	 * @return Puntero a la textura de rugosidad.
	 */
	Texture* getRoughness() const { return m_roughness; }

	/**
	 * @brief Obtiene la textura de oclusión ambiental actual.
	 * @return Puntero a la textura AO.
	 */
	Texture* getAO() const { return m_ao; }

	/**
	 * @brief Obtiene la textura emisiva actual.
	 * @return Puntero a la textura emisiva.
	 */
	Texture* getEmissive() const { return m_emissive; }

	/**
	 * @brief Acceso por referencia a los parámetros PBR numéricos (para lectura/escritura).
	 * @return Referencia a MaterialParams.
	 */
	MaterialParams& 
	getParams() { return m_params; }

	/**
	 * @brief Acceso de solo lectura a los parámetros PBR numéricos.
	 * @return Referencia constante a MaterialParams.
	 */
	const MaterialParams& 
	getParams() const { return m_params; }

	/**
	 * @brief Enlaza (bind) todas las texturas válidas de esta instancia al contexto de dispositivo de la GPU.
	 * @param deviceContext Referencia al DeviceContext de DirectX.
	 */
	void 
	bindTextures(DeviceContext& deviceContext) const{}

private:
	Material* m_material = nullptr;      /**< Material base que define los shaders. */
	Texture* m_albedo = nullptr;         /**< Textura de color difuso/albedo. */
	Texture* m_normal = nullptr;         /**< Mapa de normales tangentes. */
	Texture* m_metallic = nullptr;       /**< Textura de nivel metálico. */
	Texture* m_roughness = nullptr;      /**< Textura de nivel de rugosidad. */
	Texture* m_ao = nullptr;             /**< Textura de oclusión ambiental. */
	Texture* m_emissive = nullptr;       /**< Textura de emisión de luz. */
	MaterialParams m_params;             /**< Parámetros físicos (escalares/vectores) de PBR. */
};