#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Texture.h"
#include "Device.h"
#include "DeviceContext.h"
#include <cstdint>
#include <fstream>

namespace {
constexpr uint32_t kTextureCacheMagic = 0x58545657; // WVTX
constexpr uint32_t kTextureCacheVersion = 1;

struct CachedTextureData {
  int width = 0;
  int height = 0;
  std::vector<unsigned char> rgba;
};

bool GetFileWriteTime(const std::string& path, ULONGLONG& outWriteTime) {
  WIN32_FILE_ATTRIBUTE_DATA attributes{};
  if (!GetFileAttributesExA(path.c_str(), GetFileExInfoStandard, &attributes)) {
    return false;
  }

  ULARGE_INTEGER fileTime{};
  fileTime.LowPart = attributes.ftLastWriteTime.dwLowDateTime;
  fileTime.HighPart = attributes.ftLastWriteTime.dwHighDateTime;
  outWriteTime = fileTime.QuadPart;
  return true;
}

std::string GetTextureCachePath(const std::string& sourcePath) {
  return sourcePath + ".wvtx";
}

bool IsTextureCacheUpToDate(const std::string& sourcePath, const std::string& cachePath) {
  ULONGLONG sourceWriteTime = 0;
  ULONGLONG cacheWriteTime = 0;
  if (!GetFileWriteTime(sourcePath, sourceWriteTime)) {
    return false;
  }
  if (!GetFileWriteTime(cachePath, cacheWriteTime)) {
    return false;
  }
  return cacheWriteTime >= sourceWriteTime;
}

bool SaveTextureCache(const std::string& cachePath, int width, int height, const unsigned char* data) {
  std::ofstream stream(cachePath, std::ios::binary | std::ios::trunc);
  if (!stream.is_open()) {
    return false;
  }

  const uint32_t dataSize = static_cast<uint32_t>(width * height * 4);
  stream.write(reinterpret_cast<const char*>(&kTextureCacheMagic), sizeof(kTextureCacheMagic));
  stream.write(reinterpret_cast<const char*>(&kTextureCacheVersion), sizeof(kTextureCacheVersion));
  stream.write(reinterpret_cast<const char*>(&width), sizeof(width));
  stream.write(reinterpret_cast<const char*>(&height), sizeof(height));
  stream.write(reinterpret_cast<const char*>(&dataSize), sizeof(dataSize));
  stream.write(reinterpret_cast<const char*>(data), dataSize);
  return stream.good();
}

bool LoadTextureCache(const std::string& cachePath, CachedTextureData& outTexture) {
  std::ifstream stream(cachePath, std::ios::binary);
  if (!stream.is_open()) {
    return false;
  }

  uint32_t magic = 0;
  uint32_t version = 0;
  uint32_t dataSize = 0;
  stream.read(reinterpret_cast<char*>(&magic), sizeof(magic));
  stream.read(reinterpret_cast<char*>(&version), sizeof(version));
  stream.read(reinterpret_cast<char*>(&outTexture.width), sizeof(outTexture.width));
  stream.read(reinterpret_cast<char*>(&outTexture.height), sizeof(outTexture.height));
  stream.read(reinterpret_cast<char*>(&dataSize), sizeof(dataSize));

  if (!stream.good() ||
      magic != kTextureCacheMagic ||
      version != kTextureCacheVersion ||
      outTexture.width <= 0 ||
      outTexture.height <= 0 ||
      dataSize != static_cast<uint32_t>(outTexture.width * outTexture.height * 4)) {
    return false;
  }

  outTexture.rgba.resize(dataSize);
  stream.read(reinterpret_cast<char*>(outTexture.rgba.data()), dataSize);
  return stream.good();
}

HRESULT CreateTextureFromRGBA(Device& device,
                              int width,
                              int height,
                              const unsigned char* data,
                              ID3D11Texture2D** outTexture,
                              ID3D11ShaderResourceView** outSRV) {
  D3D11_TEXTURE2D_DESC textureDesc = {};
  textureDesc.Width = width;
  textureDesc.Height = height;
  textureDesc.MipLevels = 1;
  textureDesc.ArraySize = 1;
  textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  textureDesc.SampleDesc.Count = 1;
  textureDesc.Usage = D3D11_USAGE_DEFAULT;
  textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

  D3D11_SUBRESOURCE_DATA initData = {};
  initData.pSysMem = data;
  initData.SysMemPitch = width * 4;

  HRESULT hr = device.CreateTexture2D(&textureDesc, &initData, outTexture);
  if (FAILED(hr)) {
    return hr;
  }

  D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
  srvDesc.Format = textureDesc.Format;
  srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  srvDesc.Texture2D.MipLevels = 1;

  hr = device.m_device->CreateShaderResourceView(*outTexture, &srvDesc, outSRV);
  if (FAILED(hr)) {
    if (*outTexture != nullptr) {
      (*outTexture)->Release();
      *outTexture = nullptr;
    }
  }
  return hr;
}

HRESULT InitTextureFromImage(Device& device, const std::string& fullPath, Texture& texture) {
  CachedTextureData cachedTexture;
  const std::string cachePath = GetTextureCachePath(fullPath);

  int width = 0;
  int height = 0;
  int channels = 0;
  unsigned char* decodedData = nullptr;
  const unsigned char* uploadData = nullptr;

  if (IsTextureCacheUpToDate(fullPath, cachePath) && LoadTextureCache(cachePath, cachedTexture)) {
    width = cachedTexture.width;
    height = cachedTexture.height;
    uploadData = cachedTexture.rgba.data();
  }
  else {
    decodedData = stbi_load(fullPath.c_str(), &width, &height, &channels, 4);
    if (!decodedData) {
      ERROR("Texture", "init",
        ("Failed to load texture: " + std::string(stbi_failure_reason())).c_str());
      return E_FAIL;
    }
    uploadData = decodedData;
    SaveTextureCache(cachePath, width, height, decodedData);
  }

  HRESULT hr = CreateTextureFromRGBA(device, width, height, uploadData, &texture.m_texture, &texture.m_textureFromImg);
  if (decodedData) {
    stbi_image_free(decodedData);
  }

  if (FAILED(hr)) {
    SAFE_RELEASE(texture.m_texture);
    SAFE_RELEASE(texture.m_textureFromImg);
    ERROR("Texture", "init", "Failed to create shader resource view for cached image texture");
    return hr;
  }

  SAFE_RELEASE(texture.m_texture);
  return S_OK;
}
}

