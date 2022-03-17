#include <BindInstanceGFX.hpp>
#include <InstanceManager.hpp>
#include <CRSMath.hpp>

BindInstanceGFX::BindInstanceGFX() noexcept : m_vertexLayoutAvailable(false) {}

BindInstanceGFX::BindInstanceGFX(
	std::unique_ptr<IPipelineObject> pso,
	std::unique_ptr<IRootSignature> signature
) noexcept
	: m_pso(std::move(pso)),
	m_rootSignature(std::move(signature)), m_vertexLayoutAvailable(false) {}

void BindInstanceGFX::AddPSO(std::unique_ptr<IPipelineObject> pso) noexcept {
	m_pso = std::move(pso);
}

void BindInstanceGFX::AddRootSignature(std::unique_ptr<IRootSignature> signature) noexcept {
	m_rootSignature = std::move(signature);
}

void BindInstanceGFX::AddModel(
	const IModel* const modelRef
) noexcept {
	size_t indexBufferSize = modelRef->GetIndexBufferSize();
	size_t vertexBufferSize = modelRef->GetVertexBufferSize();

	m_modelsRaw.emplace_back(
		std::make_unique<ModelRaw>(
			modelRef,
			D3D12_VERTEX_BUFFER_VIEW{
				0u,
				static_cast<UINT>(vertexBufferSize),
				static_cast<UINT>(modelRef->GetVertexStrideSize())
			},
			VertexBufferInst::GetRef()->AddDataAndGetSharedAddress(
				modelRef->GetVertexData(), vertexBufferSize
			),
			D3D12_INDEX_BUFFER_VIEW{
				0u,
				static_cast<UINT>(indexBufferSize),
				DXGI_FORMAT_R16_UINT
			},
			IndexBufferInst::GetRef()->AddDataAndGetSharedAddress(
				modelRef->GetIndexData(), indexBufferSize
			),
			modelRef->GetIndexCount()
			)
	);

	if (!m_vertexLayoutAvailable) {
		m_vertexLayout.InitLayout(modelRef->GetVertexLayout());
		m_vertexLayoutAvailable = true;
	}
}

void BindInstanceGFX::BindModels(ID3D12GraphicsCommandList* commandList) noexcept {
	for (auto& model : m_modelsRaw)
		model->Draw(commandList);
}

void BindInstanceGFX::BindPipelineObjects(ID3D12GraphicsCommandList* commandList) noexcept {
	commandList->SetPipelineState(m_pso->Get());
	commandList->SetGraphicsRootSignature(m_rootSignature->Get());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void BindInstanceGFX::SetGPUVirtualAddresses() noexcept {
	for (auto& model : m_modelsRaw)
		model->UpdateGPUAddressOffsets();
}

VertexLayout BindInstanceGFX::GetVertexLayout() const noexcept {
	return m_vertexLayout;
}

// Model Raw
BindInstanceGFX::ModelRaw::ModelRaw(const IModel* const modelRef) noexcept
	:
	m_modelRef(modelRef),
	m_indexCount(0u),
	m_indexBufferView{}, m_vertexBufferView{} {}

BindInstanceGFX::ModelRaw::ModelRaw(
	const IModel* const modelRef,
	D3D12_VERTEX_BUFFER_VIEW&& vertexBufferView,
	D3DGPUSharedAddress vbvSharedAddress,
	D3D12_INDEX_BUFFER_VIEW&& indexBufferView,
	D3DGPUSharedAddress ibvSharedAddress,
	size_t indexCount
) noexcept
	:
	m_modelRef(modelRef),
	m_vertexBufferView(std::move(vertexBufferView), vbvSharedAddress),
	m_indexBufferView(std::move(indexBufferView), ibvSharedAddress),
	m_indexCount(static_cast<UINT>(indexCount)) {}

void BindInstanceGFX::ModelRaw::AddVBV(
	D3D12_VERTEX_BUFFER_VIEW&& vertexBufferView,
	D3DGPUSharedAddress vbvSharedAddress
) noexcept {
	m_vertexBufferView = { std::move(vertexBufferView), vbvSharedAddress };
}

void BindInstanceGFX::ModelRaw::AddIBV(
	D3D12_INDEX_BUFFER_VIEW&& indexBufferView,
	D3DGPUSharedAddress ibvSharedAddress,
	size_t indexCount
) noexcept {
	m_indexBufferView = { std::move(indexBufferView), ibvSharedAddress };
	m_indexCount = static_cast<UINT>(indexCount);
}

void BindInstanceGFX::ModelRaw::UpdateGPUAddressOffsets() {
	m_indexBufferView.SetGPUAddress();
	m_vertexBufferView.SetGPUAddress();
}

void BindInstanceGFX::ModelRaw::Draw(ID3D12GraphicsCommandList* commandList) noexcept {
	commandList->IASetVertexBuffers(0u, 1u, m_vertexBufferView.GetAddress());
	commandList->IASetIndexBuffer(m_indexBufferView.GetAddress());
	commandList->SetGraphicsRoot32BitConstant(0u, m_modelRef->GetTextureIndex(), 0u);

	commandList->DrawIndexedInstanced(m_indexCount, 1u, 0u, 0u, 0u);
}
