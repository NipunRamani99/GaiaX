#include <ModelContainer.hpp>
#include <BindInstanceGFX.hpp>
#include <RootSignatureDynamic.hpp>
#include <Shader.hpp>
#include <PipelineObjectGFX.hpp>
#include <VertexLayout.hpp>

ModelContainer::ModelContainer(const char* shaderPath) noexcept
	: m_coloredInstanceData{}, m_texturedInstanceData{}, m_shaderPath(shaderPath) {}

void ModelContainer::AddColoredModel(ID3D12Device* device, std::unique_ptr<IModel> model) {
	if (!m_coloredInstanceData.available) {
		InitNewInstance(m_coloredInstanceData);

		if (!m_pRootSignature)
			CreateRootSignature(device);

		m_bindInstances[m_coloredInstanceData.index]->AddRootSignature(m_pRootSignature);

		VertexLayout layout = VertexLayout(model->GetVertexLayout());

		std::shared_ptr<Shader> vs = std::make_shared<Shader>();
		vs->LoadBinary(m_shaderPath + "VSColored.cso");

		std::shared_ptr<Shader> ps = std::make_shared<Shader>();
		ps->LoadBinary(m_shaderPath + "PSColored.cso");

		std::unique_ptr<PipelineObjectGFX> pso = std::make_unique<PipelineObjectGFX>();
		pso->CreatePipelineState(
			device,
			layout,
			m_pRootSignature->Get(),
			vs,
			ps
		);

		m_bindInstances[m_coloredInstanceData.index]->AddPSO(std::move(pso));
	}

	m_bindInstances[m_coloredInstanceData.index]->AddColoredModel(device, std::move(model));
}

void ModelContainer::AddTexturedModel(ID3D12Device* device, std::unique_ptr<IModel> model) {
	if (!m_texturedInstanceData.available) {
		InitNewInstance(m_texturedInstanceData);

		if (!m_pRootSignature)
			CreateRootSignature(device);

		m_bindInstances[m_texturedInstanceData.index]->AddRootSignature(m_pRootSignature);

		// Init Pipeline Object
	}

	m_bindInstances[m_texturedInstanceData.index]->AddTexturedModel(device, std::move(model));
}

void ModelContainer::BindCommands(ID3D12GraphicsCommandList* commandList) noexcept {
	for (auto& bindInstance : m_bindInstances)
		bindInstance->BindCommands(commandList);
}

void ModelContainer::InitNewInstance(InstanceData& instanceData) noexcept {
	m_bindInstances.emplace_back(std::make_unique<BindInstanceGFX>());
	instanceData = { true, static_cast<std::uint32_t>(m_bindInstances.size()) - 1u };
}

void ModelContainer::CreateRootSignature(ID3D12Device* device) {
	std::shared_ptr<RootSignatureDynamic> signature = std::make_shared<RootSignatureDynamic>();
	signature->CompileSignature(false);
	signature->CreateSignature(device);

	m_pRootSignature = signature;
}
