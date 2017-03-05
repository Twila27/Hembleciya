#include "Game/TheGame.hpp"
#include "Engine/Renderer/TheRenderer.hpp"
#include "Engine/Core/TheConsole.hpp"

#include "Game/Biomes/BiomeBlueprint.hpp"
#include "Game/Map.hpp"
#include "Game/Cell.hpp"
#include "Game/GameEntity.hpp"
#include "Game/Player.hpp"
#include "Game/Items/Item.hpp"
#include "Game/Features/Feature.hpp"


//--------------------------------------------------------------------------------------------------------------
bool TheGame::RenderStartup2D()
{
	bool didRender = false;

	g_theRenderer->DrawTextProportional2D
	(
		Vector2f( 100.f, (float)g_theRenderer->GetScreenHeight() - 50.f ),
		"Starting Up...",
		1.f
	);

	didRender = true;

	return didRender;
}


//--------------------------------------------------------------------------------------------------------------
bool TheGame::RenderMainMenu2D()
{
	bool didRender = false;

	g_theRenderer->DrawTextProportional2D
		(
			Vector2f( 100.f, (float)g_theRenderer->GetScreenHeight() - 50.f ),
			"Hembleciya",
			1.f
		);
	g_theRenderer->DrawTextProportional2D
		(
			g_theRenderer->GetScreenCenter() + 150.f,
			"(N)ew Game"
		);

	if ( m_foundSave )
		g_theRenderer->DrawTextProportional2D
		(
			g_theRenderer->GetScreenCenter() - 50.f,
			"(C)ontinue"
		);

	g_theRenderer->DrawTextProportional2D
		(
			g_theRenderer->GetScreenCenter() - 250.f,
			"(Q)uit"
		);

	didRender = true;

	return didRender;
}


//--------------------------------------------------------------------------------------------------------------
bool TheGame::RenderMapSelection2D()
{
	bool didRender = false;

	g_theRenderer->DrawTextProportional2D
		(
			Vector2f( 100.f, (float)g_theRenderer->GetScreenHeight() - 50.f ),
			"Quest Selection",
			1.f
		);

	std::string genModeText;
	switch ( g_generationMode )
	{
	case GENERATION_MODE_AUTO: genModeText = "Auto"; break;
	case GENERATION_MODE_MANUAL_FINITE_STEPS: genModeText = "Manual (Finite Steps)"; break;
	case GENERATION_MODE_MANUAL_INFINITE_STEPS: genModeText = "Manual (Infinite Steps)"; break;
	default: genModeText = "UNKNOWN";
	}

	g_theRenderer->DrawTextProportional2D
		(
			Vector2f( (float)g_theRenderer->GetScreenWidth() - 475.f, (float)g_theRenderer->GetScreenHeight() - 50.f ),
			"Generation Mode:",
			.25f,
			nullptr,
			Rgba::GREEN * Rgba::GRAY
		);
	g_theRenderer->DrawTextProportional2D
		(
			Vector2f( (float)g_theRenderer->GetScreenWidth() - 475.f, (float)g_theRenderer->GetScreenHeight() - 100.f ),
			Stringf( "%s", genModeText.c_str() ),
			.25f,
			nullptr,
			Rgba::GREEN
		);

	unsigned int numQuests = 0;
	auto biomeBlueprintIterEnd = BiomeBlueprint::GetRegistry().cend();

	for ( auto biomeBlueprintIter = BiomeBlueprint::GetRegistry().cbegin(); biomeBlueprintIter != biomeBlueprintIterEnd; ++biomeBlueprintIter )
	{
		const std::string& currentBiomeBlueprintName = biomeBlueprintIter->first;
		g_theRenderer->DrawTextProportional2D
			(
				Vector2f( 50.f, (float)g_theRenderer->GetScreenHeight() - ( 250 + ( 60.f * numQuests ) ) ),
				Stringf( "%d.) %s", numQuests + 1, currentBiomeBlueprintName.c_str() ),
				.35f
				);
		++numQuests;
	}

	static const std::string enterText = Stringf( "Enter: 1-%hhu", numQuests );
	g_theRenderer->DrawTextProportional2D
		(
			Vector2f( 50.f, 50.f ),
			enterText,
			.25f
		);
	g_theRenderer->DrawTextProportional2D
		(
			Vector2f( 100.f + g_theRenderer->CalcTextPxWidthUpToIndex( enterText, enterText.size(), .25f ), 50.f ),
			"Back: Escape",
			.25f
		);

	didRender = true;

	return didRender;
}


