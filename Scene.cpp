//-----------------------------------------------------------------------------
// File: CScene.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Scene.h"
#include "GameFramework.h"
extern CGameFramework* g_pFramework;

CScene::CScene(CPlayer* pPlayer)
{
	m_pPlayer = pPlayer;
}

CScene::~CScene()
{
}

void CScene::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);

	CMesh *pUfoMesh = new CCubeMesh(pd3dDevice, pd3dCommandList, 1.0f, 1.0f, 1.0f);
	CMesh* pFlyerMesh = new CMesh(pd3dDevice, pd3dCommandList, "Models/Title.obj");

	m_nObjects = 5;
	m_ppObjects = new CTitleObject *[m_nObjects];

	CPseudoLightingShader *pShader = new CPseudoLightingShader();
	pShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	pShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	m_ppObjects[0] = new CTitleObject();
	m_ppObjects[0]->SetMesh(pUfoMesh);
	m_ppObjects[0]->SetShader(pShader);
	m_ppObjects[0]->SetPosition(6.0f, 0.0f, 13.0f);
	m_ppObjects[0]->SetColor(XMFLOAT3(0.7f, 0.0f, 0.0f));

	m_ppObjects[1] = new CTitleObject();
	m_ppObjects[1]->SetMesh(pUfoMesh);
	m_ppObjects[1]->SetShader(pShader);
	m_ppObjects[1]->SetPosition(10.0f, -2.0f, 8.0f);
	m_ppObjects[1]->SetColor(XMFLOAT3(0.0f, 0.7f, 0.0f));

	m_ppObjects[2] = new CTitleObject();
	m_ppObjects[2]->SetMesh(pUfoMesh);
	m_ppObjects[2]->SetShader(pShader);
	m_ppObjects[2]->SetPosition(-5.0f, -4.0f, 11.0f);
	m_ppObjects[2]->SetColor(XMFLOAT3(0.0f, 0.0f, 0.7f));

	m_ppObjects[3] = new CTitleObject();
	m_ppObjects[3]->SetMesh(pUfoMesh);
	m_ppObjects[3]->SetShader(pShader);
	m_ppObjects[3]->SetPosition(-10.0f, -2.0f, 8.0f);

	m_ppObjects[4] = new CTitleObject();
	m_ppObjects[4]->SetMesh(pFlyerMesh);
	m_ppObjects[4]->SetShader(pShader);
	m_ppObjects[4]->SetPosition(0.0f, 4.0f, 20.0f);
	m_ppObjects[4]->Rotate(0.0f, 0.0f, 0.0f);
	m_ppObjects[4]->SetColor(XMFLOAT3(0.25f, 0.75f, 0.65f));
}

ID3D12RootSignature *CScene::CreateGraphicsRootSignature(ID3D12Device *pd3dDevice)
{
	ID3D12RootSignature *pd3dGraphicsRootSignature = NULL;

	D3D12_ROOT_PARAMETER pd3dRootParameters[3];
	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[0].Constants.Num32BitValues = 4; //Time, ElapsedTime, xCursor, yCursor
	pd3dRootParameters[0].Constants.ShaderRegister = 0; //Time
	pd3dRootParameters[0].Constants.RegisterSpace = 0;
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[1].Constants.Num32BitValues = 19; //16 + 3
	pd3dRootParameters[1].Constants.ShaderRegister = 1; //World
	pd3dRootParameters[1].Constants.RegisterSpace = 0;
	pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[2].Constants.Num32BitValues = 35; //16 + 16 + 3
	pd3dRootParameters[2].Constants.ShaderRegister = 2; //Camera
	pd3dRootParameters[2].Constants.RegisterSpace = 0;
	pd3dRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = 0;
	d3dRootSignatureDesc.pStaticSamplers = NULL;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob *pd3dSignatureBlob = NULL;
	ID3DBlob *pd3dErrorBlob = NULL;
	D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void **)&pd3dGraphicsRootSignature);
	if (pd3dSignatureBlob) pd3dSignatureBlob->Release();
	if (pd3dErrorBlob) pd3dErrorBlob->Release();

	return(pd3dGraphicsRootSignature);
}

void CScene::ReleaseObjects()
{
	if (m_pd3dGraphicsRootSignature) m_pd3dGraphicsRootSignature->Release();
	if (m_ppObjects)
	{
		for (int j = 0; j < m_nObjects; j++) if (m_ppObjects[j]) delete m_ppObjects[j];
		delete[] m_ppObjects;
	}
}

void CScene::ReleaseUploadBuffers()
{
	if (m_ppObjects)
	{
		for (int j = 0; j < m_nObjects; j++) if (m_ppObjects[j]) m_ppObjects[j]->ReleaseUploadBuffers();
	}
}

void CScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
}
void CScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
}

bool CScene::ProcessInput()
{
	return(false);
}

void CScene::Animate(float fTimeElapsed)
{
	for (int j = 0; j < m_nObjects; j++)
	{
		m_ppObjects[j]->Animate(fTimeElapsed);
	}
}

void CScene::PrepareRender(ID3D12GraphicsCommandList* pd3dCommandList)
{
	pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);
}

void CScene::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pCamera->UpdateShaderVariables(pd3dCommandList);

	for (int j = 0; j < m_nObjects; j++)
	{
		if (m_ppObjects[j]) m_ppObjects[j]->Render(pd3dCommandList, pCamera);
	}
	if (m_pPlayer) m_pPlayer->Render(pd3dCommandList, pCamera);
}
void CScene::BuildGraphicsRootSignature(ID3D12Device* pd3dDevice)
{
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CTitleScene::CTitleScene(CPlayer* pPlayer) : CScene(pPlayer) {}
void CTitleScene::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) 
{
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);
	CMesh* cTitleMesh = new CMesh(pd3dDevice, pd3dCommandList, "Models/Title.obj");
	CCubeMesh* pCubeMesh = new CCubeMesh(pd3dDevice, pd3dCommandList, 0.05f, 0.05f, 0.05f);

	CPseudoLightingShader* pShader = new CPseudoLightingShader();
	pShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	pShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	m_pTitleObjects = new CTitleObject();
	m_pTitleObjects->SetMesh(cTitleMesh);
	m_pTitleObjects->SetColor(XMFLOAT3(1.0f, 0.0f, 0.0f));
	m_pTitleObjects->SetShader(pShader);
	m_pTitleObjects->SetPosition(0.0f, 0.0f, 1.0f);
	m_pTitleObjects->UpdateBoundingBox();

	m_pExplosionObjects = new CExplosionObject();
	m_pExplosionObjects->SetMesh(pCubeMesh);
	m_pExplosionObjects->SetShader(pShader);
	m_pExplosionObjects->SetColor(XMFLOAT3(1.0f, 0.0f, 0.0f));
	m_pExplosionObjects->SetPosition(0.0f, 0.0f, 1.0f);
	m_pExplosionObjects->UpdateBoundingBox();
}
void CTitleScene::ReleaseUploadBuffers() {
	if (m_pTitleObjects) m_pTitleObjects->ReleaseUploadBuffers();
	if (m_pExplosionObjects) m_pExplosionObjects->ReleaseUploadBuffers();
}
void CTitleScene::ReleaseObjects()
{
	if (m_pd3dGraphicsRootSignature) m_pd3dGraphicsRootSignature->Release();
	if (m_pExplosionObjects) delete m_pExplosionObjects;
	if (m_pTitleObjects) delete m_pTitleObjects;
}
void CTitleScene::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pCamera->UpdateShaderVariables(pd3dCommandList);

	if (m_pTitleObjects && m_pExplosionObjects) {
		if (m_pTitleObjects->IsBlowingUp()) {
			m_pExplosionObjects->Render(pd3dCommandList, pCamera);
		}
		else
		m_pTitleObjects->Render(pd3dCommandList, pCamera);
	}
}

void CTitleScene::Animate(float fElapsedTime)
{
	if (m_pTitleObjects) {
		m_pTitleObjects->Animate(fElapsedTime);
		if (m_pTitleObjects->IsBlowingUp()) {
			for (int i = 0; i < EXPLOSION_DEBRISES; i++) {
				m_pExplosionObjects->m_pxmf4x4Transforms[i] = m_pTitleObjects->m_pxmf4x4Transforms[i];
				m_pExplosionObjects->m_pxmf3SphereVectors[i] = m_pTitleObjects->m_pxmf3SphereVectors[i];
			}
		}
	}

}
CGameObject* CTitleScene::PickObjectPointedByCursor(int xClient, int yClient, CCamera* pCamera)
{

	XMFLOAT3 xmf3PickPosition;
	xmf3PickPosition.x = (((2.0f * xClient) / (float)pCamera->m_d3dViewport.Width) - 1) / pCamera->m_xmf4x4Projection._11;
	xmf3PickPosition.y = -(((2.0f * yClient) / (float)pCamera->m_d3dViewport.Height) - 1) / pCamera->m_xmf4x4Projection._22;
	xmf3PickPosition.z = 1.0f;

	XMVECTOR xmvPickPosition = XMLoadFloat3(&xmf3PickPosition);
	XMMATRIX xmmtxView = XMLoadFloat4x4(&pCamera->m_xmf4x4View);

	float fNearestHitDistance = FLT_MAX;
	CGameObject* pNearestObject = NULL;
	if (m_pTitleObjects)
	{
		int hit = m_pTitleObjects->PickObjectByRayIntersection(xmvPickPosition, xmmtxView, &fNearestHitDistance);
		if (hit > 0)
		{
			pNearestObject = m_pTitleObjects;
		}
	}
	return(pNearestObject);
}

void CTitleScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_LBUTTONDOWN:
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);
		CCamera* pCamera = m_pPlayer->GetCamera();  //¿©±â¼­ È®º¸
		CGameObject* pPickedObject = PickObjectPointedByCursor(x, y, pCamera);

		if (pPickedObject) {
			if (!m_pTitleObjects->IsBlowingUp()) {
				m_pTitleObjects->PrepareExplosion();
			}
		}

	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CMenuScene::CMenuScene(CPlayer* pPlayer) : CScene(pPlayer) {}
void CMenuScene::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);

	CPseudoLightingShader* pShader = new CPseudoLightingShader();
	pShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	pShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	for (int i = 0; i < m_nCubeObjects; i++)
	{
		m_pCubeObjects[i] = nullptr;

		m_pCubeObjects[i] = new CGameObject();
		switch (i)
		{
		case 4:
		{
			CMesh* pCubeMesh = new CMesh(pd3dDevice, pd3dCommandList, "Models/Tutorial.obj");
			m_pCubeObjects[i]->SetMesh(pCubeMesh);
			break;
		}
		case 3:
		{
			CMesh* pCubeMesh = new CMesh(pd3dDevice, pd3dCommandList, "Models/Level-1.obj");
			m_pCubeObjects[i]->SetMesh(pCubeMesh);
			break;
		}
		case 2:
		{
			CMesh* pCubeMesh = new CMesh(pd3dDevice, pd3dCommandList, "Models/Level-2.obj");
			m_pCubeObjects[i]->SetMesh(pCubeMesh);
			break;
		}
		case 1:
		{
			CMesh* pCubeMesh = new CMesh(pd3dDevice, pd3dCommandList, "Models/Start.obj");
			m_pCubeObjects[i]->SetMesh(pCubeMesh);
			break;
		}
		case 0:
		{
			CMesh* pCubeMesh = new CMesh(pd3dDevice, pd3dCommandList, "Models/End.obj");
			m_pCubeObjects[i]->SetMesh(pCubeMesh);
			break;
		}
		}
		m_pCubeObjects[i]->SetColor(XMFLOAT3(1.0f, 0.0f, 0.0f));
		m_pCubeObjects[i]->SetShader(pShader);
		m_pCubeObjects[i]->SetPosition(-0.8f, -0.58f + 0.35f * i, 1.0f);
		m_pCubeObjects[i]->UpdateBoundingBox();

		XMFLOAT3 cameraPos = m_pPlayer->GetCamera()->GetPosition();
		XMFLOAT3 upVector = XMFLOAT3(0.0f, 1.0f, 0.0f);
		m_pCubeObjects[i]->LookAt(cameraPos, upVector);

		XMMATRIX rotationY = XMMatrixRotationY(XMConvertToRadians(180.0f));
		XMMATRIX world = XMLoadFloat4x4(&m_pCubeObjects[i]->m_xmf4x4World);
		world = XMMatrixMultiply(rotationY, world);
		XMStoreFloat4x4(&m_pCubeObjects[i]->m_xmf4x4World, world);
	}

}
void CMenuScene::ReleaseUploadBuffers() {
	for (int i = 0; i < m_nCubeObjects; i++) {
		if (m_pCubeObjects[i]) m_pCubeObjects[i]->ReleaseUploadBuffers();
	}
}

