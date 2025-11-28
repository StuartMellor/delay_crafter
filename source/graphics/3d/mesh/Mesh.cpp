#include "Mesh.h"
#include <string>
#include <bx/readerwriter.h>
#include <bx/file.h>
#include <bx/error.h>
#include <meshoptimizer.h>

// Group implementation
Group::Group()
	: m_vbh(BGFX_INVALID_HANDLE)
	, m_ibh(BGFX_INVALID_HANDLE)
	, m_numVertices(0)
	, m_vertices(nullptr)
	, m_numIndices(0)
	, m_indices(nullptr)
{
	// spheres/aabb/obb left default constructed
}

void Group::reset()
{
	m_vbh = BGFX_INVALID_HANDLE;
	m_ibh = BGFX_INVALID_HANDLE;
	m_numVertices = 0;
	m_vertices = nullptr;
	m_numIndices = 0;
	m_indices = nullptr;
	m_prims.clear();
}

Mesh::~Mesh() = default;
Mesh::Mesh() : m_camera()
{
}

void Mesh::submit(bgfx::ViewId _id, bgfx::ProgramHandle _program, const float* _mtx, uint64_t _state) const
{
    for (const Group& group : m_groups)
    {
        bgfx::setVertexBuffer(0, group.m_vbh);
        bgfx::setTransform(_mtx);
        bgfx::setState(_state);
        bgfx::submit(_id, _program);
    }
}

 void Mesh::load(const bx::FilePath& _filePath, bool _ramcopy)
{
	// Minimal file loading using bx::FileReader; ignore errors silently if file can't open.
	bx::FileReader reader;
	bx::Error err;
	if (reader.open(_filePath, &err))
	{
		load(&reader, _ramcopy);
		reader.close();
	}
}

void Mesh::update (float _dt)
{
	// use the camera's view projection matrix
	
}

