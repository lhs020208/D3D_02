//-----------------------------------------------------------------------------
// File: Scene.h
//-----------------------------------------------------------------------------

#pragma once

#include "Shader.h"
#include "Player.h"

class CScene
{
public:
	CScene() {}
    CScene(CPlayer* pPlayer);
    ~CScene();

	virtual void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	virtual void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseObjects();

	ID3D12RootSignature *CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);
	ID3D12RootSignature *GetGraphicsRootSignature() { return(m_pd3dGraphicsRootSignature); }

	bool ProcessInput();
	virtual void Animate(float fTimeElapsed);
	void PrepareRender(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera=NULL);

	void ReleaseUploadBuffers();
	void SetPlayer(CPlayer* pPlayer) { m_pPlayer = pPlayer; }
	void BuildGraphicsRootSignature(ID3D12Device* pd3dDevice);
protected:
	CPlayer* m_pPlayer = NULL;
protected:
	ID3D12RootSignature			*m_pd3dGraphicsRootSignature = NULL;

	CTitleObject				**m_ppObjects = 0;
	int							m_nObjects = 0;
};

class CTitleScene : public CScene {
public:
	CTitleScene::CTitleScene() {}
	CTitleScene(CPlayer* pPlayer);
	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) override;
	virtual void ReleaseObjects() override;
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera) override;
	virtual void Animate(float fElapsedTime) override;

	CGameObject* PickObjectPointedByCursor(int xClient, int yClient, CCamera* pCamera);
	virtual void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) override;
private:
	CTitleObject* m_pTitleObjects;
};