void CMenuScene::ReleaseObjects()
{
	if (m_pd3dGraphicsRootSignature) m_pd3dGraphicsRootSignature->Release();
	for (int i = 0; i < m_nCubeObjects; i++) {
		if (m_pCubeObjects[i]) 
			delete m_pCubeObjects[i];
	}
}
void CMenuScene::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pCamera->UpdateShaderVariables(pd3dCommandList);

	if (m_pPlayer) m_pPlayer->Render(pd3dCommandList, pCamera);
	for (int i = 0; i < m_nCubeObjects; i++) {
		if (m_pCubeObjects[i]) m_pCubeObjects[i]->Render(pd3dCommandList, pCamera);
	}
}
void CMenuScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	extern CGameFramework* g_pFramework;
	switch (nMessageID)
	{
	case WM_LBUTTONDOWN:
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);
		CCamera* pCamera = m_pPlayer->GetCamera();  //¿©±â¼­ È®º¸
		CGameObject* pPickedObject = PickObjectPointedByCursor(x, y, pCamera);

		if (pPickedObject)
		{
			for (int i = 0; i < m_nCubeObjects; i++)
			{
				if (pPickedObject == m_pCubeObjects[i])
				{
					// ÀÎµ¦½ºº°·Î ºÐ±â
					switch (i)
					{
					case 0:
						OutputDebugString(L"Cube 0 Å¬¸¯µÊ\n"); // End
						SendMessage(hWnd, WM_CLOSE, 0, 0);
						break;
					case 1:
						OutputDebugString(L"Cube 1 Å¬¸¯µÊ\n"); //Start
						g_pFramework->ChangeScene(2);
						break;
					case 2:
						OutputDebugString(L"Cube 2 Å¬¸¯µÊ\n"); //L2
						g_pFramework->ChangeScene(3);
						break;
					case 3:
						OutputDebugString(L"Cube 3 Å¬¸¯µÊ\n"); //L1
						g_pFramework->ChangeScene(2);
						break;
					case 4:
						OutputDebugString(L"Cube 4 Å¬¸¯µÊ\n"); //Tutorial
						break;
					}
					break;
				}
			}
		}

	}
}
CGameObject* CMenuScene::PickObjectPointedByCursor(int xClient, int yClient, CCamera* pCamera)
{
	XMFLOAT3 xmf3PickPosition;
	xmf3PickPosition.x = (((2.0f * xClient) / (float)pCamera->m_d3dViewport.Width) - 1) / pCamera->m_xmf4x4Projection._11;
	xmf3PickPosition.y = -(((2.0f * yClient) / (float)pCamera->m_d3dViewport.Height) - 1) / pCamera->m_xmf4x4Projection._22;
	xmf3PickPosition.z = 1.0f;

	XMVECTOR xmvPickPosition = XMLoadFloat3(&xmf3PickPosition);
	XMMATRIX xmmtxView = XMLoadFloat4x4(&pCamera->m_xmf4x4View);

	float fNearestHitDistance = FLT_MAX;
	CGameObject* pNearestObject = NULL;
	for (int i = 0; i < m_nCubeObjects; i++) {
		if (m_pCubeObjects)
		{
			int hit = m_pCubeObjects[i]->PickObjectByRayIntersection(xmvPickPosition, xmmtxView, &fNearestHitDistance);
			if (hit > 0) {
				pNearestObject = m_pCubeObjects[i];
			}
		}
	}
	return(pNearestObject);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CRollerCoasterScene::CRollerCoasterScene(CPlayer* pPlayer) : CScene(pPlayer) {}
void CRollerCoasterScene::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);
	CPseudoLightingShader* pShader = new CPseudoLightingShader();
	pShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	pShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	m_ppObjects = new CGameObject * [m_nObjects];

	for (int i = 0; i < m_nObjects; i++) {

		CCubeMesh* pCubeMesh = new CCubeMesh(pd3dDevice, pd3dCommandList, i);
		m_ppObjects[i] = new CGameObject();
		m_ppObjects[i]->SetMesh(pCubeMesh);
		m_ppObjects[i]->SetColor(XMFLOAT3(0.0f, 1.0f, 0.0f));
		m_ppObjects[i]->SetShader(pShader);
		m_ppObjects[i]->SetPosition(0.0f, 0.0f, 0.0f);
		m_ppObjects[i]->UpdateBoundingBox();
	}
}
void CRollerCoasterScene::ReleaseUploadBuffers() 
{
	for (int i = 0; i < m_nObjects; i++) {
		if (m_ppObjects[i]) m_ppObjects[i]->ReleaseUploadBuffers();
	}
}
void CRollerCoasterScene::ReleaseObjects()
{

	if (m_pd3dGraphicsRootSignature) m_pd3dGraphicsRootSignature->Release();
	for (int i = 0; i < m_nObjects; i++) {
		if (m_ppObjects[i]) delete m_ppObjects[i];
	}
	delete[] m_ppObjects;
}
void CRollerCoasterScene::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera) {
	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pCamera->UpdateShaderVariables(pd3dCommandList);

	if (m_pPlayer) m_pPlayer->Render(pd3dCommandList, pCamera);
	for (int i = 0; i < m_nObjects; i++) {
		m_ppObjects[i]->Render(pd3dCommandList, pCamera);
	}
}

void CRollerCoasterScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	extern CGameFramework* g_pFramework;
	switch (nMessageID)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case 'o':
		case 'O':
			m_pPlayer->overview = true;
			break;
		case 'l':
		case 'L':
			m_pPlayer->overview = false;
			XMFLOAT3 start_pos = RollerCoasterPos(timer);
			m_pPlayer->reset();
			m_pPlayer->SetPosition(start_pos.x, start_pos.y, start_pos.z);
			m_pPlayer->SetCameraOffset(XMFLOAT3(0.0f, 0.1f, -1.5f));
			break;
		case 'n':
		case 'N':
			g_pFramework->ChangeScene(3);
			break;
		case 'm':
		case 'M':
			XMFLOAT3 playerPos = m_pPlayer->GetPosition();
			XMFLOAT3 objectPos = m_ppObjects[0]->GetPosition();

			wchar_t msg[128];
			swprintf_s(msg, 128, L"[DEBUG] Player Pos: (%.2f, %.2f, %.2f)\n", playerPos.x, playerPos.y, playerPos.z);
			OutputDebugString(msg);

			swprintf_s(msg, 128, L"[DEBUG] Object[0] Pos: (%.2f, %.2f, %.2f)\n", objectPos.x, objectPos.y, objectPos.z);
			OutputDebugString(msg);
			break;
		case VK_ESCAPE:
			g_pFramework->ChangeScene(1);
			break;
		case VK_SPACE:
			move = true;
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}

void CRollerCoasterScene::Animate(float fElapsedTime)
{
	extern CGameFramework* g_pFramework;
	if (move) {
		m_pPlayer->SetPosition(RollerCoasterPos(timer).x, RollerCoasterPos(timer).y, RollerCoasterPos(timer).z);
		timer += speed;
		if (timer >= 0.71) {
			speed = 0.01;
		}
		if (timer >= 1.42) {
			speed = 0.005;
		}
	}
	if (timer >= 15.14) {
		move = false;
		timer = 0.0;
		speed = 0.002;
		g_pFramework->ChangeScene(3);
	}
}
//ÅÊÅ© Scene////////////////////////////////////////////////////////////////////////////////////////////////
CTankScene::CTankScene(CPlayer* pPlayer) : CScene(pPlayer) {}
void CTankScene::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);
	CPseudoLightingShader* pShader = new CPseudoLightingShader();
	pShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	pShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	using namespace std;
	default_random_engine dre{ random_device{}() };
	uniform_real_distribution<float> uid{ 0.0f,1.0f };

	uniform_real_distribution<float> uid_x{ 0,18.0f };
	uniform_real_distribution<float> uid_z{ 0,18.0f };
	uniform_int_distribution<int> uid_x_int(-9, 9);
	uniform_int_distribution<int> uid_z_int(-9, 9);
	uniform_real_distribution<float> uid_rot{ 0,360.0f };

	for (int i = 0; i < m_nTanks; i++)
	{
		float red = uid(dre);
		float green = uid(dre);
		float blue = uid(dre);

		m_pTank[i] = nullptr;
		m_pTank[i] = new CTankObject();
		CMesh* pTankMesh = new CMesh(pd3dDevice, pd3dCommandList, "Models/Tank.obj");
		m_pTank[i]->SetMesh(pTankMesh);
		m_pTank[i]->SetShader(pShader);
		m_pTank[i]->SetColor(XMFLOAT3(red, green, blue));
		m_pTank[i]->SetPosition(uid_x(dre) - 9.0f, 0.0f, uid_z(dre) - 9.0f);
		m_pTank[i]->Rotate(0.0f, uid_rot(dre), 0.0f);
		m_pTank[i]->UpdateBoundingBox();

		m_pTank[i]->bullet = new CGameObject();
		CMesh* pMesh = new CMesh(pd3dDevice, pd3dCommandList, "Models/Bullet.obj");
		m_pTank[i]->bullet->SetMesh(pMesh);
		m_pTank[i]->bullet->SetColor(XMFLOAT3(red, green, blue));
		m_pTank[i]->bullet->SetPosition(-2.0f + 0.5f * i, 0.0f, 1.0f);
		m_pTank[i]->bullet->SetShader(pShader);
		m_pTank[i]->bullet->UpdateBoundingBox();

		CCubeMesh* pCubeMesh = new CCubeMesh(pd3dDevice, pd3dCommandList, 0.05f, 0.05f, 0.05f);
		m_pTank[i]->m_pExplosionObjects = new CExplosionObject();
		m_pTank[i]->m_pExplosionObjects->SetMesh(pCubeMesh);
		m_pTank[i]->m_pExplosionObjects->SetShader(pShader);
		m_pTank[i]->m_pExplosionObjects->SetColor(XMFLOAT3(red, green, blue));
		m_pTank[i]->m_pExplosionObjects->SetPosition(0.0f, 0.0f, 1.0f);
		m_pTank[i]->m_pExplosionObjects->UpdateBoundingBox();
	}
	for (int i = 0; i < m_nCubeObjects; i++) {
		CCubeMesh* pCubeMesh = new CCubeMesh(pd3dDevice, pd3dCommandList, 1.0f, 1.0f, 1.0f);
		m_pCubeObjects[i] = new CCubeObject();
		m_pCubeObjects[i]->SetMesh(pCubeMesh);
		m_pCubeObjects[i]->SetShader(pShader);
		m_pCubeObjects[i]->SetColor(XMFLOAT3(0.0f, 0.0f, 1.0f));
		m_pCubeObjects[i]->SetPosition((float)uid_x_int(dre), 0.3f, (float)uid_z_int(dre));
		m_pCubeObjects[i]->UpdateBoundingBox();
	}
	CCubeMesh* pCubeMesh = new CCubeMesh(pd3dDevice, pd3dCommandList, 1.0f, 0.0f, 1.0f);
	m_pFloorObject = new CCubeObject();
	m_pFloorObject->SetMesh(pCubeMesh);
	m_pFloorObject->SetShader(pShader);
	m_pFloorObject->SetColor(XMFLOAT3(1.0f, 1.0f, 1.0f));
	m_pFloorObject->SetPosition(0.0f, -0.2f, 0.0f);
	m_pFloorObject->UpdateBoundingBox();

	CMesh* cTitleMesh = new CMesh(pd3dDevice, pd3dCommandList, "Models/YouWin.obj");
	m_pYWObjects = new CTitleObject();
	m_pYWObjects->SetMesh(cTitleMesh);
	m_pYWObjects->SetColor(XMFLOAT3(1.0f, 0.0f, 0.0f));
	m_pYWObjects->SetShader(pShader);
	m_pYWObjects->SetPosition(0.0f, 1.0f, 0.0f);
	m_pYWObjects->UpdateBoundingBox();
}

