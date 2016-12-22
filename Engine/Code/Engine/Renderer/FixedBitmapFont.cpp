#include "Engine/Renderer/FixedBitmapFont.hpp"


#include "Engine/EngineCommon.hpp"

#define STATIC // Do-nothing indicator that method/member is static in class definition


//---------------------------------------------------------------------------
STATIC std::map< std::string, FixedBitmapFont* >	FixedBitmapFont::s_fontRegistry;


//--------------------------------------------------------------------------------------------------------------
FixedBitmapFont::FixedBitmapFont( const std::string& fixedBitmapFontName )
	: m_spriteSheet( fixedBitmapFontName, BITMAP_FONT_GLYPHS_WIDE, BITMAP_FONT_GLYPHS_HIGH, BITMAP_FONT_GLYPH_WIDTH, BITMAP_FONT_GLYPH_HEIGHT )
{
}


//--------------------------------------------------------------------------------------------------------------
FixedBitmapFont* FixedBitmapFont::CreateOrGetFont( const std::string& fixedBitmapFontName )
{
	if ( s_fontRegistry.find( fixedBitmapFontName ) != s_fontRegistry.end() ) return s_fontRegistry[ fixedBitmapFontName ];
	else s_fontRegistry[ fixedBitmapFontName ] = new FixedBitmapFont( fixedBitmapFontName );

	return s_fontRegistry[ fixedBitmapFontName ];
}


//--------------------------------------------------------------------------------------------------------------
AABB2f FixedBitmapFont::GetTexCoordsForGlyph( int glyphUnicode ) const
{
	return m_spriteSheet.GetTexCoordsFromSpriteIndex( glyphUnicode ); //Assumes ASCII maps directly to indices.
}