/**
 * @file MaterialInstance.h
 * @brief Declara la API de MaterialInstance dentro del subsistema de renderizado.
 */
#pragma once
#include "Prerequisites.h"
#include "Rendering/RenderTypes.h"

/** @brief Declara class Material. */
class Material;
/** @brief Declara class DeviceContext. */
class DeviceContext;
/** @brief Declara class Texture. */
class Texture;

/**
 * @class MaterialInstance
 * @brief Representa una instancia concreta de un material con texturas y parÃ¡metros
 * propios.
 *
 * Permite reutilizar un Material base mientras se modifican:
 * - Texturas (albedo, normal, metallic, etc.)
 * - ParÃ¡metros especÃ­ficos (MaterialParams)
 *
 * Es Ãºtil para renderizar mÃºltiples objetos con el mismo shader pero diferentes
 * propiedades visuales.
 */
class
MaterialInstance {
public:
  /**
   * @brief Asigna el material base.
   * @param material Puntero al Material.
   */
  void
  setMaterial(Material *material) {
    m_material = material;
  }

  /**
   * @brief Asigna la textura de albedo (color base).
   * @param texture Puntero a Texture.
   */
  void
  setAlbedo(Texture *texture) {
    m_albedo = texture;
  }

  /**
   * @brief Asigna la textura de normales.
   * @param texture Puntero a Texture.
   */
  void
  setNormal(Texture *texture) {
    m_normal = texture;
  }

  /**
   * @brief Asigna la textura metÃ¡lica.
   * @param texture Puntero a Texture.
   */
  void
  setMetallic(Texture *texture) {
    m_metallic = texture;
  }

  /**
   * @brief Asigna la textura de rugosidad.
   * @param texture Puntero a Texture.
   */
  void
  setRoughness(Texture *texture) {
    m_roughness = texture;
  }

  /**
   * @brief Asigna la textura de ambient occlusion.
   * @param texture Puntero a Texture.
   */
  void
  setAO(Texture *texture) {
    m_ao = texture;
  }

  /**
   * @brief Asigna la textura emisiva.
   * @param texture Puntero a Texture.
   */
  void
  setEmissive(Texture *texture) {
    m_emissive = texture;
  }

  /**
   * @brief Obtiene el material base.
   * @return Puntero a Material.
   */
  Material *
  getMaterial() const {
    return m_material;
  }

  /**
   * @brief Obtiene la textura de albedo.
   * @return Puntero a Texture.
   */
  Texture *
  getAlbedo() const {
    return m_albedo;
  }

  /**
   * @brief Obtiene la textura de normales.
   * @return Puntero a Texture.
   */
  Texture *
  getNormal() const {
    return m_normal;
  }

  /**
   * @brief Obtiene la textura metÃ¡lica.
   * @return Puntero a Texture.
   */
  Texture *
  getMetallic() const {
    return m_metallic;
  }

  /**
   * @brief Obtiene la textura de rugosidad.
   * @return Puntero a Texture.
   */
  Texture *
  getRoughness() const {
    return m_roughness;
  }

  /**
   * @brief Obtiene la textura de ambient occlusion.
   * @return Puntero a Texture.
   */
  Texture *
  getAO() const {
    return m_ao;
  }

  /**
   * @brief Obtiene la textura emisiva.
   * @return Puntero a Texture.
   */
  Texture *
  getEmissive() const {
    return m_emissive;
  }

  /**
   * @brief Obtiene los parÃ¡metros modificables del material.
   * @return Referencia a MaterialParams.
   */
  MaterialParams &
  getParams() {
    return m_params;
  }

  /**
   * @brief Obtiene los parÃ¡metros del material (const).
   * @return Referencia constante a MaterialParams.
   */
  const
      /** @brief Declara o ejecuta getParams. */
      MaterialParams &
      getParams() const {
    return m_params;
  }

  /**
   * @brief Vincula las texturas al pipeline grÃ¡fico.
   * @param deviceContext Contexto del dispositivo.
   */
  void bindTextures(DeviceContext &deviceContext) const;

private:
  /** @brief Material base asociado. */
  Material *m_material = nullptr;

  /** @brief Textura de albedo. */
  Texture *m_albedo = nullptr;

  /** @brief Textura de normales. */
  Texture *m_normal = nullptr;

  /** @brief Textura metÃ¡lica. */
  Texture *m_metallic = nullptr;

  /** @brief Textura de rugosidad. */
  Texture *m_roughness = nullptr;

  /** @brief Textura de ambient occlusion. */
  Texture *m_ao = nullptr;

  /** @brief Textura emisiva. */
  Texture *m_emissive = nullptr;

  /** @brief ParÃ¡metros del material. */
  MaterialParams m_params;
};
