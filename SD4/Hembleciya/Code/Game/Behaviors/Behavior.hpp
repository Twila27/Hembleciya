#pragma once

#include <string>
#include <map>
#include "Game/GameCommon.hpp"
struct XMLNode;
class Agent;


class Behavior
{
public:
	Behavior( const XMLNode& behaviorNode );
	virtual ~Behavior() {}
	virtual inline void operator=( const Behavior& other );


	virtual UtilityValue CalcUtility() = 0;
	virtual CooldownSeconds Run() = 0;
	bool IsAboveChanceToRun();
	void SetAgent( Agent* agent ) { m_agent = agent; }
	std::string GetName() const { return m_name; }
	virtual Behavior* CreateClone() const = 0;
	virtual void WriteToXMLNode( XMLNode& behaviorsNode );

protected:

	Agent* m_agent;
	std::string m_name;
	float m_chanceToRun;
	static const float s_DEFAULT_CHANCE_TO_RUN;
};

typedef Behavior* (BehaviorCreationFunc)( const XMLNode& behaviorNode );
class BehaviorRegistration
{
private:
	std::string m_name;
	BehaviorCreationFunc* m_creationFunc;


public:
	BehaviorRegistration( const std::string& name, BehaviorCreationFunc* creationFunc )
		: m_name( name ), m_creationFunc( creationFunc ) //May want to assert creationFunc != nullptr.
	{
		//We don't know the order of instantiation of static variables, hence if this is the first time through the function:
		if ( s_behaviorRegistry == nullptr )
			s_behaviorRegistry = new std::map< std::string, BehaviorRegistration* >(); //another typedef, see above if can't remember.

		if ( s_behaviorRegistry->find( name ) == s_behaviorRegistry->end() )
			s_behaviorRegistry->insert( std::pair< std::string, BehaviorRegistration* >( name, this ) );
	}
	static Behavior* CreateBehaviorByNameAndXML( const std::string& name, const XMLNode& behaviorNode );

	static const std::map< std::string, BehaviorRegistration* >* GetRegistry() {
		return s_behaviorRegistry;
	}

protected:
	static std::map< std::string, BehaviorRegistration* >* s_behaviorRegistry;
};
