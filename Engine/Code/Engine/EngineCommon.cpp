#include "Engine/EngineCommon.hpp"


bool g_inDebugMode = false;
DebugRenderMode g_currentDebugRenderMode = RENDER_MODE_NONE;


//--------------------------------------------------------------------------------------------------------------
Matrix4x4f GetWorldChangeOfBasis( Ordering ordering )
{
	//Having to send in [down, back, left] because of transpose.
	//My basis turns the default [x y z] to [-y z x], like SD2 SimpleMiner, 
	//But projection matrix then inverts k-vector, so we need to pass a matrix to turn [x y z] to [-y z -x].

	Matrix4x4f changeOfBasisTransform( ordering );
	if ( ordering == COLUMN_MAJOR )
	{
		const float values[ 16 ] = {
			0.f,	-1.f,	0.f,	0.f, // i == 0,-1,0 == -y for world right.
			0.f,	0.f,	1.f,	0.f, // j == 0, 0,1 == +z for world up.
			-1.f,	0.f,	0.f,	0.f, // k == -1,0,0 == -x for world BACKWARD, after perspective projection inverts k-vector.
			0.f,	0.f,	0.f,	1.f,
		};
		changeOfBasisTransform.SetAllValuesAssumingSameOrdering( values );
	}
	else if ( ordering == ROW_MAJOR )
	{
		const float values[ 16 ] = {
			0.f,	0.f,	-1.f,	0.f,
			-1.f,	0.f,	0.f,	0.f,
			0.f,	1.f,	0.f,	0.f,
			0.f,	0.f,	0.f,	1.f,
		};
		changeOfBasisTransform.SetAllValuesAssumingSameOrdering( values );
	}
	return changeOfBasisTransform;
}