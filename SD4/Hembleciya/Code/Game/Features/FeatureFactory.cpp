#include "Game/Features/FeatureFactory.hpp"
#include "Game/GameCommon.hpp"
#include <vector>
#include "Engine/FileUtils/FileUtils.hpp"
#include "Engine/Error/ErrorWarningAssert.hpp"
#include "Engine/String/StringUtils.hpp"


//--------------------------------------------------------------------------------------------------------------
FeatureFactoryCategory FeatureFactory::s_featureFactoryRegistry[ NUM_FEATURE_TYPES ]; //Do we need to assign the maps?


//--------------------------------------------------------------------------------------------------------------
void FeatureFactory::PopulateFromXMLNode( const XMLNode& featureBlueprintNode )
{
	m_templateFeature = new Feature( featureBlueprintNode, nullptr );
}


//--------------------------------------------------------------------------------------------------------------
void FeatureFactory::SetFeatureType( FeatureType type )
{
	m_factoryFeatureType = type;
	m_templateFeature->SetFeatureType( type );
}


//--------------------------------------------------------------------------------------------------------------
STATIC void FeatureFactory::LoadAllFeatureBlueprints()
{
	//Note we may have more than one Feature in a file.
	std::vector< std::string > m_factoryFiles = EnumerateFilesInDirectory( "Data/XML/Features", "*.Feature.xml" );

	for ( unsigned int factoryFileIndex = 0; factoryFileIndex < m_factoryFiles.size(); factoryFileIndex++ )
	{
		const char* xmlFilename = m_factoryFiles[ factoryFileIndex ].c_str();
		XMLNode blueprintsRoot = XMLNode::openFileHelper( xmlFilename, "FeatureBlueprints" );

		for ( int featureBlueprintIndex = 0; featureBlueprintIndex < blueprintsRoot.nChildNode(); featureBlueprintIndex++ )
		{
			XMLNode featureBlueprintNode = blueprintsRoot.getChildNode( featureBlueprintIndex );

			FeatureFactory* newFactory = new FeatureFactory( featureBlueprintNode ); //Each FeatureFactory corresponds to a FeatureBlueprint element.
			newFactory->m_name = ReadXMLAttribute( featureBlueprintNode, "name", newFactory->m_name );

			std::string featureTypeAsString;
			featureTypeAsString = ReadXMLAttribute( featureBlueprintNode, "type", featureTypeAsString );
			newFactory->SetFeatureType( GetFeatureTypeForString( featureTypeAsString ) );

			FeatureFactoryCategory& categoryRegistry = s_featureFactoryRegistry[ newFactory->m_factoryFeatureType ];

			if ( categoryRegistry.find( newFactory->m_name ) != categoryRegistry.end() )
				ERROR_AND_DIE( Stringf( "Found duplicate Feature factory type/name %s in LoadAllFeatureBlueprints!", newFactory->m_name.c_str() ) );

			categoryRegistry.insert( std::pair< std::string, FeatureFactory* >( newFactory->m_name, newFactory ) );
		}
	}
}


//--------------------------------------------------------------------------------------------------------------
Feature* FeatureFactory::CreateFeature( Map* map /*= nullptr*/, const XMLNode& featureInstanceNode /*= XMLNode::emptyNode()*/ )
{
	Feature* newFeature = new Feature( *m_templateFeature, map, featureInstanceNode ); //Note overridden copy constructor at work to perform deep copy.

	return newFeature;
}


//---------------------------------------------- ----------------------------------------------------------------
FeatureFactory* FeatureFactory::GetRandomFactoryForFeatureType( FeatureType type )
{
	FeatureFactoryCategory& factories = s_featureFactoryRegistry[ type ];
	FeatureFactoryCategory::iterator factoryIter = factories.begin();
	std::advance( factoryIter, GetRandomIntInRange( 0, factories.size() - 1 ) );
	return factoryIter->second;
}