void CTankScene::ReleaseObjects()
{
	if (m_pd3dGraphicsRootSignature) m_pd3dGraphicsRootSignature->Release();
	if (m_pFloorObject) delete m_pFloorObject;
	for (int i = 0; i < m_nTanks; i++) {
		if (m_pTank[i]->bullet)delete m_pTank[i]->bullet;
		if (m_pTank[i]->m_pExplosionObjects)delete m_pTank[i]->m_pExplosionObjects;
		if (m_pTank[i])delete m_pTank[i];
	}
	for (int i = 0; i < m_nCubeObjects; i++)
		if (m_pCubeObjects[i])delete m_pCubeObjects[i];
	if (m_pYWObjects) delete m_pYWObjects;
}
void CTankScene::ReleaseUploadBuffers()
{
	if (m_pFloorObject) m_pFloorObject->ReleaseUploadBuffers();
	for (int i = 0; i < m_nTanks; i++) {
		if (m_pTank[i]) m_pTank[i]->ReleaseUploadBuffers();
		if (m_pTank[i]->m_pExplosionObjects) m_pTank[i]->m_pExplosionObjects->ReleaseUploadBuffers();
	}
	for (int i = 0; i < m_nCubeObjects; i++) {
		if (m_pCubeObjects[i]) m_pCubeObjects[i]->ReleaseUploadBuffers();
	}
	if (m_pYWObjects) m_pYWObjects->ReleaseUploadBuffers();
}
void CTankScene::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pCamera->UpdateShaderVariables(pd3dCommandList);

	if (m_pPlayer) m_pPlayer->Render(pd3dCommandList, pCamera);
	if (m_pFloorObject) {
		for (int i = 0; i < 20; i++) {
			for (int j = 0; j < 20; j++) {
				m_pFloorObject->SetPosition(-10.0f + 1.0f * i, -0.2f, -10.0f + 1.0f * j);
				m_pFloorObject->Render(pd3dCommandList, pCamera);
			}
		}
	}
	for (int i = 0; i < m_nTanks; i++) {
		
		if (m_pTank[i]->IsExist()) {
			if (m_pTank[i]->IsBlowingUp()) {
				m_pTank[i]->m_pExplosionObjects->Render(pd3dCommandList, pCamera);
			}
			else {
				m_pTank[i]->Render(pd3dCommandList, pCamera);
			}
		}
	}

	for (int i = 0; i < m_nCubeObjects; i++) {
		m_pCubeObjects[i]->Render(pd3dCommandList, pCamera);
	}
	if (m_pYWObjects && GameSet >= 10) m_pYWObjects->Render(pd3dCommandList, pCamera);
}
void CTankScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	extern CGameFramework* g_pFramework;
	CTankPlayer* pTankPlayer = dynamic_cast<CTankPlayer*>(m_pPlayer);
	switch (nMessageID)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case 'W':
			if (m_pPlayer->move_z < 1)m_pPlayer->move_z += 1;
			break;
		case 'S':
			if (m_pPlayer->move_z > -1)m_pPlayer->move_z -= 1;
			break;
		case 'A':
			if (m_pPlayer->move_x > -1)m_pPlayer->move_x -= 1;
			break;
		case 'D':
			if (m_pPlayer->move_x < 1)m_pPlayer->move_x += 1;
			break;
		case VK_SPACE:
			for (int i = 0; i < 10; i++)
				if (!m_pTank[i]->IsBlowingUp()) {
					m_pTank[i]->PrepareExplosion();
					GameSet++;
				}
			break;
		case 'Q':
			if (pTankPlayer) {
				pTankPlayer->SwitchShild();
			}
			break;
		case 'E':
			if (pTankPlayer && !pTankPlayer->shot) {
				pTankPlayer->SwitchBullet();
				pTankPlayer->SetBulletPosition();
			}
			break;
		case VK_ESCAPE:
			g_pFramework->ChangeScene(1);
			break;
		case VK_RETURN:
			if (pTankPlayer->Toggle) {
				pTankPlayer->Toggle = false;
				m_pFloorObject->SetColor(XMFLOAT3(1.0f, 1.0f, 1.0f));
			}
			else {
				pTankPlayer->Toggle = true;
				m_pFloorObject->SetColor(XMFLOAT3(1.0f, 0.0f, 0.0f));
			}
			break;
		default:
			break;
		}
		break;
	case WM_KEYUP:
		switch (wParam)
		{
		case 'W':
			if (m_pPlayer->move_z > -1)m_pPlayer->move_z -= 1;
			break;
		case 'S':
			if (m_pPlayer->move_z < 1)m_pPlayer->move_z += 1;
			break;
		case 'A':
			if (m_pPlayer->move_x < 1)m_pPlayer->move_x += 1;
			break;
		case 'D':
			if (m_pPlayer->move_x > -1)m_pPlayer->move_x -= 1;
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}
CGameObject* CTankScene::PickObjectPointedByCursor(int xClient, int yClient, CCamera* pCamera)
{

	XMFLOAT3 xmf3PickPosition;
	xmf3PickPosition.x = (((2.0f * xClient) / (float)pCamera->m_d3dViewport.Width) - 1) / pCamera->m_xmf4x4Projection._11;
	xmf3PickPosition.y = -(((2.0f * yClient) / (float)pCamera->m_d3dViewport.Height) - 1) / pCamera->m_xmf4x4Projection._22;
	xmf3PickPosition.z = 1.0f;

	XMVECTOR xmvPickPosition = XMLoadFloat3(&xmf3PickPosition);
	XMMATRIX xmmtxView = XMLoadFloat4x4(&pCamera->m_xmf4x4View);

	float fNearestHitDistance = FLT_MAX;
	CGameObject* pNearestObject = NULL;
	for (int i = 0; i < m_nTanks; i++) {
		if (m_pTank[i])
		{
			int hit = m_pTank[i]->PickObjectByRayIntersection(xmvPickPosition, xmmtxView, &fNearestHitDistance);
			if (hit > 0)
			{
				pNearestObject = m_pTank[i];
			}
		}
	}
	return(pNearestObject);

}
void CTankScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	CTankPlayer* pTankPlayer = dynamic_cast<CTankPlayer*>(m_pPlayer);
	switch (nMessageID)
	{
	case WM_RBUTTONDOWN:
	{
		if (pTankPlayer->Toggle) {

			int x = LOWORD(lParam);
			int y = HIWORD(lParam);
			CCamera* pCamera = m_pPlayer->GetCamera();

			CGameObject* pPickedObject = PickObjectPointedByCursor(x, y, pCamera);

			if (pPickedObject) {
				for (int i = 0; i < m_nTanks; i++) {
					if (pPickedObject == m_pTank[i]) {
						pTankPlayer->ToggleObject = m_pTank[i];
					}
				}
			}

			break;
		}
	}
	}
}
void CTankScene::CheckTankByBulletCollisions()
{
	CTankPlayer* pTankPlayer = dynamic_cast<CTankPlayer*>(m_pPlayer);
	if (pTankPlayer && pTankPlayer->shot) {
		for (int i = 0; i < 10; i++)
		{
			if (m_pTank[i]->IsExist())
				if (m_pTank[i]->m_xmOOBB.Intersects(pTankPlayer->m_pBullet->m_xmOOBB))
				{
					if (!m_pTank[i]->IsBlowingUp()) {
						m_pTank[i]->PrepareExplosion();
						pTankPlayer->shot = false;
						pTankPlayer->bullet_timer = 0;
						pTankPlayer->ToggleObject = NULL;
						GameSet++;
					}
				}
		}
	}
}
void CTankScene::CheckPlayerByBulletCollisions()
{
	CTankPlayer* pTankPlayer = dynamic_cast<CTankPlayer*>(m_pPlayer);
	if (pTankPlayer) {
		for (int i = 0; i < m_nTanks; i++)
		{
			if (!pTankPlayer->OnShild) {
				if (m_pTank[i]->IsExist() && m_pTank[i]->IsShot())
					if (pTankPlayer->m_xmOOBB.Intersects(m_pTank[i]->bullet->m_xmOOBB))
					{
						XMFLOAT3 color = m_pTank[i]->m_xmf3Color;

						pTankPlayer->SetColor(color);
						pTankPlayer->m_pBullet->SetColor(color);
						pTankPlayer->m_pShild->SetColor(color);

						m_pTank[i]->SwitchShot();
					}
			}
			else {
				if (m_pTank[i]->IsExist() && m_pTank[i]->IsShot())
					if (pTankPlayer->m_pShild->m_xmOOBB.Intersects(m_pTank[i]->bullet->m_xmOOBB))
					{
						m_pTank[i]->SwitchShot();
					}
			}
		}
	}
}

