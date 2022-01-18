#ifndef __I_GRAPHICS_QUEUE_MANAGER_HPP__
#define __I_GRAPHICS_QUEUE_MANAGER_HPP__
#include <D3DHeaders.hpp>
#include <cstdint>

class IGraphicsQueueManager {
public:
	virtual ~IGraphicsQueueManager() = default;

	virtual void InitSyncObjects(ID3D12Device* device, size_t backBufferIndex) = 0;
	virtual void WaitForGPU(size_t backBufferIndex) = 0;
	virtual void ExecuteCommandLists(
		ID3D12GraphicsCommandList* commandList
	) const noexcept = 0;
	virtual void MoveToNextFrame(size_t backBufferIndex) = 0;
	virtual void ResetFenceValuesWith(size_t valueIndex) = 0;

	virtual ID3D12CommandQueue* GetQueueRef() const noexcept = 0;
};

IGraphicsQueueManager* CreateGraphicsQueueInstance(
	ID3D12Device* device,
	size_t bufferCount
);

#endif
