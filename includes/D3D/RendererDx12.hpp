#ifndef RENDERER_DX12_HPP_
#define RENDERER_DX12_HPP_
#include <Renderer.hpp>
#include <string>
#include <ObjectManager.hpp>

class RendererDx12 final : public Renderer {
public:
	RendererDx12(
		const char* appName,
		void* windowHandle, std::uint32_t width, std::uint32_t height,
		std::uint32_t bufferCount, RenderEngineType engineType
	);

	void Resize(std::uint32_t width, std::uint32_t height) override;

	[[nodiscard]]
	Resolution GetFirstDisplayCoordinates() const override;

	void SetThreadPool(std::shared_ptr<IThreadPool> threadPoolArg) noexcept override;
	void SetBackgroundColour(const std::array<float, 4>& colour) noexcept override;
	void SetShaderPath(const wchar_t* path) noexcept override;
	void SetSharedDataContainer(
		std::shared_ptr<ISharedDataContainer> sharedData
	) noexcept override;

	[[nodiscard]]
	size_t AddTexture(
		std::unique_ptr<std::uint8_t> textureData, size_t width, size_t height
	) override;

	void AddModelSet(
		std::vector<std::shared_ptr<IModel>>&& models, const std::wstring& pixelShader
	) override;
	void AddMeshletModelSet(
		std::vector<MeshletModel>&& meshletModels, const std::wstring& pixelShader
	) override;
	void AddModelInputs(
		std::vector<Vertex>&& gVertices, std::vector<std::uint32_t>&& gIndices
	) override;
	void AddModelInputs(
		std::vector<Vertex>&& gVertices, std::vector<std::uint32_t>&& gVerticesIndices,
		std::vector<std::uint32_t>&& gPrimIndices
	) override;

	void Update() override;
	void Render() override;
	void WaitForAsyncTasks() override;
	void ProcessData() override;

private:
	const std::string m_appName;
	std::uint32_t m_width;
	std::uint32_t m_height;
	std::uint32_t m_bufferCount;
	ObjectManager m_objectManager;
};
#endif
