//-----------------------------------------------------------------------------
// File: CGameObject.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Mesh.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////

CPolygon::CPolygon(int nVertices)
{
	m_nVertices = nVertices;
	m_pVertices = new CVertex[nVertices];
}

CPolygon::~CPolygon()
{
	if (m_pVertices) delete[] m_pVertices;
}

void CPolygon::SetVertex(int nIndex, CVertex& vertex)
{
	if ((0 <= nIndex) && (nIndex < m_nVertices) && m_pVertices)
	{
		m_pVertices[nIndex] = vertex;
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////

CMesh::CMesh(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, char *pstrFileName)
{
	if (pstrFileName) LoadMeshFromFile(pd3dDevice, pd3dCommandList, pstrFileName);
}
CMesh::CMesh(int nPolygons)
{
	m_nPolygons = nPolygons;
	m_ppPolygons = new CPolygon * [nPolygons];
}

CMesh::~CMesh()
{
	if (m_pxmf3Positions) delete[] m_pxmf3Positions;
	if (m_pxmf3Normals) delete[] m_pxmf3Normals;
	if (m_pxmf2TextureCoords) delete[] m_pxmf2TextureCoords;

	if (m_pnIndices) delete[] m_pnIndices;

	if (m_pd3dVertexBufferViews) delete[] m_pd3dVertexBufferViews;

	if (m_pd3dPositionBuffer) m_pd3dPositionBuffer->Release();
	if (m_pd3dNormalBuffer) m_pd3dNormalBuffer->Release();
	if (m_pd3dTextureCoordBuffer) m_pd3dTextureCoordBuffer->Release();
	if (m_pd3dIndexBuffer) m_pd3dIndexBuffer->Release();
}

void CMesh::ReleaseUploadBuffers() 
{
	if (m_pd3dPositionUploadBuffer) m_pd3dPositionUploadBuffer->Release();
	if (m_pd3dNormalUploadBuffer) m_pd3dNormalUploadBuffer->Release();
	if (m_pd3dTextureCoordUploadBuffer) m_pd3dTextureCoordUploadBuffer->Release();
	if (m_pd3dIndexUploadBuffer) m_pd3dIndexUploadBuffer->Release();

	m_pd3dPositionUploadBuffer = NULL;
	m_pd3dNormalUploadBuffer = NULL;
	m_pd3dTextureCoordUploadBuffer = NULL;
	m_pd3dIndexUploadBuffer = NULL;
};

void CMesh::Render(ID3D12GraphicsCommandList *pd3dCommandList)
{
	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);
	pd3dCommandList->IASetVertexBuffers(m_nSlot, m_nVertexBufferViews, m_pd3dVertexBufferViews);
	if (m_pd3dIndexBuffer)
	{
		pd3dCommandList->IASetIndexBuffer(&m_d3dIndexBufferView);
		pd3dCommandList->DrawIndexedInstanced(m_nIndices, 1, 0, 0, 0);
	}
	else
	{
		pd3dCommandList->DrawInstanced(m_nVertices, 1, m_nOffset, 0);
	}
}

void CMesh::LoadMeshFromFile(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, char* filename)
{
	std::ifstream file(filename);
	if (!file.is_open()) return;

	std::vector<XMFLOAT3> vertices;
	std::vector<UINT> indices;

	std::string line;
	while (std::getline(file, line)) {
		std::istringstream iss(line);
		std::string prefix;
		iss >> prefix;

		if (prefix == "v") {
			float x, y, z;
			iss >> x >> y >> z;
			vertices.emplace_back(x, y, z);
		}
		else if (prefix == "f") {
			for (int i = 0; i < 3; ++i) {
				std::string token;
				iss >> token;
				std::istringstream tokenStream(token);
				std::string vIdx;
				std::getline(tokenStream, vIdx, '/');
				int idx = std::stoi(vIdx) - 1;
				indices.push_back(static_cast<UINT>(idx));
			}
		}
	}
	file.close();

	m_nVertices = static_cast<UINT>(vertices.size());
	m_nIndices = static_cast<UINT>(indices.size());

	m_pxmf3Positions = new XMFLOAT3[m_nVertices];
	memcpy(m_pxmf3Positions, vertices.data(), sizeof(XMFLOAT3) * m_nVertices);

	m_pnIndices = new UINT[m_nIndices];
	memcpy(m_pnIndices, indices.data(), sizeof(UINT) * m_nIndices);

	// ===== GPU 리소스 생성 =====

	// 1. 정점 버퍼
	UINT vbSize = sizeof(XMFLOAT3) * m_nVertices;
	m_pd3dPositionBuffer = CreateBufferResource(device, cmdList, m_pxmf3Positions, vbSize,
		D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dPositionUploadBuffer);

	m_nVertexBufferViews = 1;
	m_pd3dVertexBufferViews = new D3D12_VERTEX_BUFFER_VIEW[1];
	m_pd3dVertexBufferViews[0].BufferLocation = m_pd3dPositionBuffer->GetGPUVirtualAddress();
	m_pd3dVertexBufferViews[0].StrideInBytes = sizeof(XMFLOAT3);
	m_pd3dVertexBufferViews[0].SizeInBytes = vbSize;

	// 2. 인덱스 버퍼
	UINT ibSize = sizeof(UINT) * m_nIndices;
	m_pd3dIndexBuffer = CreateBufferResource(device, cmdList, m_pnIndices, ibSize,
		D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, &m_pd3dIndexUploadBuffer);

	m_d3dIndexBufferView.BufferLocation = m_pd3dIndexBuffer->GetGPUVirtualAddress();
	m_d3dIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
	m_d3dIndexBufferView.SizeInBytes = ibSize;

	// OBB 계산
	XMFLOAT3 min = vertices[0], max = vertices[0];
	for (const auto& v : vertices) {
		if (v.x < min.x) min.x = v.x;
		if (v.y < min.y) min.y = v.y;
		if (v.z < min.z) min.z = v.z;

		if (v.x > max.x) max.x = v.x;
		if (v.y > max.y) max.y = v.y;
		if (v.z > max.z) max.z = v.z;
	}

	XMFLOAT3 center = {
		(min.x + max.x) * 0.5f,
		(min.y + max.y) * 0.5f,
		(min.z + max.z) * 0.5f
	};
	XMFLOAT3 extent = {
		(max.x - min.x) * 0.5f,
		(max.y - min.y) * 0.5f,
		(max.z - min.z) * 0.5f
	};

	m_xmOOBB = BoundingOrientedBox(center, extent, XMFLOAT4(0, 0, 0, 1));
}

void CMesh::SetPolygon(int nIndex, CPolygon* pPolygon)
{
	if ((0 <= nIndex) && (nIndex < m_nPolygons)) m_ppPolygons[nIndex] = pPolygon;
}

int CMesh::CheckRayIntersection(XMVECTOR& xmvPickRayOrigin, XMVECTOR& xmvPickRayDirection, float* pfNearHitDistance)
{
	int nHits = 0;
	float fNearestHit = FLT_MAX;

	for (UINT i = 0; i < m_nIndices; i += 3)
	{
		XMVECTOR v0 = XMLoadFloat3(&m_pxmf3Positions[m_pnIndices[i]]);
		XMVECTOR v1 = XMLoadFloat3(&m_pxmf3Positions[m_pnIndices[i + 1]]);
		XMVECTOR v2 = XMLoadFloat3(&m_pxmf3Positions[m_pnIndices[i + 2]]);

		float fDist = 0.0f;
		if (TriangleTests::Intersects(xmvPickRayOrigin, xmvPickRayDirection, v0, v1, v2, fDist))
		{
			if (fDist < fNearestHit)
			{
				fNearestHit = fDist;
				nHits++;
				if (pfNearHitDistance) *pfNearHitDistance = fNearestHit;
			}
		}
	}

	return nHits;
}
BOOL CMesh::RayIntersectionByTriangle(XMVECTOR& xmRayOrigin, XMVECTOR& xmRayDirection, XMVECTOR v0, XMVECTOR v1, XMVECTOR v2, float* pfNearHitDistance)
{
	float fHitDistance;
	BOOL bIntersected = TriangleTests::Intersects(xmRayOrigin, xmRayDirection, v0, v1, v2, fHitDistance);
	if (bIntersected && (fHitDistance < *pfNearHitDistance)) *pfNearHitDistance = fHitDistance;

	return(bIntersected);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

CCubeMesh::CCubeMesh(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList,
	float fWidth, float fHeight, float fDepth) : CMesh(6)
{
	// 1. 정점 설정
	float w = fWidth * 0.5f, h = fHeight * 0.5f, d = fDepth * 0.5f;

	XMFLOAT3 vertices[] = {
		{-w, +h, -d}, {+w, +h, -d}, {+w, -h, -d}, {-w, -h, -d}, // Front
		{-w, +h, +d}, {+w, +h, +d}, {+w, -h, +d}, {-w, -h, +d}  // Back
	};
	m_nVertices = _countof(vertices);
	m_pxmf3Positions = new XMFLOAT3[m_nVertices];
	memcpy(m_pxmf3Positions, vertices, sizeof(vertices));

	// 2. 인덱스 설정 (삼각형 12개 → 36개 인덱스)
	UINT indices[] = {
		0,1,2, 0,2,3,  // Front
		4,6,5, 4,7,6,  // Back
		4,5,1, 4,1,0,  // Top
		3,2,6, 3,6,7,  // Bottom
		4,0,3, 4,3,7,  // Left
		1,5,6, 1,6,2   // Right
	};
	m_nIndices = _countof(indices);
	m_pnIndices = new UINT[m_nIndices];
	memcpy(m_pnIndices, indices, sizeof(indices));

	// 3. 정점 버퍼 생성
	UINT vbSize = sizeof(XMFLOAT3) * m_nVertices;
	m_pd3dPositionBuffer = CreateBufferResource(device, cmdList, m_pxmf3Positions, vbSize,
		D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dPositionUploadBuffer);

	m_nVertexBufferViews = 1;
	m_pd3dVertexBufferViews = new D3D12_VERTEX_BUFFER_VIEW[1];
	m_pd3dVertexBufferViews[0].BufferLocation = m_pd3dPositionBuffer->GetGPUVirtualAddress();
	m_pd3dVertexBufferViews[0].StrideInBytes = sizeof(XMFLOAT3);
	m_pd3dVertexBufferViews[0].SizeInBytes = vbSize;

	// 4. 인덱스 버퍼 생성
	UINT ibSize = sizeof(UINT) * m_nIndices;
	m_pd3dIndexBuffer = CreateBufferResource(device, cmdList, m_pnIndices, ibSize,
		D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, &m_pd3dIndexUploadBuffer);

	m_d3dIndexBufferView.BufferLocation = m_pd3dIndexBuffer->GetGPUVirtualAddress();
	m_d3dIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
	m_d3dIndexBufferView.SizeInBytes = ibSize;
}
CCubeMesh::CCubeMesh(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, int T) : CMesh(6)
{
	float fWidth = 0.1f;
	float fHeight = 0.1f;

	float t = T * 0.01f;

	XMFLOAT3 p0 = RollerCoasterPos(t);
	XMFLOAT3 p1 = RollerCoasterPos(t + 0.01f);

	XMVECTOR forward = XMVector3Normalize(XMLoadFloat3(&p1) - XMLoadFloat3(&p0));

	XMVECTOR worldUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	if (fabs(XMVectorGetX(XMVector3Dot(forward, worldUp))) > 0.99f) {
		worldUp = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
	}

	XMVECTOR right = XMVector3Normalize(XMVector3Cross(worldUp, forward));
	XMVECTOR up = XMVector3Normalize(XMVector3Cross(forward, right));

	right = XMVectorScale(right, fWidth * 0.5f);
	up = XMVectorScale(up, fHeight * 0.5f);

	XMVECTOR base0 = XMLoadFloat3(&p0);
	XMVECTOR base1 = XMLoadFloat3(&p1);

	XMVECTOR p0_v[4] = {
		base0 - right - up,
		base0 + right - up,
		base0 + right + up,
		base0 - right + up
	};

	XMVECTOR p1_v[4] = {
		base1 - right - up,
		base1 + right - up,
		base1 + right + up,
		base1 - right + up
	};

	// 1. 정점 구성 (8개)
	XMFLOAT3 vertices[8];
	for (int i = 0; i < 4; ++i) XMStoreFloat3(&vertices[i], p0_v[i]);
	for (int i = 0; i < 4; ++i) XMStoreFloat3(&vertices[i + 4], p1_v[i]);

	m_nVertices = 8;
	m_pxmf3Positions = new XMFLOAT3[8];
	memcpy(m_pxmf3Positions, vertices, sizeof(vertices));

	// 2. 인덱스 구성 (삼각형 12개)
	m_nIndices = 36;
	m_pnIndices = new UINT[m_nIndices];
	UINT fixedIndices[36] = {
		0,1,2, 0,2,3,
		5,4,7, 5,7,6,
		4,5,1, 4,1,0,
		3,2,6, 3,6,7,
		4,0,3, 4,3,7,
		1,5,6, 1,6,2
	};
	memcpy(m_pnIndices, fixedIndices, sizeof(fixedIndices));

	// 3. 정점 버퍼 생성
	UINT vbSize = sizeof(XMFLOAT3) * m_nVertices;
	m_pd3dPositionBuffer = CreateBufferResource(device, cmdList, m_pxmf3Positions, vbSize,
		D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dPositionUploadBuffer);

	m_nVertexBufferViews = 1;
	m_pd3dVertexBufferViews = new D3D12_VERTEX_BUFFER_VIEW[1];
	m_pd3dVertexBufferViews[0].BufferLocation = m_pd3dPositionBuffer->GetGPUVirtualAddress();
	m_pd3dVertexBufferViews[0].StrideInBytes = sizeof(XMFLOAT3);
	m_pd3dVertexBufferViews[0].SizeInBytes = vbSize;

	// 4. 인덱스 버퍼 생성
	UINT ibSize = sizeof(UINT) * m_nIndices;
	m_pd3dIndexBuffer = CreateBufferResource(device, cmdList, m_pnIndices, ibSize,
		D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, &m_pd3dIndexUploadBuffer);

	m_d3dIndexBufferView.BufferLocation = m_pd3dIndexBuffer->GetGPUVirtualAddress();
	m_d3dIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
	m_d3dIndexBufferView.SizeInBytes = ibSize;

	// 5. OOBB 설정
	m_xmOOBB = BoundingOrientedBox(p0, XMFLOAT3(0.01f, fHeight, fWidth), XMFLOAT4(0, 0, 0, 1));
	/*
	XMFLOAT3 center;
	XMStoreFloat3(&center, XMVectorScale(base0 + (base1 - base0), 0.5f));
	m_xmOOBB = BoundingOrientedBox(center, XMFLOAT3(0.01f, fHeight, fWidth), XMFLOAT4(0, 0, 0, 1));
	*/
	/*
	XMMATRIX rotation = XMMatrixIdentity();
	rotation.r[0] = right;  // x축
	rotation.r[1] = up;     // y축
	rotation.r[2] = forward; // z축
	rotation.r[3] = XMVectorSet(0, 0, 0, 1);

	XMFLOAT4 orientation;
	XMStoreFloat4(&orientation, XMQuaternionRotationMatrix(rotation));
	m_xmOOBB = BoundingOrientedBox(center, XMFLOAT3(0.01f, fHeight, fWidth), orientation);
	*/
}


CCubeMesh::~CCubeMesh()
{
}
