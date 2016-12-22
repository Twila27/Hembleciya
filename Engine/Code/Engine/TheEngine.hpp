#pragma once


//--------------------------------------------------------------------------------------------------------------
class TheEngine
{
public:
	void RunFrame();
	void Startup( double screenWidth, double screenHeight );
	void Shutdown();
	bool IsQuitting();

private:
	void Render();
	void Update( float deltaSeconds );

	void RenderDebug3D();
	void RenderDebug2D();
};


//--------------------------------------------------------------------------------------------------------------
extern TheEngine* g_theEngine;