void CTankScene::CheckPlayerByObjectCollisions(float fElapsedTime)
{
	CTankPlayer* pTankPlayer = dynamic_cast<CTankPlayer*>(m_pPlayer);
	if (!pTankPlayer) return;

	const XMFLOAT3& moveVec = pTankPlayer->GetMoveVector();

	BoundingOrientedBox movedBox = pTankPlayer->m_xmOOBB;
	movedBox.Center.x += moveVec.x;
	movedBox.Center.z += moveVec.z;

	bool blocked = false;
	for (int i = 0; i < m_nCubeObjects; i++) {
		if (m_pCubeObjects[i] && movedBox.Intersects(m_pCubeObjects[i]->m_xmOOBB)) {
			blocked = true;
			break;
		}
	}

	if (!blocked) {
		XMFLOAT3 nowPos = pTankPlayer->GetPosition();
		pTankPlayer->SetPosition(nowPos.x + moveVec.x, nowPos.y, nowPos.z + moveVec.z);
		if (pTankPlayer->m_pShild) {
			pTankPlayer->m_pShild->SetPosition(nowPos.x + moveVec.x, nowPos.y, nowPos.z + moveVec.z);
		}
	}

	pTankPlayer->UpdateBoundingBox();
	if (pTankPlayer->m_pShild) pTankPlayer->m_pShild->UpdateBoundingBox();

	pTankPlayer->ClearMoveVector();
}

