#pragma once


//For default arguments.
#include "Engine/EngineCommon.hpp"
#include "Engine/Renderer/Vertexes.hpp"
#include <string>
#include <vector>
#include <memory>


//-----------------------------------------------------------------------------
class TheRenderer;
class BitmapFont;
class FixedBitmapFont;
class Texture;
class Sampler;
class FrameBuffer;
class Mesh;
class MeshRenderer;
class RenderState;
class MeshBuilder;
class Material;
class Skeleton;
class AnimationSequence;
class Camera3D;


//-----------------------------------------------------------------------------
extern TheRenderer* g_theRenderer;


//-----------------------------------------------------------------------------
class TheRenderer
{
public:

	static const unsigned int DEFAULT_TEXTURE_ID;
	static const unsigned int DEFAULT_SAMPLER_ID;
	static const RenderState DEFAULT_RENDER_STATE;
	static const RenderState DEFAULT_FBO_RENDER_STATE;
	
	enum VertexGroupingRule { AS_LINES, AS_POINTS, AS_LINE_LOOP, AS_LINE_STRIP, AS_TRIANGLES, AS_QUADS };
	unsigned int GetOpenGLVertexGroupingRule( unsigned int engineVertexGroupingRule );

	TheRenderer( double screenWidth, double screenHeight );
	~TheRenderer();

	void PreGameStartup();
	void PostGameStartup();
	void Shutdown();

	//State machine context commands.
	void ClearScreenToColor( const Rgba& colorToClearTo );
	void ClearScreenToColor( float red, float green, float blue );
	void ClearScreenDepthBuffer();

	void EnableDepthTesting( bool flagValue );
	void EnableBackfaceCulling( bool flagValue );
	void EnableAlphaTesting( bool flagValue );
	void SetAlphaFunc( int alphaComparatorFunction, float alphaComparatorValue );
	void SetDrawColor( float red, float green, float blue, float opacity ); 
	void SetLineWidth( float newLineWidth );
	void SetBlendFunc( int sourceBlend, int destinationBlend );
	void SetRenderFlag( int flagNameToSet );
	void SetPointSize( float thickness );
	void BindTexture( const Texture* texture );
	void UnbindTexture();

	void SetScreenDimensions( double screenWidth, double screenHeight );
	void SetOrtho( const Vector2f& bottomLeft, const Vector2f& topRight );
	void SetPerspective( float fovDegreesY, float aspect, float nearDist, float farDist );
	void TranslateView( const Vector2f& translation );
	void TranslateView( const Vector3f& translation );
	void RotateViewByDegrees( float degrees, const Vector3f& axisOfRotation = Vector3f(0.f, 0.f, 1.f) );
	void RotateViewByRadians( float radians, const Vector3f& axisOfRotation = Vector3f(0.f, 0.f, 1.f) );
	void ScaleView( float uniformScale );
	void PushView();
	void PopView();

	BitmapFont* GetDefaultFont() const { return m_defaultProportionalFont; }
	float CalcTextPxWidthUpToIndex( const std::string& inputText, unsigned int indexExclusive, float scale = .25f, const BitmapFont* font = nullptr );
	float CalcTextPxHeight( const std::string& inputText, float scale = .25f, const BitmapFont* font = nullptr );

	//Drawing commands.

	void DrawPoint( const Vector3f& position, float thickness, const Rgba& color = Rgba() );

	void DrawLine( const Vector2f& startPos, const Vector2f& endPos, const Rgba& startColor = Rgba(), const Rgba& endColor = Rgba(), float lineThickness = 1.f );
	void DrawLine( const Vector3f& startPos, const Vector3f& endPos, const Rgba& startColor = Rgba(), const Rgba& endColor = Rgba(), float lineThickness = 1.f );

	void DrawAABB( const int vertexGroupingRule, const AABB2f& bounds, const Texture& texture, const AABB2f& texCoords = AABB2f( 0.f, 0.f, 1.f, 1.f ), const Rgba& tint = Rgba(), float lineThickness = 1.f );
	void DrawAABB( const int vertexGroupingRule, const AABB3f& bounds, const Texture& texture, const AABB2f* texCoords, const Rgba& tint = Rgba(), float lineThickness = 1.f ); //Ptr b/c can't have array of refs.

