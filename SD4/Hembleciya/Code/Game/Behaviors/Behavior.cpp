#include "Game/Behaviors/Behavior.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/FileUtils/XMLUtils.hpp"


//--------------------------------------------------------------------------------------------------------------
std::map< std::string, BehaviorRegistration* >* BehaviorRegistration::s_behaviorRegistry = nullptr;
STATIC const float Behavior::s_DEFAULT_CHANCE_TO_RUN = 1.f;


//--------------------------------------------------------------------------------------------------------------
Behavior::Behavior( const XMLNode& behaviorElement )
	: m_agent( nullptr )	
	, m_chanceToRun( s_DEFAULT_CHANCE_TO_RUN )
	, m_name( behaviorElement.getName() )
{
	m_chanceToRun = ReadXMLAttribute( behaviorElement, "chanceToRun", m_chanceToRun );
}


//--------------------------------------------------------------------------------------------------------------
void Behavior::operator=( const Behavior& other )
{
	m_agent = other.m_agent;
	m_chanceToRun = other.m_chanceToRun;
	m_name = other.m_name;
}


//--------------------------------------------------------------------------------------------------------------
bool Behavior::IsAboveChanceToRun()
{
	return m_chanceToRun >= GetRandomFloatZeroTo( 1.f );
}


//--------------------------------------------------------------------------------------------------------------
void Behavior::WriteToXMLNode( XMLNode& behaviorsNode )
{
	XMLNode behaviorNode = behaviorsNode.addChild( m_name.c_str() );
	WriteXMLAttribute( behaviorNode, "chanceToRun", m_chanceToRun, s_DEFAULT_CHANCE_TO_RUN );
}

//--------------------------------------------------------------------------------------------------------------
Behavior* BehaviorRegistration::CreateBehaviorByNameAndXML( const std::string& name, const XMLNode& behaviorNode )
{
	Behavior* behavior = nullptr;
	BehaviorRegistration* behaviorRegistration = nullptr;

	ASSERT_OR_DIE( s_behaviorRegistry != nullptr, "BehaviorRegistry Was Found Null in BehaviorRegistration::CreateBehaviorByNameAndXML!" );

	std::map< std::string, BehaviorRegistration* >::iterator behaviorRegistrationIter = s_behaviorRegistry->find( name );

	if ( behaviorRegistrationIter != s_behaviorRegistry->end() )
	{
		behaviorRegistration = behaviorRegistrationIter->second;
		behavior = ( *behaviorRegistration->m_creationFunc )( behaviorNode ); //ACTUAL POINT OF INSTANTIATION OF SUBCLASS!
	}
	else DebuggerPrintf( "WARNING: CreateBehaviorByNameAndXML Did Not Find %s", name );

	return behavior;
}