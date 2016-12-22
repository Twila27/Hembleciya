#pragma once


#include <map>
#include <string>
#include "Engine/FileUtils/XMLUtils.hpp"
#include "Engine/Math/Interval.hpp"
class NPC;
class Map;


class NPCFactory
{
public:
	NPCFactory( const XMLNode& npcBlueprintNode ) { PopulateFromXMLNode( npcBlueprintNode ); }
	static void LoadAllNPCBlueprints();
	NPC* CreateNPC( Map* map = nullptr, const XMLNode& npcInstanceNode = XMLNode::emptyNode() );
	static std::map< std::string, NPCFactory* >& GetRegistry() { return s_npcFactoryRegistry; }


private:
	void PopulateFromXMLNode( const XMLNode& npcBlueprintNode = XMLNode::emptyNode() );
	//Constructor doesn't parse XML directly, instead calling this from constructor.
	//Public if we have to call it anywhere else as needed, can be protected/private hitherto.

	static std::map< std::string, NPCFactory* > s_npcFactoryRegistry;
	std::string m_name;
	NPC* m_templateNPC;
	Interval<int> m_maxHealthRange;
};
