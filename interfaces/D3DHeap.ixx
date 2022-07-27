module;

#include <D3DHeaders.hpp>

export module D3DHeap;

export class D3DHeap {
public:
	D3DHeap(D3D12_HEAP_TYPE type) noexcept;

	void CreateHeap(ID3D12Device* device);

	[[nodiscard]]
	size_t ReserveSizeAndGetOffset(size_t heapSize, UINT64 alignment) noexcept;

	[[nodiscard]]
	ID3D12Heap* GetHeap() const noexcept;

private:
	D3D12_HEAP_TYPE m_heapType;
	UINT64 m_maxAlignment;
	UINT64 m_totalHeapSize;
	ComPtr<ID3D12Heap> m_pHeap;
};
