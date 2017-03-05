#pragma once

#include <string>
#include <map>
#include <vector>
#include <memory>
#include "Engine/EngineCommon.hpp"

class Mesh;
class Material;
struct Rgba;
class Sampler;
class Texture;
class VertexDefinition;


class MeshRenderer
{
public:
	static MeshRenderer* CreateOrGetMeshRenderer( const std::string& meshRendererName, std::shared_ptr<Mesh> mesh, Material* material );
	static MeshRenderer* OverwriteMeshRenderer( const std::string& overwrittenMeshRendererName, std::shared_ptr<Mesh> mesh, Material* material );
	static MeshRenderer* CreateAndAddFboEffectRenderer( std::shared_ptr<Mesh> mesh, Material* material );
	static MeshRenderer* GetFboEffectRenderer( unsigned int index ) { return s_fboQuadRendererRegistry[index]; }
	static void ClearMeshRenderers();
	static const std::map< std::string, MeshRenderer* >* GetMeshRendererRegistry() { return &s_meshRendererRegistry; }
	static const std::vector< MeshRenderer* >* GetFboEffectRendererRegistry() { return &s_fboQuadRendererRegistry; }
	~MeshRenderer();

	const std::string& GetMaterialName() const;
	std::shared_ptr<Mesh> GetMesh() const { return m_mesh; };
	const VertexDefinition* GetVertexDefinition() const;

	void SetMesh( std::shared_ptr<Mesh> mesh );
	void SetMaterial( Material* material, bool overwriteMemberMaterial );
	void SetMeshAndMaterial( std::shared_ptr<Mesh> mesh, Material* material ) { SetMesh(mesh); SetMaterial(material, true); }

	void Render();
	void Render( Mesh* mesh, Material* material );

	//Just calls the Material equivalents.
	void SetInt( const std::string& uniformNameVerbatim, int* newValue );
	void SetFloat( const std::string& uniformNameVerbatim, float* newValue );
	void SetVector2( const std::string& uniformNameVerbatim, const Vector2f* newValue );
	void SetVector3( const std::string& uniformNameVerbatim, const Vector3f* newValue );
	void SetVector4( const std::string& uniformNameVerbatim, const Vector4f* newValue );
	void SetMatrix4x4( const std::string& uniformNameVerbatim, bool shouldTranspose, const Matrix4x4f* newValue );
	void SetColor( const std::string& uniformNameVerbatim, const Rgba* newValue );
	void SetSampler( const std::string& uniformNameVerbatim, unsigned int newSamplerID );
	void SetTexture( const std::string& uniformNameVerbatim, unsigned int newTextureID );


private:
	MeshRenderer( std::shared_ptr<Mesh> mesh, Material* material );

	void ResetVAO( Mesh* mesh, Material* material );
	static std::map< std::string, MeshRenderer* > s_meshRendererRegistry;
	static std::vector< MeshRenderer* > s_fboQuadRendererRegistry;

	unsigned int m_vaoID;

	//Using shared pointers because multiple MeshRenderers may otherwise delete meshes/materials still in use.
	std::shared_ptr<Mesh> m_mesh;
	Material* m_material;
};
