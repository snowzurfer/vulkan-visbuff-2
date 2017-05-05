#ifndef VKS_RENDERER
#define VKS_RENDERER

#include <EASTL/array.h>
#include <EASTL/string.h>
#include <EASTL/unique_ptr.h>
#include <EASTL/vector.h>
#include <framebuffer.h>
#define GLM_FORCE_CXX11
#include <glm/glm.hpp>
#include <light.h>
#include <material.h>
#include <renderpass.h>
#include <vulkan/vulkan.h>
#include <vulkan_buffer.h>
#include <vulkan_image.h>

namespace szt {
class Camera;
} // namespace szt

namespace vks {

class VulkanDevice;
class Model;
class ModelWithHeaps;
class VulkanTexture;
class VertexSetup;

extern const VkFormat kVisBarysBufferFormat;
extern const VkFormat kColourBufferFormat;
const uint32_t kFramesCaptureNum = 20U;
const uint32_t kCapturesNum = 10U;
// extern const uint32_t kVertexBuffersBaseBindPos;
// extern const uint32_t kIndirectDrawCmdsBindingPos;
// extern const uint32_t kIdxBufferBindPos;
// extern const uint32_t kModelMatxsBufferBindPos;
// extern const uint32_t kMaterialIDsBufferBindPos;

struct SetsEnum {
  enum Sets { VIS_GENERIC = 0U, SKYBOX, num_items }; // enum Sets
};                                                   // struct SetsEnum
typedef SetsEnum::Sets SetTypes;

struct DescSetLayoutsEnum {
  enum DescSetLayouts {
    VIS_GENERIC = 0U,
    HEAP,
    SKYBOX,
    num_items
  }; // enum DescSetLayouts
};   // struct DescSetLayoutsEnum
typedef DescSetLayoutsEnum::DescSetLayouts DescSetLayoutTypes;

class Renderer {
public:
  Renderer();

  void Init(szt::Camera *cam, const VertexSetup &vtx_setup);
  void Shutdown();
  void PreRender();
  void Render();
  void PostRender();

  void ReloadAllShaders();
  void CaptureBandwidthDataFromPositions(
      const eastl::array<glm::vec3, kCapturesNum> &sample_positions,
      const eastl::array<glm::vec3, kCapturesNum> &sample_directions) const;
  void CaptureBandwidthDataAtPosition() const;

  // Register a model for rendering.
  void RegisterModel(Model &model);

private:
  void SetupRenderPass(const VulkanDevice &device);
  void SetupFrameBuffers(const VulkanDevice &device);
  void SetupMaterials();
  void SetupMaterialPipelines(const VulkanDevice &device,
                              const VertexSetup &g_store_vertex_setup);
  void SetupUniformBuffers(const VulkanDevice &device);
  // Create both the desc set layouts and the pipe layouts
  void SetupDescriptorSetAndPipeLayout(const VulkanDevice &device);
  void SetupDescriptorPool(const VulkanDevice &device);
  void SetupDescriptorSets(const VulkanDevice &device);
  void SetupCommandBuffers();
  void SetupSamplers(const VulkanDevice &device);
  void UpdatePVMatrices();
  void UpdateBuffers(const VulkanDevice &device);
  void SetupFullscreenQuad(const VulkanDevice &device);
  void CreateCubeMesh(const VulkanDevice &device);
  void CreateFramebufferAttachment(const VulkanDevice &device, VkFormat format,
                                   VkImageUsageFlags img_usage_flags,
                                   const eastl::string &name,
                                   VulkanTexture **attachment) const;
  void UpdateLights(eastl::vector<Light> &transformed_lights);
  void OutputPerformanceDataToFile() const;
  void CreateFences(const VulkanDevice &device);
  void CaptureData();
  void CaptureScreenshot(const eastl::string &filename) const;
  void FinalInit(const VulkanDevice &device);

  eastl::unique_ptr<Renderpass> renderpass_;

  /**
   * @brief Have as many framebuffs as there are swapchain images
   */
  eastl::vector<eastl::unique_ptr<Framebuffer>> framebuffers_;
  uint32_t current_swapchain_img_;

  /**
   * @brief Have as many command buffers as there are swapchain images
   */
  eastl::vector<VkCommandBuffer> cmd_buffers_;

  VulkanTexture *vis_buffer_;
  VulkanTexture *derivs_and_barys_buffer_;
  VulkanTexture *accum_buffer_;
  VulkanTexture *depth_buffer_;
  VkImageView *depth_buffer_depth_view_;

  Material *vis_shade_material_;
  Material *vis_store_material_;
  Material *tonemap_material_;
  Material *skybox_material_;

  /**
   * @brief Texture used in replacement in materials which don't have a texture
   *        for a given type of map.
   */
  VulkanTexture *dummy_texture_;
  VulkanTexture *skybox_texture_;

  eastl::vector<VkDescriptorSetLayout> desc_set_layouts_;
  eastl::array<VkDescriptorSet, SetTypes::num_items> desc_sets_;
  VkDescriptorPool desc_pool_;

  struct PipeLayoutsEnum {
    enum PipeLayouts { VPASS = 0U, num_items }; // enum PipeLayouts
  };                                            // struct PipeLayoutsEnum
  typedef PipeLayoutsEnum::PipeLayouts PipeLayoutTypes;
  eastl::vector<VkPipelineLayout> pipe_layouts_;

  VulkanBuffer main_static_buff_;

  // These are contained in camera, but this way they can be easily used to
  // update the VulkanBuffers
  glm::mat4 proj_mat_;
  glm::mat4 view_mat_;
  glm::mat4 inv_proj_mat_;
  glm::mat4 inv_view_mat_;

  szt::Camera *cam_;

  VkSampler aniso_sampler_;
  VkSampler nearest_sampler_;
  VkSampler aniso_edge_sampler_;

  eastl::vector<Model *> registered_models_;
  Model *fullscreenquad_;
  Model *cube_;

  eastl::vector<MaterialConstants> mat_consts_;

  VkFence renderpasses_fence_;

  mutable uint32_t frames_captured_;
  mutable uint32_t num_captures_;
  mutable uint32_t num_captures_to_collect_;
  mutable bool capturing_from_positions_enabled_;
  mutable bool capturing_enabled_;
  struct FrameMemoryData {
    uint32_t first_frame;
    uint32_t second_frame;
  };
  mutable eastl::vector<eastl::array<FrameMemoryData, kFramesCaptureNum>>
      mem_perf_data_reads_;
  mutable eastl::vector<eastl::array<FrameMemoryData, kFramesCaptureNum>>
      mem_perf_data_writes_;
  mutable eastl::array<glm::vec3, kCapturesNum> camera_sample_positions_;
  mutable eastl::array<glm::vec3, kCapturesNum> camera_sample_directions_;
  mutable glm::mat4 last_cam_mat_;

  mutable bool capture_screenshot_;

  // Used to call the second part of the initialisation
  bool first_run_;

  VertexSetup vtx_setup_;

}; // class Renderer

} // namespace vks

#endif
