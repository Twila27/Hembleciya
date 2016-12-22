#pragma once


#include <map>


enum ShaderType
{
	VERTEX_SHADER,
	FRAGMENT_SHADER,
	INVALID_SHADER,
	NUM_SHADER_TYPES
};


class Shader
{
public:

	static Shader* CreateOrGetShader( const std::string& shaderFilePath, ShaderType shaderType );
	inline unsigned int GetID() const {	return m_shaderID; }
	inline unsigned int GetShaderType() const { return m_shaderType; }


private:

	Shader( const std::string& sourceFilePath, ShaderType shaderType, unsigned int shaderID )
		: m_sourceFilePath( sourceFilePath )
		, m_shaderType( shaderType )
		, m_shaderID( shaderID ) 
	{
	}

	static std::map<std::string, Shader*> s_shaderRegistry;
	unsigned int m_shaderID;
	unsigned int m_shaderType;
	std::string m_sourceFilePath;
};