void Mesh::load(bx::ReaderSeekerI* _reader, bool _ramcopy)
{
    constexpr uint32_t kChunkVertexBuffer           = BX_MAKEFOURCC('V', 'B', ' ', 0x1);
	constexpr uint32_t kChunkVertexBufferCompressed = BX_MAKEFOURCC('V', 'B', 'C', 0x0);
	constexpr uint32_t kChunkIndexBuffer            = BX_MAKEFOURCC('I', 'B', ' ', 0x0);
	constexpr uint32_t kChunkIndexBufferCompressed  = BX_MAKEFOURCC('I', 'B', 'C', 0x1);
	constexpr uint32_t kChunkPrimitive              = BX_MAKEFOURCC('P', 'R', 'I', 0x0);

	using namespace bx;
	using namespace bgfx;

	Group group;

	static bx::DefaultAllocator s_allocator;
	bx::AllocatorI* allocator = &s_allocator;

	uint32_t chunk;
	bx::Error err;
	while (4 == bx::read(_reader, chunk, &err)
	   &&  err.isOk() )
	{
		switch (chunk)
		{
			case kChunkVertexBuffer:
			{
				read(_reader, group.m_sphere, &err);
				read(_reader, group.m_aabb, &err);
				read(_reader, group.m_obb, &err);

				read(_reader, m_layout, &err);

				uint16_t stride = m_layout.getStride();

				read(_reader, group.m_numVertices, &err);
				const bgfx::Memory* mem = bgfx::alloc(group.m_numVertices*stride);
				read(_reader, mem->data, mem->size, &err);

				if (_ramcopy)
				{
					group.m_vertices = (uint8_t*)bx::alloc(allocator, group.m_numVertices*stride);
					bx::memCopy(group.m_vertices, mem->data, mem->size);
				}

				group.m_vbh = bgfx::createVertexBuffer(mem, m_layout);
			}
				break;

			case kChunkVertexBufferCompressed:
			{
				read(_reader, group.m_sphere, &err);
				read(_reader, group.m_aabb, &err);
				read(_reader, group.m_obb, &err);

				read(_reader, m_layout, &err);

				uint16_t stride = m_layout.getStride();

				read(_reader, group.m_numVertices, &err);

				const bgfx::Memory* mem = bgfx::alloc(group.m_numVertices*stride);

				uint32_t compressedSize;
				bx::read(_reader, compressedSize, &err);

				void* compressedVertices = bx::alloc(allocator, compressedSize);
				bx::read(_reader, compressedVertices, compressedSize, &err);

				meshopt_decodeVertexBuffer(mem->data, group.m_numVertices, stride, (uint8_t*)compressedVertices, compressedSize);

				bx::free(allocator, compressedVertices);

				if (_ramcopy)
				{
					group.m_vertices = (uint8_t*)bx::alloc(allocator, group.m_numVertices*stride);
					bx::memCopy(group.m_vertices, mem->data, mem->size);
				}

				group.m_vbh = bgfx::createVertexBuffer(mem, m_layout);
			}
				break;

			case kChunkIndexBuffer:
			{
				read(_reader, group.m_numIndices, &err);

				const bgfx::Memory* mem = bgfx::alloc(group.m_numIndices*2);
				read(_reader, mem->data, mem->size, &err);

				if (_ramcopy)
				{
					group.m_indices = (uint16_t*)bx::alloc(allocator, group.m_numIndices*2);
					bx::memCopy(group.m_indices, mem->data, mem->size);
				}

				group.m_ibh = bgfx::createIndexBuffer(mem);
			}
				break;

			case kChunkIndexBufferCompressed:
			{
				bx::read(_reader, group.m_numIndices, &err);

				const bgfx::Memory* mem = bgfx::alloc(group.m_numIndices*2);

				uint32_t compressedSize;
				bx::read(_reader, compressedSize, &err);

				void* compressedIndices = bx::alloc(allocator, compressedSize);

				bx::read(_reader, compressedIndices, compressedSize, &err);

				meshopt_decodeIndexBuffer(mem->data, group.m_numIndices, 2, (uint8_t*)compressedIndices, compressedSize);

				bx::free(allocator, compressedIndices);

				if (_ramcopy)
				{
					group.m_indices = (uint16_t*)bx::alloc(allocator, group.m_numIndices*2);
					bx::memCopy(group.m_indices, mem->data, mem->size);
				}

				group.m_ibh = bgfx::createIndexBuffer(mem);
			}
				break;

			case kChunkPrimitive:
			{
				uint16_t len;
				read(_reader, len, &err);

				stl::string material;
				material.resize(len);
				read(_reader, const_cast<char*>(material.c_str() ), len, &err);

				uint16_t num;
				read(_reader, num, &err);

				for (uint32_t ii = 0; ii < num; ++ii)
				{
					read(_reader, len, &err);

					stl::string name;
					name.resize(len);
					read(_reader, const_cast<char*>(name.c_str() ), len, &err);

					Primitive prim;
					read(_reader, prim.m_startIndex, &err);
					read(_reader, prim.m_numIndices, &err);
					read(_reader, prim.m_startVertex, &err);
					read(_reader, prim.m_numVertices, &err);
					read(_reader, prim.m_sphere, &err);
					read(_reader, prim.m_aabb, &err);
					read(_reader, prim.m_obb, &err);

					group.m_prims.push_back(prim);
				}

				m_groups.push_back(group);
				group.reset();
			}
				break;

			default:
				// Unknown chunk; skip remains at current position.
				break;
		}
	}
}

void Mesh::unload()
{
	static bx::DefaultAllocator s_allocator;
	bx::AllocatorI* allocator = &s_allocator;

    for (const Group& group : m_groups)
    {
        bgfx::destroy(group.m_vbh);

        if (bgfx::isValid(group.m_ibh))
        {
            bgfx::destroy(group.m_ibh);
        }

        if (group.m_vertices != nullptr)
        {
            bx::free(allocator, group.m_vertices);
        }

        if (group.m_indices != nullptr)
        {
            bx::free(allocator, group.m_indices);
        }
    }
    m_groups.clear();
}

