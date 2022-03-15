#include <GraphicsEngineDx12.hpp>
#include <D3DThrowMacros.hpp>
#include <InstanceManager.hpp>

GraphicsEngineDx12::GraphicsEngineDx12(
	const char* appName,
	void* windowHandle, std::uint32_t width, std::uint32_t height,
	size_t bufferCount
) : m_backgroundColour{0.1f, 0.1f, 0.1f, 0.1f}, m_appName(appName) {
	DeviceInst::Init();

#ifdef _DEBUG
	DebugInfoInst::Init();
#endif

	ID3D12Device5* deviceRef = DeviceInst::GetRef()->GetDeviceRef();

	DepthBuffInst::Init(
		deviceRef
	);
	DepthBuffInst::GetRef()->CreateDepthBuffer(
		deviceRef, width, height
	);

	GfxQueInst::Init(
		deviceRef,
		bufferCount
	);
	GfxCmdListInst::Init(
		deviceRef,
		bufferCount
	);

	IGraphicsQueueManager* queueRef = GfxQueInst::GetRef();

	SwapchainInst::Init(
		deviceRef,
		DeviceInst::GetRef()->GetFactoryRef(),
		queueRef->GetQueueRef(),
		windowHandle,
		bufferCount, width, height
	);

	queueRef->InitSyncObjects(
		deviceRef,
		SwapchainInst::GetRef()->GetCurrentBackBufferIndex()
	);

	ViewPAndScsrInst::Init(width, height);

	CpyQueInst::Init(deviceRef);
	CpyQueInst::GetRef()->InitSyncObjects(deviceRef);

	CpyCmdListInst::Init(deviceRef);
	HeapManagerInst::Init();
	VertexBufferInst::Init();
	IndexBufferInst::Init();
	DescTableManInst::Init();
	TexStorInst::Init();
}

GraphicsEngineDx12::~GraphicsEngineDx12() noexcept {
	TexStorInst::CleanUp();
	DescTableManInst::CleanUp();
	ViewPAndScsrInst::CleanUp();
	CpyCmdListInst::CleanUp();
	CpyQueInst::CleanUp();
	ModelContainerInst::CleanUp();
	VertexBufferInst::CleanUp();
	IndexBufferInst::CleanUp();
	SwapchainInst::CleanUp();
	GfxCmdListInst::CleanUp();
	GfxQueInst::CleanUp();
	DepthBuffInst::CleanUp();
	HeapManagerInst::CleanUp();
	DeviceInst::CleanUp();
#ifdef _DEBUG
	DebugInfoInst::CleanUp();
#endif
}

void GraphicsEngineDx12::SubmitModel(const IModel* const modelRef) {
	ModelContainerInst::GetRef()->AddModel(
		modelRef
	);
}

void GraphicsEngineDx12::Render() {
	IGraphicsQueueManager* queueRef = GfxQueInst::GetRef();
	ISwapChainManager* swapRef = SwapchainInst::GetRef ();
	ICommandListManager* listManRef = GfxCmdListInst::GetRef();
	ID3D12GraphicsCommandList* commandList = listManRef->GetCommandListRef();

	listManRef->Reset(
		swapRef->GetCurrentBackBufferIndex()
	);
	D3D12_RESOURCE_BARRIER renderBarrier = swapRef->GetRenderStateBarrier();
	commandList->ResourceBarrier(1u, &renderBarrier);

	IViewportAndScissorManager* viewportRef = ViewPAndScsrInst::GetRef();

	ID3D12DescriptorHeap* ppHeap[] = { DescTableManInst::GetRef()->GetDescHeapRef() };
	commandList->SetDescriptorHeaps(1u, ppHeap);

	commandList->RSSetViewports(1u, viewportRef->GetViewportRef());
	commandList->RSSetScissorRects(1u, viewportRef->GetScissorRef());

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = swapRef->GetRTVHandle();

	swapRef->ClearRTV(
		commandList, &m_backgroundColour.x, rtvHandle
	);

	IDepthBuffer* depthRef = DepthBuffInst::GetRef();

	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = depthRef->GetDSVHandle();

	depthRef->ClearDSV(commandList, dsvHandle);

	commandList->OMSetRenderTargets(1u, &rtvHandle, FALSE, &dsvHandle);

	// Record objects
	ModelContainerInst::GetRef()->BindCommands(commandList);

	D3D12_RESOURCE_BARRIER presentBarrier = swapRef->GetPresentStateBarrier();
	commandList->ResourceBarrier(1u, &presentBarrier);

	listManRef->Close();
	queueRef->ExecuteCommandLists(commandList);

	size_t backBufferIndex = swapRef->GetCurrentBackBufferIndex();

	swapRef->PresentWithTear();
	queueRef->MoveToNextFrame(backBufferIndex);
}

