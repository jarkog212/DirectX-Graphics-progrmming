#pragma once
#ifndef MATERIAL_LIBRARY_H
#define MATERIAL_LIBRARY_H

#include "DefaultShader.h"
#include "DXF.h"
#include <map>

class MaterialLibrary
{
public:
	MaterialLibrary(TextureManager* textureMgr, D3D* renderer)
	{
		//! just a pointer, legacy compatibility with DX framework
		textureMgr_ = textureMgr;
		//! loads all textures
		loadTextures();
	
	// MATERIALS //
	
	//! define and initialise all materials here
	// TEST
		materials_["Test"] = new DefaultShader::MaterialBufferType();
		materials_["Test"]->diffuse = XMFLOAT4(1.f, 0.f, 0.5f, 1.f);
		materials_["Test"]->roughness = 0.f;
		materials_["Test"]->shadingType = (int)ShadingType::Normal;
	// LAND
		materials_["Land"] = new DefaultShader::MaterialBufferType();
		materials_["Land"]->roughness = 0.85f;
		materials_["Land"]->uvScale = {20.f,20.f};
	// FOLIAGE
		materials_["Foliage"] = new DefaultShader::MaterialBufferType();
		materials_["Foliage"]->diffuse = k_InvalidFloat4;
		materials_["Foliage"]->roughness = 0.9f;
		materials_["Foliage"]->shadingType = (int)ShadingType::Texture;
	// WATER
		materials_["Water"] = new DefaultShader::MaterialBufferType();
		materials_["Water"]->diffuse = XMFLOAT4(0.f, 0.f, 0.5f, -2.f);  //! below -1 culls it from light baking
		materials_["Water"]->roughness = 0.f;
		materials_["Water"]->uvScale = { 10.f, 10.f };
		materials_["Water"]->uvOffset = { 0, 0 };
		materials_["Water"]->shadingType = (int)ShadingType::Texture_Normal;	
	// BASE
		materials_["Base"] = new DefaultShader::MaterialBufferType();
		materials_["Base"]->diffuse = XMFLOAT4(1.f, 0.f, 0.5f, -1.f);
		materials_["Base"]->roughness = 0.9f;
		materials_["Base"]->shadingType = (int)ShadingType::Texture_Normal;
	
	// MODELS //
	//! define and initialise all models here

		models_["Cottage"] = new Model(renderer->getDevice(), renderer->getDeviceContext(), "res/models/cottage.obj");
		models_["Tree"] = new Model(renderer->getDevice(), renderer->getDeviceContext(), "res/models/tree.obj");
	
	};
	
	~MaterialLibrary() 
	{
		for (auto it : materials_)
			delete it.second;

		for (auto it : models_)
			delete it.second;

		materials_.clear();
	};
	
	//! accessors
	DefaultShader::MaterialBufferType* getMaterial(std::string key) { return materials_.find(key) != materials_.end() ? materials_[key] : NULL; };
	ID3D11ShaderResourceView* getTexture(const wchar_t* key) { return textureMgr_->getTexture(key); }
	Model* getMesh(std::string key) { return models_.find(key) != models_.end() ? models_[key] : NULL; }

private:
	std::map<std::string, DefaultShader::MaterialBufferType*> materials_;
	std::map<std::string, Model*> models_;
	TextureManager* textureMgr_;
	
	//! load all the textures to be used
	void loadTextures() 
	{
		textureMgr_->loadTexture(L"landscapeH", L"res/landscape.png");
		textureMgr_->loadTexture(L"grass", L"res/grass.png");
		textureMgr_->loadTexture(L"grassN", L"res/grassN.png");
		textureMgr_->loadTexture(L"stone1", L"res/stone.png");
		textureMgr_->loadTexture(L"stone1N", L"res/stoneN.png");
		textureMgr_->loadTexture(L"stone2", L"res/stone2.png");
		textureMgr_->loadTexture(L"tree", L"res/tree.png");
		textureMgr_->loadTexture(L"foliageBrush", L"res/foliage_brush.png");
		textureMgr_->loadTexture(L"water", L"res/waterSurface.png");
		textureMgr_->loadTexture(L"waterBelow", L"res/waterUnder.png");
		textureMgr_->loadTexture(L"cottageD", L"res/cottage_diffuse.png");
		textureMgr_->loadTexture(L"cottageN", L"res/cottage_normal.png");
		textureMgr_->loadTexture(L"tree3D", L"res/tree3D.png");
		textureMgr_->loadTexture(L"windM", L"res/windM.jpg");
		textureMgr_->loadTexture(L"tree3DW", L"res/treeWindBrush.png");
	}

};

#endif
