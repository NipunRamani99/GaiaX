#ifndef __I_RESOURCE_BUFFER_HPP__
#define __I_RESOURCE_BUFFER_HPP__
#include <cstdint>
#include <D3DHeaders.hpp>

enum class BufferType {
	Index,
	Vertex
};

class IResourceBuffer {
public:
	virtual ~IResourceBuffer() = default;

	virtual size_t AddData(const void* source, size_t bufferSize) = 0;
	virtual void CreateBuffer(ID3D12Device* device) = 0;
	virtual void CopyData() noexcept = 0;
	virtual void RecordUpload(ID3D12GraphicsCommandList* copyList) = 0;
	virtual void ReleaseUploadBuffer() = 0;
	virtual D3D12_GPU_VIRTUAL_ADDRESS GetGPUHandle() const noexcept = 0;
};

IResourceBuffer* CreateResourceBufferInstance(BufferType type);
#endif
