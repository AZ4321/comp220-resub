#include "Mesh.h"

Mesh::Mesh()
{
	m_VBO = 0;
	m_EBO = 0;
	m_VAO = 0;
	m_NumberOfVertices = 0;
	m_NumberOfIndices = 0;
}

Mesh::~Mesh()
{
	destroy();
}

void Mesh::init()
{
	glGenVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);

	glGenBuffers(1, &m_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glGenBuffers(1, &m_EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
}

void Mesh::copyBufferData(Vertex * pVerts, unsigned int numberOfVerts, unsigned int * pIndices, unsigned int numberOfIndices)
{
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, numberOfVerts * sizeof(Vertex), pVerts, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, numberOfIndices * sizeof(unsigned int), pIndices, GL_STATIC_DRAW);

	m_NumberOfIndices = numberOfIndices;
	m_NumberOfVertices = numberOfVerts;
	glBindVertexArray(m_VAO);
	// 1rst attribute buffer : vertices

	//Assigning attribute arrays with a pointer
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(7 * sizeof(float)));

	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(9 * sizeof(float)));

	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(12 * sizeof(float)));

	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(15 * sizeof(float)));
}

void Mesh::render()
{
	//Binding buffer arrays
	glBindVertexArray(m_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);

	//drawing elements
	glDrawElements(GL_TRIANGLES, m_NumberOfIndices, GL_UNSIGNED_INT, (void*)0);

}

void Mesh::destroy()
{
	//Destroying arrays/buffers
	glDeleteVertexArrays(1, &m_VAO);
	glDeleteBuffers(1, &m_VBO);
	glDeleteBuffers(1, &m_EBO);
}

MeshCollection::MeshCollection()
{
}

MeshCollection::~MeshCollection()
{
	destroy();
}

void MeshCollection::addMesh(Mesh * pMesh)
{
	m_Meshes.push_back(pMesh);
}

void MeshCollection::render()
{
	for (Mesh *pMesh : m_Meshes)
	{
		pMesh->render();
	}
}

void MeshCollection::destroy()
{
	auto iter = m_Meshes.begin();
	while (iter != m_Meshes.end())
	{
		if (*iter)
		{
			(*iter)->destroy();
			delete (*iter);
			(*iter) = nullptr;
			iter = m_Meshes.erase(iter);
		}
		else
		{
			iter++;
		}
	}

	m_Meshes.clear();
}