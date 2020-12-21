#include <utility>
#include "vulkan/vulkan.h"

// TODO: explore other vulkan headers

enum class DType { f32_1, f32_2, f32_3, i32_3, u32_3, Uknown };

constexpr VkFormat vk_dtype(DType type) {
  // float (vec1): VK_FORMAT_R32_SFLOAT
  // vec2: VK_FORMAT_R32G32_SFLOAT
  // vec3: VK_FORMAT_R32G32B32_SFLOAT
  // vec4: VK_FORMAT_R32G32B32A32_SFLOAT

  switch (type) {
    case DType::f32_1:
      return VK_FORMAT_R32_SFLOAT;
    case DType::f32_2:
      return VK_FORMAT_R32G32_SFLOAT;
    case DType::f32_3:
      return VK_FORMAT_R32G32B32_SFLOAT;
    case DType::i32_3:
      return VK_FORMAT_R32G32B32_SINT;
    case DType::u32_3:
      return VK_FORMAT_R32G32B32_UINT;
    default:
      return static_cast<VkFormat>(-1);
  }
}

constexpr size_t dtype_size(DType type) {
  switch (type) {
    case DType::f32_1:
      return sizeof(float);
    case DType::f32_2:
      return sizeof(float) * 2;
    case DType::f32_3:
      return sizeof(float) * 3;
    case DType::i32_3:
      return sizeof(int32_t) * 3;
    case DType::u32_3:
      return sizeof(uint32_t) * 3;
    default:
      return 0;
  }
}

constexpr size_t location_increment(DType type) {
  switch (type) {
    case DType::f32_1:
      return 1;
    case DType::f32_2:
      return 1;
    case DType::f32_3:
      return 1;
    case DType::i32_3:
      return 1;
    case DType::u32_3:
      return 1;
    default:
      return 0;
  }
  // not valid for matrices, double (f64) and others.
}

constexpr size_t recursive_fill_desc(
    stx::Span<VkVertexInputAttributeDescription> const&, uint32_t, uint32_t,
    uint32_t, uint32_t) {
  return 0;
}

template <typename Head_DType, typename... Tail_DTypes>
constexpr size_t recursive_fill_desc(
    stx::Span<VkVertexInputAttributeDescription> const& attribute_descriptions,
    uint32_t binding, uint32_t location, uint32_t offset, uint32_t index,
    Head_DType type, Tail_DTypes... tail_types) {
  attribute_descriptions[index].binding = binding;
  attribute_descriptions[index].location = location;
  attribute_descriptions[index].format = vk_dtype(type);
  attribute_descriptions[index].offset = offset;

  return dtype_size(type) +
         recursive_fill_desc(attribute_descriptions, binding,
                             location + location_increment(type),
                             offset + dtype_size(type), index + 1,
                             tail_types...);
}

template <typename T>
struct Desc {
  T desc;
  size_t total_size;
};

template <typename... DType>
constexpr Desc<std::array<VkVertexInputAttributeDescription, sizeof...(DType)>>
make_vertex_description(uint32_t binding, DType... types) {
  std::array<VkVertexInputAttributeDescription, sizeof...(DType)>
      descriptions{};

  auto size = recursive_fill_desc(descriptions, binding, 0, 0, 0, types...);
  return {descriptions, size};
}

#define VLK_VERTEX_STRUCT struct __attribute__((packed))

VLK_VERTEX_STRUCT Vertex {
  float position[2];
  float color[3];

  static constexpr VkVertexInputBindingDescription
  make_input_binding_description() {
    VkVertexInputBindingDescription binding_desciption{};

    binding_desciption.binding = 0;

    // VK_VERTEX_INPUT_RATE_VERTEX: Move to the next data entry after each
    // vertex (per-vertex data)
    // VK_VERTEX_INPUT_RATE_INSTANCE: Move to the next data entry after each
    // instance (per-instance data)
    binding_desciption.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    binding_desciption.stride = sizeof(Vertex);

    return binding_desciption;
  }

  static constexpr auto make_attributes_descriptions() {
    constexpr auto desc =
        make_vertex_description(0, DType::f32_2, DType::f32_3);
    static_assert(desc.total_size == sizeof(Vertex));

    return desc.desc;
  }
};