void GraphicsEngineDx12::Resize(std::uint32_t width, std::uint32_t height) {
	ISwapChainManager* swapRef = SwapchainInst::GetRef();
	IGraphicsQueueManager* pGraphicsQueue = GfxQueInst::GetRef();
	ID3D12Device* deviceRef = DeviceInst::GetRef()->GetDeviceRef();

	size_t backBufferIndex = swapRef->GetCurrentBackBufferIndex();

	pGraphicsQueue->WaitForGPU(
		backBufferIndex
	);

	if (swapRef->Resize(deviceRef, width, height)) {
		pGraphicsQueue->ResetFenceValuesWith(
			backBufferIndex
		);

		DepthBuffInst::GetRef()->CreateDepthBuffer(
			deviceRef, width, height
		);
		ViewPAndScsrInst::GetRef()->Resize(width, height);
	}
}

void GraphicsEngineDx12::GetMonitorCoordinates(
	std::uint64_t& monitorWidth, std::uint64_t& monitorHeight
) {
	ComPtr<IDXGIOutput> pOutput;
	HRESULT hr;
	D3D_THROW_FAILED(hr,
		SwapchainInst::GetRef()->GetRef()->GetContainingOutput(&pOutput)
	);

	DXGI_OUTPUT_DESC desc;
	D3D_THROW_FAILED(hr, pOutput->GetDesc(&desc));

	monitorWidth = static_cast<std::uint64_t>(desc.DesktopCoordinates.right);
	monitorHeight = static_cast<std::uint64_t>(desc.DesktopCoordinates.bottom);
}

void GraphicsEngineDx12::SetBackgroundColour(const Ceres::Float32_4& colour) noexcept {
	m_backgroundColour = colour;
}

void GraphicsEngineDx12::WaitForAsyncTasks() {
	GfxQueInst::GetRef()->WaitForGPU(
		SwapchainInst::GetRef()->GetCurrentBackBufferIndex()
	);
	CpyQueInst::GetRef()->WaitForGPU();
}

void GraphicsEngineDx12::SetShaderPath(const char* path) noexcept {
	m_shaderPath = path;
}

void GraphicsEngineDx12::InitResourceBasedObjects() {
	ModelContainerInst::Init(m_shaderPath.c_str());
}

void GraphicsEngineDx12::ProcessData() {
	ID3D12Device* device = DeviceInst::GetRef()->GetDeviceRef();
	IModelContainer* modelContainerRef = ModelContainerInst::GetRef();
	IDescriptorTableManager* descTableRef = DescTableManInst::GetRef();
	ITextureStorage* texStoreRef = TexStorInst::GetRef();

	descTableRef->CreateDescriptorTable(device);

	modelContainerRef->CreateBuffers(device);

	std::atomic_size_t workCount = 0u;

	modelContainerRef->CopyData(workCount);
	texStoreRef->CopyData(workCount);

	while (workCount != 0u);

	texStoreRef->CreateBufferViews(device);

	descTableRef->CopyUploadHeap(device);

	ICommandListManager* copyListManager = CpyCmdListInst::GetRef();
	copyListManager->Reset(0u);
	ID3D12GraphicsCommandList* copyList = copyListManager->GetCommandListRef();

	modelContainerRef->RecordUploadBuffers(copyList);

	copyListManager->Close();

	ICopyQueueManager* copyQue = CpyQueInst::GetRef();
	copyQue->ExecuteCommandLists(copyList);
	copyQue->WaitForGPU();

	modelContainerRef->InitPipelines(device);

	texStoreRef->ReleaseUploadBuffer();
	descTableRef->ReleaseUploadHeap();
	modelContainerRef->ReleaseUploadBuffers();
}

size_t GraphicsEngineDx12::RegisterResource(
	const void* data, size_t rowPitch, size_t rows
) {
	return TexStorInst::GetRef()->AddTexture(
		DeviceInst::GetRef()->GetDeviceRef(), data, rowPitch, rows
	);
}