void CTankScene::CheckBulletByObjectCollisions()
{
	CTankPlayer* pTankPlayer = dynamic_cast<CTankPlayer*>(m_pPlayer);
	if (pTankPlayer) {
		for (int i = 0; i < m_nCubeObjects; i++)
		{
			if (m_pCubeObjects[i] && pTankPlayer->shot)
				if (pTankPlayer->m_pBullet->m_xmOOBB.Intersects(m_pCubeObjects[i]->m_xmOOBB))
				{
					pTankPlayer->shot = false;
					pTankPlayer->bullet_timer = 0;
					pTankPlayer->ToggleObject = NULL;
				}
		}
	}
}

void CTankScene::Animate(float fElapsedTime)
{
	for (int i = 0; i < m_nTanks; i++) {
		if (m_pTank[i]) {
			m_pTank[i]->Animate(fElapsedTime);
			if (m_pTank[i]->IsBlowingUp()) {
				for (int j = 0; j < EXPLOSION_DEBRISES; j++) {
					m_pTank[i]->m_pExplosionObjects->m_pxmf4x4Transforms[j] = m_pTank[i]->m_pxmf4x4Transforms[j];
					m_pTank[i]->m_pExplosionObjects->m_pxmf3SphereVectors[j] = m_pTank[i]->m_pxmf3SphereVectors[j];
				}
			}
		}
	}

	CTankPlayer* pTankPlayer = dynamic_cast<CTankPlayer*>(m_pPlayer);
	pTankPlayer->Animate(fElapsedTime);
	if (m_pYWObjects && GameSet >= 10) m_pYWObjects->Animate(fElapsedTime);

	CheckPlayerByObjectCollisions(fElapsedTime);
	CheckBulletByObjectCollisions();
	CheckTankByBulletCollisions();
	CheckPlayerByBulletCollisions();
}