//--------------------------------------------------------------------------------------------------------------
bool TheGame::RenderMapGenerating2D()
{
	bool didRender = false;

	if ( m_currentMap == nullptr )
		return false;

	m_currentMap->Render();

	didRender = true;

	return didRender;
}


//--------------------------------------------------------------------------------------------------------------
bool TheGame::RenderPlaying2D()
{
	bool didRender = false;

	m_currentMap->Render();

	for ( auto entityIter = m_livingEntities.begin(); entityIter != m_livingEntities.end(); ++entityIter )
	{
		GameEntity* currentGameEntity = *entityIter;
		currentGameEntity->Render();
	}

	if ( m_player != nullptr )
	{
		g_theRenderer->DrawTextMonospaced2D
		(
			Vector2f( 50.f, 25.f ),
			Stringf( "Health: %d/%d", (int)m_player->GetHealth(), (int)m_player->GetMaxHealth() ),
			24.f,
			Rgba::RED + Rgba::GRAY
		);

		g_theRenderer->DrawTextMonospaced2D
		(
			Vector2f( (float)g_theRenderer->GetScreenWidth() - 350.f, 25.f ),
			Stringf( "Turn #: %d", (int)m_player->GetNumTurns() ),
			24.f,
			Rgba::CYAN * Rgba::GRAY
		);

		if ( !m_player->IsAlive() )
		{
			g_theConsole->HideConsole();

			static Rgba topLeftOverlayColor = Rgba::GRAY + Rgba::HALF_OPAQUE_BLACK;
			static Rgba topRightOverlayColor = Rgba::DARK_GRAY + Rgba::HALF_OPAQUE_BLACK;
			static Rgba bottomLeftOverlayColor = Rgba::DARK_GRAY + Rgba::HALF_OPAQUE_BLACK;
			static Rgba bottomRightOverlayColor = Rgba::BLACK + Rgba::HALF_OPAQUE_BLACK;

			byte_t newAlpha = static_cast<byte_t>( 255 * RangeMap( m_fadeoutTimer, m_FADEOUT_LENGTH_SECONDS, 0.f, 0.f, 1.f ) );
			topLeftOverlayColor.alphaOpacity = newAlpha;
			topRightOverlayColor.alphaOpacity = newAlpha;
			bottomLeftOverlayColor.alphaOpacity = newAlpha;
			bottomRightOverlayColor.alphaOpacity = newAlpha;

			g_theRenderer->DrawShadedAABB( TheRenderer::AS_QUADS, AABB2f( Vector2f::ZERO, Vector2f( (float)g_theRenderer->GetScreenWidth(), (float)g_theRenderer->GetScreenHeight() ) ),
										   topLeftOverlayColor, topRightOverlayColor, bottomLeftOverlayColor, bottomRightOverlayColor );

			g_theRenderer->DrawTextMonospaced2D
			(
				Vector2f( 350.f, 450.f ),
				"Game Over",
				96.f,
				Rgba( 255, 0, 0, newAlpha ),
				nullptr,
				1.f,
				false
			);
			g_theRenderer->DrawTextMonospaced2D
			(
				Vector2f( 350.f, 700.f ),
				"And now you too",
				48.f,
				Rgba( 172, 172, 172, newAlpha ),
				nullptr,
				1.f,
				false
			);
			g_theRenderer->DrawTextMonospaced2D
			(
				Vector2f( 150.f, 650.f ),
				"will never wake up again...",
				48.f,
				Rgba( 172, 172, 172, newAlpha ),
				nullptr,
				1.f,
				false
			);
			g_theRenderer->DrawTextMonospaced2D
			(
				Vector2f( 150.f, .5f * (float)g_theRenderer->GetScreenHeight() - 100.f ),
				Stringf( "# Monsters Killed: %d", (int)m_player->GetNumKills() ),
				24.f,
				Rgba( 0, 127, 127, newAlpha ),
				nullptr,
				1.f,
				false
			);
			g_theRenderer->DrawTextMonospaced2D
			(
				Vector2f( 950.f, .5f * (float)g_theRenderer->GetScreenHeight() - 100.f ),
				Stringf( "# Turns Survived: %d", (int)m_player->GetNumTurns() ),
				24.f,
				Rgba( 0, 127, 0, newAlpha ),
				nullptr,
				1.f,
				false
			);
		}
	}

	didRender = true;

	return didRender;
}


