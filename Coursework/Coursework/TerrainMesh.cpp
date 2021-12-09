#include "TerrainMesh.h"
#define TERRAIN_X_LEN 32
#define TERRAIN_Z_LEN 32
#define TERRAIN_SCALE 100.f	//Careful! Modifying this value needs adjustments on the Pixel Shader for the normal estimation

TerrainMesh::TerrainMesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	initBuffers(device);
}

TerrainMesh::~TerrainMesh()
{
	// Run parent deconstructor
	BaseMesh::~BaseMesh();
}

void TerrainMesh::sendData(ID3D11DeviceContext* deviceContext, D3D_PRIMITIVE_TOPOLOGY top)
{
	unsigned int stride;
	unsigned int offset;

	stride = sizeof(VertexType);
	offset = 0;

	deviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	deviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	// Set the type of primitive that should be rendered from this vertex buffer, in this case control patch for tessellation.
	deviceContext->IASetPrimitiveTopology(top);
}

int TerrainMesh::clamp(int n, int lower, int upper)
{
	//if n < lower return lower
	//if n > upper return upper
	//else return n
	return (n < lower) * lower + (n > upper) * upper + (n >= lower && n <= upper) * n;
}

#define book_clamp(value, minimum, maximum) (max( min( (value), (maximum) ), (minimum) ))
void TerrainMesh::initBuffers(ID3D11Device* device)
{
	VertexType* vertices;
	unsigned long* indices;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;

	vertexCount = (TERRAIN_X_LEN + 1) * (TERRAIN_Z_LEN + 1);
	indexCount = 12 * (TERRAIN_X_LEN) * (TERRAIN_Z_LEN);

	vertices = new VertexType[vertexCount];
	indices = new unsigned long[indexCount];

	//Create vertex buffer
	float fWidht = static_cast<float>(TERRAIN_X_LEN);
	float fHeight = static_cast<float>(TERRAIN_Z_LEN);
	for (int x = 0; x < TERRAIN_X_LEN + 1; ++x)
	{
		for (int z = 0; z < TERRAIN_Z_LEN + 1; ++z)
		{
			float fX = static_cast<float>(x) / fWidht - .5f;
			float fZ = static_cast<float>(z) / fHeight - .5f;
			vertices[x + z * (TERRAIN_X_LEN + 1)].position = XMFLOAT3(fX * TERRAIN_SCALE, 0.f, fZ * TERRAIN_SCALE);
			vertices[x + z * (TERRAIN_X_LEN + 1)].texture = XMFLOAT2(fX + .5f, fZ + .5f);
			vertices[x + z * (TERRAIN_X_LEN + 1)].normal = XMFLOAT3(0.f, 1.f, 0.f);
		}
	}

	//Create index buffer
	int index = 0;
	for (int x = 0; x < TERRAIN_X_LEN; ++x)
	{
		for (int z = 0; z < TERRAIN_Z_LEN; ++z, index += 12)
		{
			//Define 12 control points per terrain quad

			//0-3 are the actual quad vertices
			indices[index + 0] = (z + 0) + (x + 0) * (TERRAIN_X_LEN + 1);
			indices[index + 1] = (z + 1) + (x + 0) * (TERRAIN_X_LEN + 1);
			indices[index + 2] = (z + 0) + (x + 1) * (TERRAIN_X_LEN + 1);
			indices[index + 3] = (z + 1) + (x + 1) * (TERRAIN_X_LEN + 1);

			//4-5 are +x
			indices[index + 4] = book_clamp(z + 0, 0, TERRAIN_Z_LEN) + book_clamp(x + 2, 0, TERRAIN_X_LEN) * (TERRAIN_X_LEN + 1);
			indices[index + 5] = book_clamp(z + 1, 0, TERRAIN_Z_LEN) + book_clamp(x + 2, 0, TERRAIN_X_LEN) * (TERRAIN_X_LEN + 1);

			//6-7 are +z
			indices[index + 6] = book_clamp(z + 2, 0, TERRAIN_Z_LEN) + book_clamp(x + 0, 0, TERRAIN_X_LEN) * (TERRAIN_X_LEN + 1);
			indices[index + 7] = book_clamp(z + 2, 0, TERRAIN_Z_LEN) + book_clamp(x + 1, 0, TERRAIN_X_LEN) * (TERRAIN_X_LEN + 1);

			//8-9 are -x
			indices[index + 8] = book_clamp(z + 0, 0, TERRAIN_Z_LEN) + book_clamp(x - 1, 0, TERRAIN_X_LEN) * (TERRAIN_X_LEN + 1);
			indices[index + 9] = book_clamp(z + 1, 0, TERRAIN_Z_LEN) + book_clamp(x - 1, 0, TERRAIN_X_LEN) * (TERRAIN_X_LEN + 1);

			//10-11 are -z
			indices[index + 10] = book_clamp(z - 1, 0, TERRAIN_Z_LEN) + book_clamp(x + 0, 0, TERRAIN_X_LEN) * (TERRAIN_X_LEN + 1);
			indices[index + 11] = book_clamp(z - 1, 0, TERRAIN_Z_LEN) + book_clamp(x + 1, 0, TERRAIN_X_LEN) * (TERRAIN_X_LEN + 1);
		}
	}

	// Set up the description of the static vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;
	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;
	// Now create the vertex buffer.
	device->CreateBuffer(&vertexBufferDesc, &vertexData, &vertexBuffer);

	// Set up the description of the static index buffer.
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;
	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;
	// Create the index buffer.
	device->CreateBuffer(&indexBufferDesc, &indexData, &indexBuffer);

	// Release the arrays now that the vertex and index buffers have been created and loaded.
	delete[] vertices;
	vertices = 0;
	delete[] indices;
	indices = 0;
}
