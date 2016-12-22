#include "Game/NPCs/NPCFactory.hpp"
#include "Game/NPCs/NPC.hpp"
#include "Game/GameCommon.hpp"
#include <vector>
#include "Engine/FileUtils/FileUtils.hpp"
#include "Engine/Error/ErrorWarningAssert.hpp"
#include "Engine/String/StringUtils.hpp"
#include "Game/Behaviors/Behavior.hpp"


//--------------------------------------------------------------------------------------------------------------
std::map< std::string, NPCFactory* > NPCFactory::s_npcFactoryRegistry = std::map< std::string, NPCFactory* >();


//--------------------------------------------------------------------------------------------------------------
void NPCFactory::PopulateFromXMLNode( const XMLNode& npcBlueprintNode )
{
	m_maxHealthRange = ReadXMLAttribute( npcBlueprintNode, "maxHealth", m_maxHealthRange );

	m_templateNPC = new NPC( npcBlueprintNode, nullptr );
}


//--------------------------------------------------------------------------------------------------------------
STATIC void NPCFactory::LoadAllNPCBlueprints()
{
	//Note we may have more than one NPC in a file.
	std::vector< std::string > m_npcFiles = EnumerateFilesInDirectory( "Data/XML/NPCs", "*.NPC.xml" );

	for ( unsigned int npcFileIndex = 0; npcFileIndex < m_npcFiles.size(); npcFileIndex++ )
	{
		const char* xmlFilename = m_npcFiles[ npcFileIndex ].c_str();
		XMLNode blueprintsRoot = XMLNode::openFileHelper( xmlFilename, "NPCBlueprints" );

		for ( int npcBlueprintIndex = 0; npcBlueprintIndex < blueprintsRoot.nChildNode(); npcBlueprintIndex++ )
		{
			XMLNode npcBlueprintNode = blueprintsRoot.getChildNode( npcBlueprintIndex );

			NPCFactory* newFactory = new NPCFactory( npcBlueprintNode ); //Each NPCFactory corresponds to a NPCBlueprint element.
			newFactory->m_name = ReadXMLAttribute( npcBlueprintNode, "name", newFactory->m_name );

			if ( s_npcFactoryRegistry.find( newFactory->m_name ) != s_npcFactoryRegistry.end() )
				ERROR_AND_DIE( Stringf( "Found duplicate NPC factory type/name %s in LoadNPCBlueprints!", newFactory->m_name.c_str() ) );

			s_npcFactoryRegistry.insert( std::pair< std::string, NPCFactory* >( newFactory->m_name, newFactory ) );
		}
	}
}


//--------------------------------------------------------------------------------------------------------------
NPC* NPCFactory::CreateNPC( Map* map /*= nullptr*/, const XMLNode& npcInstanceNode /*= XMLNode::emptyNode()*/ )
{
	NPC* newNPC = new NPC( *m_templateNPC, map, npcInstanceNode ); //Note overridden copy constructor at work to perform deep copy.

	newNPC->SetMaxHealth( m_maxHealthRange.GetRandomElement() ); //Likewise for any interval--factory stores the range, gives instances to NPCs.
	return newNPC;
}