//--------------------------------------------------------------------------------------------------------------
bool TheGame::RenderPaused2D()
{
	bool didRender = false;

	g_theRenderer->DrawTextProportional2D
		(
			Vector2f( 100.f, (float)g_theRenderer->GetScreenHeight() - 50.f ),
			"PAUSED",
			1.f
			);

	didRender = true;

	return didRender;
}


//--------------------------------------------------------------------------------------------------------------
void TheGame::AddCarriedItemsToEntityListForAgent( const Agent* agent )
{
	std::vector< Item* > items = agent->GetEquipmentAndItems();
	m_livingEntities.insert( m_livingEntities.end(), items.begin(), items.end() );
}


//--------------------------------------------------------------------------------------------------------------
void TheGame::AddFeaturesToEntityListForMap( Map* map )
{
	std::vector< Cell >& cells = map->GetCells();
	for ( Cell& cell : cells )
	{
		if ( cell.IsOccupiedByFeature() )
		{
			Feature* feature = cell.m_occupyingFeature;
			m_livingEntities.push_back( feature );
			feature->AttachToMapAtPosition( m_currentMap, cell.m_position );
			m_currentMap->RefreshTraversableCells();
		}
	}

}


//--------------------------------------------------------------------------------------------------------------
bool TheGame::RenderShutdown2D()
{
	bool didRender = false;

	g_theRenderer->DrawTextProportional2D
		(
			Vector2f( 100.f, (float)g_theRenderer->GetScreenHeight() - 50.f ),
			"Shutting Down...",
			1.f
			);

	didRender = true;

	return didRender;
}


//--------------------------------------------------------------------------------------------------------------
bool TheGame::Render2D() //Called by TheEngine::Render().
{
	bool didRender = false;

	switch ( GetGameState() )
	{
		case GameState::GAME_STATE_STARTUP: didRender = RenderStartup2D(); break;
		case GameState::GAME_STATE_MAIN_MENU: didRender = RenderMainMenu2D(); break;
		case GameState::GAME_STATE_MAP_SELECTION: didRender = RenderMapSelection2D(); break;
		case GameState::GAME_STATE_MAP_GENERATING: didRender = RenderMapGenerating2D(); break;
		case GameState::GAME_STATE_PLAYING: didRender = RenderPlaying2D(); break;
		case GameState::GAME_STATE_PAUSED: didRender = RenderPaused2D(); break;
		case GameState::GAME_STATE_SHUTDOWN: didRender = RenderShutdown2D(); break;
	}
	if ( g_inDebugMode ) 
		RenderDebug2D();

	return didRender;
}


//-----------------------------------------------------------------------------
void TheGame::RenderDebug2D()
{
	RenderLeftDebugText2D();
	RenderRightDebugText2D();
}


//-----------------------------------------------------------------------------
void TheGame::RenderRightDebugText2D()
{
	g_theRenderer->DrawTextMonospaced2D
		(
			Vector2f( (float)g_theRenderer->GetScreenWidth() - 375.f, (float)g_theRenderer->GetScreenHeight() - 50.f ),
			Stringf( "GameState: %s", GetGameStateName( GetGameState() ) ),
			18.f,
			Rgba::GREEN,
			nullptr,
			.65f
		);
}


//-----------------------------------------------------------------------------
float g_shadowAlphaCounter = 0.f;
void TheGame::RenderLeftDebugText2D()
{
}