	void DrawAABB( const int vertexGroupingRule, const AABB2f& bounds, const Rgba& color = Rgba(), float lineThickness = 1.f );
	void DrawAABB( const int vertexGroupingRule, const AABB3f& bounds, const Rgba& color = Rgba(), float lineThickness = 1.f );

	void DrawShadedAABB( const int vertexGroupingRule, const AABB2f& bounds, const Rgba& topLeftColor = Rgba(), const Rgba& topRightColor = Rgba(), const Rgba& bottomLeftColor = Rgba(), const Rgba& bottomRightColor = Rgba(), float lineThickness = 1.f );
	void DrawShadedAABB( const int vertexGroupingRule, const AABB3f& bounds, const Rgba& topLeftColor = Rgba(), const Rgba& topRightColor = Rgba(), const Rgba& bottomLeftColor = Rgba(), const Rgba& bottomRightColor = Rgba(), float lineThickness = 1.f );
	
	void DrawQuad( const int vertexGroupingRule, const Vector2f& topLeft, const Vector2f& topRight, const Vector2f& bottomRight, const Vector2f& bottomLeft, const Rgba& color = Rgba(), float lineThickness = 1.f );
	void DrawPolygon( const int vertexGroupingRule, const Vector2f& centerPos, float radius, float numSides, float degreesOffset, const Rgba& color = Rgba(), float lineThickness = 1.f);
	void DrawAnthonyCloudySphere( Vector3f position, float radius, float numPoints, const Rgba& tint = Rgba() );
	void DrawSphereGimbal( const int vertexGroupingRule, const Vector3f& centerPos, float radius, float numSides, const Rgba& tint = Rgba(), float lineThickness = 1.0f ); //AS_LINES is ideal.
	void DrawCylinder( const int vertexGroupingRule, const Vector3f& centerPos, float radius, float height, float numSlices, float numSidesPerSlice, const Rgba& tint = Rgba(), float lineThickness = 1.0f );

	void DrawTextProportional3D( const Vector3f &lowerLeftOriginPos, const std::string& inputText, const Vector3f& textPlaneUpDir, const Vector3f& textPlaneRightDir, float scale = .25f, const BitmapFont* font = nullptr, const Rgba& tint = Rgba(), bool drawDropShadow = true, const Rgba& shadowColor = Rgba::BLACK );
	void DrawTextProportional2D( const Vector2f& originPos, const std::string& inputText, float scale = .25f, const BitmapFont* font = nullptr, const Rgba& tint = Rgba(), bool drawDropShadow = true, const Rgba& shadowColor = Rgba::BLACK );
	void DrawTextMonospaced2D( const Vector2f& startBottomLeft, const std::string& asciiText, float cellHeight, const Rgba& tint = Rgba(), const FixedBitmapFont* font = nullptr, float cellAspect = 1.f, bool drawDropShadow = true );
	void DrawTextInBox2D( const AABB2f& textboxBounds, const Rgba& textboxColor, const std::string& text, int alignmentHorizontal = -1, int alignmentVertical = 1, float textScale = .25f, const BitmapFont* font = nullptr, const Rgba& tint = Rgba(), bool drawDropShadow = true, const Rgba& shadowColor = Rgba::BLACK );

	void DrawAxes( float length, float lineThickness = 1.f, float alphaOpacity = 1.f, bool drawZ = false );
	void DrawDebugAxes( float length = 1.f, float lineThickness = 1.f, bool drawZ = false );

	void DrawVertexArray_PCT( const int vertexGroupingRule, const std::vector< Vertex3D_PCT >& vertexArrayData, unsigned int vertexArraySize );
	void DrawVertexArray_PCT( const int vertexGroupingRule, const Vertex3D_PCT* vertexArrayData, unsigned int vertexArraySize );

	void DrawVbo_PCT( unsigned int vboID, int numVerts, VertexGroupingRule vertexGroupingRule );

	//SD3 A7 - FBO

	void BindFBO( FrameBuffer* fbo );
	void CopyFBO( FrameBuffer* sourceFBO, FrameBuffer* targetFBO = nullptr );

