#pragma once


#include <map>
#include <string>
#include "Engine/FileUtils/XMLUtils.hpp"
#include "Engine/Math/Interval.hpp"
#include "Game/Features/Feature.hpp"


//-----------------------------------------------------------------------------
class Map;


//-----------------------------------------------------------------------------
class FeatureFactory;
typedef std::map< std::string, FeatureFactory* > FeatureFactoryCategory;


//--------------------------------------------------------------------------------------------------------------
class FeatureFactory
{
public:
	FeatureFactory( const XMLNode& featureBlueprintNode ) { PopulateFromXMLNode( featureBlueprintNode ); }
	static void LoadAllFeatureBlueprints();
	Feature* CreateFeature( Map* map = nullptr, const XMLNode& featureInstanceNode = XMLNode::emptyNode() );
	static FeatureFactoryCategory& GetRegistryForFeatureType( FeatureType type ) { return s_featureFactoryRegistry[ type ]; }
	static FeatureFactory* GetRandomFactoryForFeatureType( FeatureType type );


private:
	void PopulateFromXMLNode( const XMLNode& featureBlueprintNode = XMLNode::emptyNode() );
	void SetFeatureType( FeatureType type );
	//Constructor doesn't parse XML directly, instead calling this from constructor.
	//Public if we have to call it anywhere else as needed, can be protected/private hitherto.

	FeatureType m_factoryFeatureType;
	static FeatureFactoryCategory s_featureFactoryRegistry[ NUM_FEATURE_TYPES ]; //Index via FeatureType enum.
	std::string m_name;
	Feature* m_templateFeature;
	Interval<int> m_damageRange;
};