HRESULT 
Texture::init(Device& device, 
              const std::string& textureName, 
              ExtensionType extensionType) {
	if (!device.m_device) {
		ERROR("Texture", "init", "Device is null.");
		return E_POINTER;
	}
	if (textureName.empty()) {
		ERROR("Texture", "init", "Texture name cannot be empty.");
		return E_INVALIDARG;
	}

	HRESULT hr = S_OK;

	switch (extensionType) {
	case DDS: {
		m_textureName = textureName + ".dds";

		hr = D3DX11CreateShaderResourceViewFromFile(
			device.m_device,
			m_textureName.c_str(),
			nullptr,
			nullptr,
			&m_textureFromImg,
			nullptr
		);

		if (FAILED(hr)) {
			ERROR("Texture", "init",
				("Failed to load DDS texture. Verify filepath: " + m_textureName).c_str());
			return hr;
		}
		break;
	}

	case PNG: {
    m_textureName = textureName + ".png";
    hr = InitTextureFromImage(device, m_textureName, *this);
		break;
	}
	case JPG: {
    m_textureName = textureName + ".jpg";
    hr = InitTextureFromImage(device, m_textureName, *this);
		break;
	}
	default:
		ERROR("Texture", "init", "Unsupported extension type");
		return E_INVALIDARG;
	}

	return hr;
}

