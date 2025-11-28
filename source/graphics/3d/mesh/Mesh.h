#pragma once

#include <bx/bounds.h>
#include <bgfx/bgfx.h>
#include <bx/filepath.h>
#include <bx/readerwriter.h>
#include <bx/allocator.h>
#include <tinystl/allocator.h>
#include <tinystl/vector.h>
#include <tinystl/string.h>
#include "Camera.h"

namespace stl = tinystl;

struct Primitive
{
	uint32_t m_startIndex;
	uint32_t m_numIndices;
	uint32_t m_startVertex;
	uint32_t m_numVertices;

	bx::Sphere m_sphere;
	bx::Aabb   m_aabb;
	bx::Obb    m_obb;
};

typedef stl::vector<Primitive> PrimitiveArray;

struct Group
{
	Group();
	void reset();

	bgfx::VertexBufferHandle m_vbh;
	bgfx::IndexBufferHandle m_ibh;
	uint16_t m_numVertices;
	uint8_t* m_vertices;
	uint32_t m_numIndices;
	uint16_t* m_indices;
	bx::Sphere m_sphere;
	bx::Aabb   m_aabb;
	bx::Obb    m_obb;
	PrimitiveArray m_prims;
};

typedef stl::vector<Group> GroupArray;

class Mesh {
    public:
    Mesh();
    ~Mesh();
    void load(const bx::FilePath& _filePath, bool _ramcopy);
    void load(bx::ReaderSeekerI* _reader, bool _ramcopy = false);
    void unload();
    void submit(bgfx::ViewId _id, bgfx::ProgramHandle _program, const float* _mtx, uint64_t _state) const;
	void update(float _dt);
private:
    GroupArray m_groups;
    bgfx::VertexLayout m_layout;
    void* m_vertexData = nullptr;
    uint32_t m_vertexCount = 0;
	Camera m_camera;
};