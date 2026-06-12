/**
 * @file MaterialInstance.h
 * @brief Declara la API de MaterialInstance dentro del subsistema Rendering.
 * @ingroup rendering
 */
#pragma once
#include "Prerequisites.h"
#include "Rendering/RenderTypes.h"

class Material;
class DeviceContext;
class Texture;

/**
 * @class MaterialInstance
 * @brief Agrupa un material base con sus texturas y parametros concretos.
 *
 * Esta clase permite reutilizar un mismo 'Material' con diferentes mapas de texturas
 * y parametros PBR por objeto renderizado.
 */
class
	MaterialInstance {
public:
    /** @brief Sets the shared material definition used by this instance. */
    void
    	setMaterial(Material* material) { m_material = material; }

    /** @brief Assigns the albedo/base-color texture. */
    void
    	setAlbedo(Texture* texture) { m_albedo = texture; }

    /** @brief Assigns the normal-map texture. */
    void
    	setNormal(Texture* texture) { m_normal = texture; }

    /** @brief Assigns the metallic texture. */
    void
    	setMetallic(Texture* texture) { m_metallic = texture; }

    /** @brief Assigns the roughness texture. */
    void
    	setRoughness(Texture* texture) { m_roughness = texture; }

    /** @brief Assigns the ambient-occlusion texture. */
    void
    	setAO(Texture* texture) { m_ao = texture; }

    /** @brief Assigns the emissive texture. */
    void
    	setEmissive(Texture* texture) { m_emissive = texture; }

    /** @brief Returns the shared material definition. */
    Material*
    	getMaterial() const { return m_material; }

    /** @brief Returns the albedo/base-color texture. */
    Texture*
    	getAlbedo() const { return m_albedo; }

    /** @brief Returns the normal-map texture. */
    Texture*
    	getNormal() const { return m_normal; }

    /** @brief Returns the metallic texture. */
    Texture*
    	getMetallic() const { return m_metallic; }

    /** @brief Returns the roughness texture. */
    Texture*
    	getRoughness() const { return m_roughness; }

    /** @brief Returns the ambient-occlusion texture. */
    Texture*
    	getAO() const { return m_ao; }

    /** @brief Returns the emissive texture. */
    Texture*
    	getEmissive() const { return m_emissive; }

    /** @brief Returns mutable scalar/vector material parameters. */
    MaterialParams&
    	getParams() { return m_params; }

    /** @brief Returns read-only scalar/vector material parameters. */
    const MaterialParams&
    	getParams() const { return m_params; }

    /**
     * @brief Enlaza las texturas de la instancia en el contexto grafico actual.
     */
    void
    	bindTextures(DeviceContext& deviceContext) const;

private:
    /** @brief Non-owning shared material definition. */
    Material* m_material = nullptr;
    /** @brief Non-owning albedo/base-color texture. */
    Texture* m_albedo = nullptr;
    /** @brief Non-owning normal-map texture. */
    Texture* m_normal = nullptr;
    /** @brief Non-owning metallic texture. */
    Texture* m_metallic = nullptr;
    /** @brief Non-owning roughness texture. */
    Texture* m_roughness = nullptr;
    /** @brief Non-owning ambient-occlusion texture. */
    Texture* m_ao = nullptr;
    /** @brief Non-owning emissive texture. */
    Texture* m_emissive = nullptr;
    /** @brief CPU-side PBR/scalar parameters mirrored into material constant buffers. */
    MaterialParams m_params;
};
