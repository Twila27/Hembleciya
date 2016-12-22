#pragma once


#include <map>
#include "Engine/Math/AABB2.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"


class FixedBitmapFont
{
public:
	static FixedBitmapFont* CreateOrGetFont( const std::string& FixedBitmapFontName );
	AABB2f GetTexCoordsForGlyph( int glyphUnicode ) const;
	Texture* GetFontTexture() const {
		return m_spriteSheet.GetAtlasTexture();
	}

private:
	FixedBitmapFont( const std::string& FixedBitmapFontName );

	static std::map< std::string, FixedBitmapFont* > s_fontRegistry;
	SpriteSheet m_spriteSheet;

	static const int BITMAP_FONT_GLYPHS_WIDE = 16;
	static const int BITMAP_FONT_GLYPHS_HIGH = 16;
	static const int BITMAP_FONT_GLYPH_WIDTH = 16;
	static const int BITMAP_FONT_GLYPH_HEIGHT = 16;
};
