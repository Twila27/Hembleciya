#include "Game/Generators/CellularAutomataGenerator.hpp"

#include "Engine/Math/MathUtils.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Map.hpp"


//--------------------------------------------------------------------------------------------------------------
STATIC GeneratorRegistration CellularAutomataGenerator::s_gameOfLifeCaves( "GameOfLifeCaves", &CellularAutomataGenerator::CreateGenerator, &Generator::CreateBiomeCreationProcess );
STATIC GeneratorRegistration CellularAutomataGenerator::s_harwardCaverns( "HarwardCaverns", &CellularAutomataGenerator::CreateGenerator, &Generator::CreateBiomeCreationProcess );


//--------------------------------------------------------------------------------------------------------------
Map* CellularAutomataGenerator::CreateMapAndInitializeCells( const Vector2i& size, const std::string& mapName )
{
	Map* map = new Map( size, mapName );

	//60% air (conceptually "alive"), 40% stone.
		//for loop, roll a die per cell with above chances
	for ( unsigned int cellIndex = 0; cellIndex < map->GetCells().size(); cellIndex++ )
	{
		if ( m_name == "GameOfLifeCaves" )
		{
			if ( GetRandomChance( 0.50f ) ) map->SetCellTypeForIndex( cellIndex, CellType::CELL_TYPE_AIR );
			else map->SetCellTypeForIndex( cellIndex, CellType::CELL_TYPE_STONE_WALL );
		}
		else if ( m_name == "HarwardCaverns" )
		{
			if ( GetRandomChance( 0.60f ) ) map->SetCellTypeForIndex( cellIndex, CellType::CELL_TYPE_AIR );
			else map->SetCellTypeForIndex( cellIndex, CellType::CELL_TYPE_STONE_WALL );
		}
	}

	return map;
}


//--------------------------------------------------------------------------------------------------------------
static void RunGameOfLifeStep( Map* map, int /*currentStepNumber*/, CellType livingType, CellType deadType )
{
	//Automata Rules @ bitstorm.org

	std::vector<Cell>& cells = map->GetCells();
	for ( unsigned int cellIndex = 0; cellIndex < cells.size(); cellIndex++ )
	{
		Cell& currentCell = cells[ cellIndex ];
		CellType& newType = currentCell.m_nextCellType;
		unsigned int numLivingNeighbors = map->GetNumNeighborsAroundCellOfType( currentCell.m_position, livingType, 1.f );

		if ( currentCell.m_cellType == livingType )
		{
			switch ( numLivingNeighbors )
			{
			case 2: //Surviving.
			case 3:	//Surviving.
				break;
			case 0: //Solitude.
			case 1: //Solitude.
			case 4: //Overpopulation.
			default: //Overpopulation.
				newType = deadType;
				continue;
			}
		}
		else
		{
			if ( numLivingNeighbors == 3 ) //Procreation!
			{
				newType = livingType;
				continue;
			}
		}
		newType = currentCell.m_cellType; //Stay same.
	}
}


//--------------------------------------------------------------------------------------------------------------
static void RunModifiedRulesStep( Map* map, int currentStepNumber, CellType livingType, CellType deadType )
{
	int numInitialPasses = ( GetRandomChance( .5f ) ? 3 : 4 );
	bool inFirstPhase = ( currentStepNumber <= numInitialPasses );

	std::vector<Cell>& cells = map->GetCells();
	for ( unsigned int cellIndex = 0; cellIndex < cells.size(); cellIndex++ )
	{
		Cell& currentCell = cells[ cellIndex ];
		CellType& newType = currentCell.m_nextCellType;

		if ( map->GetNumNeighborsAroundCellOfType( currentCell.m_position, deadType, 1.f ) >= 5 )
		{
			newType = inFirstPhase ? livingType : deadType; //Close off inaccessible tiny holes in first phase, open up passages in second.
			continue;
		}
		if ( inFirstPhase && ( map->GetNumNeighborsAroundCellOfType( currentCell.m_position, deadType, 2.f ) <= 2 ) )
		{
			newType = livingType; //Open things up more in first phase that are close to already open areas.
			continue;
		}

		newType = currentCell.m_cellType; //Stay same.
	}
}


//--------------------------------------------------------------------------------------------------------------
bool CellularAutomataGenerator::GenerateOneStep( Map* map, int currentStepNumber, BiomeGenerationProcess* metadata )
{
	UNREFERENCED( metadata );

	bool didGenerateStep = false;

	( m_name == "HarwardCaverns" ) ? 
		RunModifiedRulesStep( map, currentStepNumber, CELL_TYPE_AIR, CELL_TYPE_STONE_WALL ) :
		RunGameOfLifeStep( map, currentStepNumber, CELL_TYPE_AIR, CELL_TYPE_STONE_WALL );

	//Update to next type. Have to wait until all finish to validate neighbor-checks between iterations.
	std::vector<Cell>& cells = map->GetCells();
	for ( Cell& cell : cells )
		cell.m_cellType = cell.m_nextCellType;

	return didGenerateStep;
}
