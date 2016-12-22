#pragma once


#include <map>

#include "Engine/Math/Vector2.hpp"

//-----------------------------------------------------------------------------
enum TextureFormat
{
	TEXTURE_FORMAT_Rgba8, //Rgba, 8 Bits Per Channel.
	TEXTURE_FORMAT_Depth24_Stencil8  //Depth 24-bits, Stencil 8-bits.
};


//-----------------------------------------------------------------------------
class Texture
{
public:
	static Texture* CreateOrGetTexture( const std::string& imageFilePath );

	static Texture* GetTextureByPath( const std::string& imageFilePath );
	unsigned int GetTextureID() const { return m_openglTextureID; }
	unsigned int GetWidth() const { return m_sizeInTexels.x; }
	unsigned int GetHeight() const { return m_sizeInTexels.y; }
	Vector2i GetTextureDimensions() const { return m_sizeInTexels; }
	static Texture* CreateTextureFromBytes( const std::string& textureName, const unsigned char* imageData, const Vector2i& textureSizeInTexels, unsigned int numComponents ); //Args from an stbi_load() prior to this call. 
	static Texture* CreateTextureFromNoData( const std::string& textureName, unsigned int width, unsigned int height, TextureFormat format ); //SD3, for FBOs.

private:
	Texture( const std::string& imageFilePath );
	Texture( const unsigned char* imageData, const Vector2i& textureSizeInTexels, unsigned int numComponents ); //Called by CreateFromBytes.
	Texture( unsigned int width, unsigned int height, TextureFormat format ); //Called by CreateFromNoData.

	static std::map<std::string, Texture*> s_textureRegistry;
	unsigned int m_openglTextureID;
	Vector2i m_sizeInTexels;
};