	//SD3 A5 - TBN
	Mesh* CreateQuadMesh3D_PCUTB( const AABB2f& planarBounds, float depth, const Rgba& tint = Rgba::WHITE, bool windClockwise = true );
	Mesh* CreateAxisTintedCubeMesh3D_PCUTB( const AABB3f& bounds );
	Mesh* CreateSphereMesh3D_PCUTB( const Vector3f& spherePos, float radiusSizeRho, float numCirclesVertical, float numCirclesHorizontal, const Rgba& tint = Rgba::WHITE );
	MeshRenderer* CreateQuadRenderer3D_PCUTB( const AABB2f& planarBounds, float depth, const Rgba& tint, const std::string& shaderProgramName, const RenderState& renderState );
	MeshRenderer* CreateCubeRenderer3D_PCUTB( const AABB3f& bounds, const std::string& shaderProgramName, const RenderState& renderState );
	MeshRenderer* CreateSphereRenderer3D_PCUTB( const Vector3f& spherePos, float radiusSizeRho, float numCirclesVertical, float numCirclesHorizontal, const Rgba& tint, const std::string& shaderProgramName, const RenderState& renderState );

	//AES A1 - MeshBuilder
	Mesh* CreateTriangleMesh3D( const VertexDefinition* vertexDefinition, const Vector3f& topLeft, const Vector3f& bottomLeft, const Vector3f& bottomRight, const Rgba& tint = Rgba::WHITE );

	Mesh* CreateSurfacePatchMesh3D( Expression graphingInput, const VertexDefinition* vertexDefinition,
									   float startX, float endX, unsigned int xSectionCount,
									   float startY, float endY, unsigned int ySectionCount,
									   bool storeMeshInSingleton = false,
									   const Rgba& tint = Rgba::WHITE );

	//AES A3 - MeshBuilder
	void AddModel( MeshBuilder* modelMesh );

	//AES A4/A5 - Skeletal Animation
	void AddSkeletonVisualization( Skeleton* skeleton ) 
	{ 
		//Too complex to tell if skeleton*'s are equal, so for now only supporting 1.
		DeleteSkeletons();
		m_skeletonVisualizations.push_back( skeleton );
	}
	void VisualizeCurrentSkeletons();
	int GetNumAddedSkeletons() const { return m_skeletonVisualizations.size(); }
	void DeleteSkeletons();
	Skeleton* GetLastSkeletonAdded() const { return m_skeletonVisualizations.back(); }

	void AddAnimationSequence( AnimationSequence* anim )
	{
		//Note allowance of dupes, use cases too complex to write an operator==.
		m_animationSequences.push_back( anim );
	}
	void ApplyAnimationToSkeleton( int numAnimation, int numSkeleton = 0 );
	int GetNumAddedAnimations() const { return m_animationSequences.size(); }
	void DeleteAnimations();
	AnimationSequence* GetLastAnimationAdded() const { return m_animationSequences.back(); }

	//Engine Overhaul
	void UpdateShaderTimersAndPositions( float deltaSeconds, const Camera3D* activeCam );
	void UpdateAndRenderPostProcess();
	void UpdateSceneMVP( const Camera3D* activeCam );
	void UpdateLights( float deltaSeconds, const Camera3D* activeCam );
	void Update( float deltaSeconds, const Camera3D* activeCam ); //Update uniforms for shader timers, scene MVP, and lights.
	void PreRenderStep();
	void PostRenderStep();
	FrameBuffer* GetCurrentFBO();
	bool IsPilotingLight();
	bool IsShowingAxes();
	bool IsShowingFBOs();
	void AddFboEffects( const char** fboEffectShaderProgramNames, unsigned int numNames );
	void AddFboEffect( const char* fboEffectShaderProgramName );
	void SetupView2D();
	void SetupView3D( const Camera3D* activeCamera );
	Vector2f GetScreenCenter() const;
	double GetScreenWidth() const { return m_screenWidth; }
	double GetScreenHeight() const { return m_screenHeight; }


private:

	void CreateBuiltInDefaults();


	BitmapFont* m_defaultProportionalFont;
	FixedBitmapFont* m_defaultMonospaceFont;
	Sampler* m_defaultSampler;
	Texture* m_defaultTexture;
	std::shared_ptr<Mesh> m_defaultFboQuad;

	unsigned int m_currentTextureID;
	FrameBuffer* m_currentFBO;
	std::vector< Skeleton* > m_skeletonVisualizations;
	std::vector< AnimationSequence* > m_animationSequences;

	double m_screenWidth, m_screenHeight;
	unsigned int m_screenWidthAsUnsignedInt, m_screenHeightAsUnsignedInt;

	static const float DROP_SHADOW_OFFSET;
};