HRESULT 
Texture::init(Device& device, 
              unsigned int width, 
              unsigned int height, 
              DXGI_FORMAT Format, 
              unsigned int BindFlags, 
              unsigned int sampleCount, 
              unsigned int qualityLevels) {
  if (!device.m_device) {
    ERROR("Texture", "init", "Device is null.");
    return E_POINTER;
  }
  if (width == 0 || height == 0) {
    ERROR("Texture", "init", "Width and height must be greater than 0");
    return E_INVALIDARG;
  }

  D3D11_TEXTURE2D_DESC desc;
  memset(&desc, 0, sizeof(desc));
  desc.Width = width;
  desc.Height = height;
  desc.MipLevels = 1;
  desc.ArraySize = 1;
  desc.Format = Format;
  desc.SampleDesc.Count = sampleCount;
  desc.SampleDesc.Quality = qualityLevels;
  desc.Usage = D3D11_USAGE_DEFAULT;
  desc.BindFlags = BindFlags;
  desc.CPUAccessFlags = 0;
  desc.MiscFlags = 0;

  HRESULT hr = device.CreateTexture2D(&desc, nullptr, &m_texture);

  if (FAILED(hr)) {
    ERROR("Texture", "init",
      ("Failed to create texture with specified params. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  return S_OK;
}

HRESULT 
Texture::init(Device& device, Texture& textureRef, DXGI_FORMAT format) {
  if (!device.m_device) {
    ERROR("Texture", "init", "Device is null.");
    return E_POINTER;
  }
  if (!textureRef.m_texture) {
    ERROR("Texture", "init", "Texture is null.");
    return E_POINTER;
  }

  D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
  srvDesc.Format = format;
  srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  srvDesc.Texture2D.MipLevels = 1;
  srvDesc.Texture2D.MostDetailedMip = 0;

  HRESULT hr = device.m_device->CreateShaderResourceView(textureRef.m_texture,
                                                         &srvDesc,
                                                         &m_textureFromImg);

  if (FAILED(hr)) {
    ERROR("Texture", "init",
      ("Failed to create shader resource view for PNG textures. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  return S_OK;
}

void 
Texture::update() {

}

void 
Texture::render(DeviceContext& deviceContext, 
                unsigned int StartSlot, 
                unsigned int NumViews) {
  if (!deviceContext.m_deviceContext) {
    ERROR("Texture", "render", "Device Context is null.");
    return;
  }

  if (m_textureFromImg) {
    deviceContext.PSSetShaderResources(StartSlot, NumViews, &m_textureFromImg);
  }
}

void 
Texture::destroy() {
  if (m_texture != nullptr) {
    SAFE_RELEASE(m_texture);
  }
  if (m_textureFromImg != nullptr) {
    SAFE_RELEASE(m_textureFromImg);
  }
}

HRESULT 
Texture::CreateCubemap(Device& device, 
                       DeviceContext& deviceContext, 
                       const std::array<std::string, 6>& facePaths, 
                       bool generateMips) {
  destroy();

  stbi_set_flip_vertically_on_load(false);

  int width = 0, height = 0, channels = 0;
  std::array<unsigned char*, 6> facePixels{};
  facePixels.fill(nullptr);

  for (int i = 0; i < 6; ++i) {
		int w = 0, h = 0, c = 0;
    facePixels[i] = stbi_load(facePaths[i].c_str(), &w, &h, &c, 4);
    if (!facePixels[i]) {
      for (int k = 0; k < i; ++k) {
        if (facePixels[k]) {
          stbi_image_free(facePixels[k]);
        }
      }
      return E_FAIL;
    }

    if (i == 0) {
      width = w;
      height = h;
    }
    else if (w != width || h != height) {
      ERROR("Texture", "CreateCubemap", "All cubemap faces must have the same dimensions.");
      for (int k = 0; k <= i; ++k) {
        if (facePixels[k]) {
          stbi_image_free(facePixels[k]);
        }
      }
      return E_FAIL;
		}
  }

  D3D11_TEXTURE2D_DESC texDesc{};
  texDesc.Width = static_cast<unsigned int>(width);
  texDesc.Height = static_cast<unsigned int>(height);
  texDesc.MipLevels = generateMips ? 0 : 1;
  texDesc.ArraySize = 6;
  texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  texDesc.SampleDesc.Count = 1;
  texDesc.SampleDesc.Quality = 0;
  texDesc.Usage = D3D11_USAGE_DEFAULT;
  texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | (generateMips ? D3D11_BIND_RENDER_TARGET : 0);
  texDesc.CPUAccessFlags = 0;
  texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE | (generateMips ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0);

  HRESULT hr = S_OK;

  if (!generateMips) {
    std::array<D3D11_SUBRESOURCE_DATA, 6> initData{};
    for (int face = 0; face < 6; ++face)
    {
      initData[face].pSysMem = facePixels[face];
      initData[face].SysMemPitch = static_cast<unsigned int>(width * 4);
      initData[face].SysMemSlicePitch = 0;
    }

		hr = device.CreateTexture2D(&texDesc, initData.data(), &m_texture);
    if (FAILED(hr)) {
      for (auto* p : facePixels) {
        if (p) {
          stbi_image_free(p);
        }
      }
      return hr;
    }
  }
  else {
    hr = device.CreateTexture2D(&texDesc, nullptr, &m_texture);
    if (FAILED(hr)) {
      for (auto* p : facePixels) {
        if (p) {
          stbi_image_free(p);
        }
      }
      return hr;
    }

    UINT mipCount = 1 + (UINT)floor(log2(max(width, height)));

    for (UINT face = 0; face < 6; ++face)
    {
      UINT sub = D3D11CalcSubresource(0, face, mipCount);

      deviceContext.UpdateSubresource(
        m_texture,
        sub,
        nullptr,
        facePixels[face],
        width * 4,
        0
      );
    }
  }

  D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
  srvDesc.Format = texDesc.Format;
  srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
  srvDesc.TextureCube.MostDetailedMip = 0;
  srvDesc.TextureCube.MipLevels = generateMips ? (unsigned int)-1 : 1;
  
  hr = device.m_device->CreateShaderResourceView(m_texture, &srvDesc, &m_textureFromImg);

  if (FAILED(hr)) {
    for (auto* p : facePixels) {
      if (p) {
        stbi_image_free(p);
			}
    }
    destroy();
		return hr;
  }

  if (generateMips)
  {
    deviceContext.m_deviceContext->GenerateMips(m_textureFromImg);
  }

  for (auto* p : facePixels) {
    if (p) {
      stbi_image_free(p);
    }
  }

  m_textureName = "Cubemap";

  return S_OK;
}
