#include "ashura/rcg.h"

namespace ash
{
namespace rcg
{

constexpr u8  MAX_SCOPE_BARRIERS = 16;
constexpr u64 BATCH_SIZE         = 16;

// warnings: can't be used as depth stencil and color attachment
// load op clear op, read write matches imageusagescope
// ssbo matches scope
// push constant size match check
// NOTE: renderpass attachments MUST not be accessed in shaders within that renderpass
// NOTE:::: update_buffer and fill_buffer MUST be multiple of 4 for dst offset and dst size
struct BufferScope
{
  gfx::PipelineStages stages = gfx::PipelineStages::None;
  gfx::Access         access = gfx::Access::None;
};

struct ImageScope
{
  gfx::PipelineStages stages = gfx::PipelineStages::None;
  gfx::Access         access = gfx::Access::None;
  gfx::ImageLayout    layout = gfx::ImageLayout::Undefined;
};

inline void acquire_buffer(gfx::BufferUsageScope scope, gfx::PipelineStages dst_stages,
                           gfx::Access                                               dst_access,
                           stx::Array<gfx::BufferMemoryBarrier, MAX_SCOPE_BARRIERS> &barriers)
{
  if (has_bits(scope, gfx::BufferUsageScope::TransferSrc | gfx::BufferUsageScope::TransferDst))
  {
    barriers
        .push(gfx::BufferMemoryBarrier{.src_stages = gfx::PipelineStages::Transfer,
                                       .dst_stages = dst_stages,
                                       .src_access =
                                           gfx::Access::TransferRead | gfx::Access::TransferWrite,
                                       .dst_access = dst_access})
        .unwrap();
  }
  else if (has_bits(scope, gfx::BufferUsageScope::TransferSrc))
  {
    barriers
        .push(gfx::BufferMemoryBarrier{.src_stages = gfx::PipelineStages::Transfer,
                                       .dst_stages = dst_stages,
                                       .src_access = gfx::Access::TransferRead,
                                       .dst_access = dst_access})
        .unwrap();
  }
  else if (has_bits(scope, gfx::BufferUsageScope::TransferDst))
  {
    barriers
        .push(gfx::BufferMemoryBarrier{.src_stages = gfx::PipelineStages::Transfer,
                                       .dst_stages = dst_stages,
                                       .src_access = gfx::Access::TransferWrite,
                                       .dst_access = dst_access})
        .unwrap();
  }

  if (has_bits(scope, gfx::BufferUsageScope::IndirectCommand))
  {
    barriers
        .push(gfx::BufferMemoryBarrier{.src_stages = gfx::PipelineStages::DrawIndirect,
                                       .dst_stages = dst_stages,
                                       .src_access = gfx::Access::IndirectCommandRead,
                                       .dst_access = dst_access})
        .unwrap();
  }

  if (has_any_bit(scope, gfx::BufferUsageScope::ComputeShaderUniform |
                             gfx::BufferUsageScope::ComputeShaderUniformTexel))
  {
    barriers
        .push(gfx::BufferMemoryBarrier{.src_stages = gfx::PipelineStages::ComputeShader,
                                       .dst_stages = dst_stages,
                                       .src_access = gfx::Access::ShaderRead,
                                       .dst_access = dst_access})
        .unwrap();
  }

  if (has_any_bit(scope, gfx::BufferUsageScope::ComputeShaderStorage |
                             gfx::BufferUsageScope::ComputeShaderStorageTexel))
  {
    barriers
        .push(gfx::BufferMemoryBarrier{.src_stages = gfx::PipelineStages::ComputeShader,
                                       .dst_stages = dst_stages,
                                       .src_access = gfx::Access::ShaderWrite,
                                       .dst_access = dst_access})
        .unwrap();
  }

  if (has_bits(scope, gfx::BufferUsageScope::IndexBuffer))
  {
    barriers
        .push(gfx::BufferMemoryBarrier{.src_stages = gfx::PipelineStages::VertexInput,
                                       .dst_stages = dst_stages,
                                       .src_access = gfx::Access::IndexRead,
                                       .dst_access = dst_access})
        .unwrap();
  }

  if (has_bits(scope, gfx::BufferUsageScope::VertexBuffer))
  {
    barriers
        .push(gfx::BufferMemoryBarrier{.src_stages = gfx::PipelineStages::VertexInput,
                                       .dst_stages = dst_stages,
                                       .src_access = gfx::Access::VertexAttributeRead,
                                       .dst_access = dst_access})
        .unwrap();
  }

  if (has_bits(scope, gfx::BufferUsageScope::VertexShaderUniform |
                          gfx::BufferUsageScope::FragmentShaderUniform))
  {
    barriers
        .push(gfx::BufferMemoryBarrier{.src_stages = gfx::PipelineStages::VertexShader |
                                                     gfx::PipelineStages::FragmentShader,
                                       .dst_stages = dst_stages,
                                       .src_access = gfx::Access::ShaderRead,
                                       .dst_access = dst_access})
        .unwrap();
  }
  else if (has_bits(scope, gfx::BufferUsageScope::VertexShaderUniform))
  {
    barriers
        .push(gfx::BufferMemoryBarrier{.src_stages = gfx::PipelineStages::VertexShader,
                                       .dst_stages = dst_stages,
                                       .src_access = gfx::Access::ShaderRead,
                                       .dst_access = dst_access})
        .unwrap();
  }
  else if (has_bits(scope, gfx::BufferUsageScope::FragmentShaderUniform))
  {
    barriers
        .push(gfx::BufferMemoryBarrier{.src_stages = gfx::PipelineStages::FragmentShader,
                                       .dst_stages = dst_stages,
                                       .src_access = gfx::Access::ShaderRead,
                                       .dst_access = dst_access})
        .unwrap();
  }
}

// release side-effects to operations that are allowed to have concurrent non-mutating access on the resource
inline void release_buffer(gfx::BufferUsageScope scope, gfx::PipelineStages src_stages,
                           gfx::Access                                               src_access,
                           stx::Array<gfx::BufferMemoryBarrier, MAX_SCOPE_BARRIERS> &barriers)
{
  if (has_bits(scope, gfx::BufferUsageScope::TransferSrc))
  {
    barriers
        .push(gfx::BufferMemoryBarrier{.src_stages = src_stages,
                                       .dst_stages = gfx::PipelineStages::Transfer,
                                       .src_access = src_access,
                                       .dst_access = gfx::Access::TransferRead})
        .unwrap();
  }

  if (has_bits(scope, gfx::BufferUsageScope::IndirectCommand))
  {
    barriers
        .push(gfx::BufferMemoryBarrier{.src_stages = src_stages,
                                       .dst_stages = gfx::PipelineStages::DrawIndirect,
                                       .src_access = src_access,
                                       .dst_access = gfx::Access::IndirectCommandRead})
        .unwrap();
  }

  if (has_any_bit(scope, gfx::BufferUsageScope::ComputeShaderUniform |
                             gfx::BufferUsageScope::ComputeShaderUniformTexel))
  {
    barriers
        .push(gfx::BufferMemoryBarrier{.src_stages = src_stages,
                                       .dst_stages = gfx::PipelineStages::ComputeShader,
                                       .src_access = src_access,
                                       .dst_access = gfx::Access::ShaderRead})
        .unwrap();
  }

  if (has_bits(scope, gfx::BufferUsageScope::IndexBuffer))
  {
    barriers
        .push(gfx::BufferMemoryBarrier{.src_stages = src_stages,
                                       .dst_stages = gfx::PipelineStages::VertexInput,
                                       .src_access = src_access,
                                       .dst_access = gfx::Access::IndexRead})
        .unwrap();
  }

  if (has_bits(scope, gfx::BufferUsageScope::VertexBuffer))
  {
    barriers
        .push(gfx::BufferMemoryBarrier{.src_stages = src_stages,
                                       .dst_stages = gfx::PipelineStages::VertexInput,
                                       .src_access = src_access,
                                       .dst_access = gfx::Access::VertexAttributeRead})
        .unwrap();
  }

  if (has_bits(scope, gfx::BufferUsageScope::VertexShaderUniform |
                          gfx::BufferUsageScope::FragmentShaderUniform))
  {
    barriers
        .push(gfx::BufferMemoryBarrier{.src_stages = src_stages,
                                       .dst_stages = gfx::PipelineStages::VertexShader |
                                                     gfx::PipelineStages::FragmentShader,
                                       .src_access = src_access,
                                       .dst_access = gfx::Access::ShaderRead})
        .unwrap();
  }
  else if (has_bits(scope, gfx::BufferUsageScope::VertexShaderUniform))
  {
    barriers
        .push(gfx::BufferMemoryBarrier{.src_stages = src_stages,
                                       .dst_stages = gfx::PipelineStages::VertexShader,
                                       .src_access = src_access,
                                       .dst_access = gfx::Access::ShaderRead})
        .unwrap();
  }
  else if (has_bits(scope, gfx::BufferUsageScope::FragmentShaderUniform))
  {
    barriers
        .push(gfx::BufferMemoryBarrier{.src_stages = src_stages,
                                       .dst_stages = gfx::PipelineStages::FragmentShader,
                                       .src_access = src_access,
                                       .dst_access = gfx::Access::ShaderRead})
        .unwrap();
  }
}

constexpr BufferScope transfer_buffer_scope(gfx::BufferUsageScope scope)
{
  gfx::PipelineStages stages = gfx::PipelineStages::Transfer;
  gfx::Access         access = gfx::Access::None;

  if (has_bits(scope, gfx::BufferUsageScope::TransferSrc))
  {
    access |= gfx::Access::TransferRead;
  }

  if (has_bits(scope, gfx::BufferUsageScope::TransferDst))
  {
    access |= gfx::Access::TransferWrite;
  }

  return BufferScope{.stages = stages, .access = access};
}

constexpr BufferScope compute_storage_buffer_scope(gfx::BufferUsageScope scope)
{
  gfx::PipelineStages stages = gfx::PipelineStages::ComputeShader;
  gfx::Access         access = gfx::Access::None;

  if (has_bits(scope, gfx::BufferUsageScope::ComputeShaderStorage))
  {
    access |= gfx::Access::ShaderWrite;
  }

  if (has_bits(scope, gfx::BufferUsageScope::ComputeShaderStorageTexel))
  {
    access |= gfx::Access::ShaderWrite;
  }

  if (has_bits(scope, gfx::BufferUsageScope::ComputeShaderUniform))
  {
    access |= gfx::Access::ShaderRead;
  }

  if (has_bits(scope, gfx::BufferUsageScope::ComputeShaderUniformTexel))
  {
    access |= gfx::Access::ShaderRead;
  }

  return BufferScope{.stages = stages, .access = access};
}

// we need a single barrier for stages where the layout is different from the base layout (readonlyoptimal)
// i.e. storage, transfer
//
// only called on dst
// Q(lamarrr): by inserting multiple barriers, will they all execute???
// only one of them will ever get executed because the previous barriers would have drained the work they had
// only executes if there is still work left to be drained???? but there will be multiple works to be drained
// once a work completes and there are multiple barriers waiting on it, will all the barriers perform the wait
//, no because this will implicitly depend on any op that performs read, writes, or layout transitions
//
// Q(lamarrr): if there's no task that has the required access and stages, will the barrier still execute?
//
//
// images and buffers must have a first command that operate on them? to transition them and provide access to other commands?
//
// what if no previous operation meets the barrier's requirements? then it will continue executing the requesting command
//
// no previous cmd means contents are undefined
//
//
// and if image was never transitioned we should be good and use undefined src layout?
//
// pre -> op -> post
// if image was already on the queue scope takes care of it
// layout+scope -> transfer src|dst optimal
// all ops that have side-effects
// transfer transfer src|dst optimal -> scope + layout???? not needed?
//
// for transfer post-stages, we can omit the barriers, we only need to give barriers to readers
//
//
// Requirements:
// - layout merging:
//      - used as storage and sampled in the same command
//      - sampled and input attachment  in the same command
//      - transfer src and transfer dst in the same command
//
//
// TODO(lamarrr): undefined layout handling
//
// we need: initial layout, operations providing the initial layout and releasing it to other scopes for pickup
//
// - easiest and best would be to have an Undefined + Init function that specifies and places barriers for the scopes
//
// - they must all have an initial transfer-dst operation? -
// - acquire_init -> transfer -> release
//
// - acquire_init -> render as color attachment -> release
//
// we need a base layout

inline void release_image_init(gfx::ImageUsageScope first_use_scope, gfx::ImageUsageScope scope,
                               gfx::ImageMemoryBarrier &barrier)
{
  // just use fill??? even if not specified with initial data
  // Scope Layout -> Other Scopes layout
  // can't use fill cos we might not know the initial layout
  //
  // our barriers assume they are coming from one usagescope and transition to same or another usagescope
  // initialization is not representable
  //
  //
  //
  // release only upon init
  //
  // Undefined, None Access, TopOfPipe -> scopes, NoAccessMask will not affect accessmasks
  // - multiple setup will mean????
  //
  // release to transfer will be as transfer dst
  // release to color attachment will be as write-only non-flushed
  //
  //
  // multiple none-accesses will cause multiple transitions because there will be no dependency chains
  //
  // the previous acquire barriers only work on
  //
  //
  // initial layout top of pipe, none access, transition layout to initial layout
  //
  //
  // add is_first?
  //
  //
  if (has_any_bit(init_scope,
                  gfx::ImageUsageScope::TransferSrc | gfx::ImageUsageScope::TransferDst))
  {
    gfx::ImageLayout new_layout = gfx::ImageLayout::Undefined;
    if (has_bits(scope, gfx::ImageUsageScope::TransferSrc | gfx::ImageUsageScope::TransferDst))
    {
      new_layout = gfx::ImageLayout::General;
    }
    else if (has_bits(scope, gfx))
      else
      {
        new_layout = gfx::ImageLayout::TransferDstOptimal;
      }

    barrier = gfx::ImageMemoryBarrier{.src_stages = gfx::PipelineStages::TopOfPipe,
                                      .dst_stages = gfx::PipelineStages::Transfer,
                                      .src_access = gfx::Access::None,
                                      .dst_access = gfx::Access::TransferWrite,
                                      .old_layout = gfx::ImageLayout::Undefined,
                                      .new_layout = new_layout};
  }
  else if (init_scope == gfx::ImageUsageScope::ComputeShaderStorage)
  {
    barrier = gfx::ImageMemoryBarrier{.src_stages = gfx::PipelineStages::TopOfPipe,
                                      .dst_stages = gfx::PipelineStages::ComputeShader,
                                      .src_access = gfx::Access::None,
                                      .dst_access = gfx::Access::ShaderWrite,
                                      .old_layout = gfx::ImageLayout::Undefined,
                                      .new_layout = gfx::ImageLayout::General};
  }
  else if (init_scope == gfx::ImageUsageScope::WriteColorAttachment)
  {
    barrier = gfx::ImageMemoryBarrier{.src_stages = gfx::PipelineStages::TopOfPipe,
                                      .dst_stages = gfx::PipelineStages::ColorAttachmentOutput,
                                      .src_access = gfx::Access::None,
                                      .dst_access = gfx::Access::ColorAttachmentWrite,
                                      .old_layout = gfx::ImageLayout::Undefined,
                                      .new_layout = gfx::ImageLayout::General};
  }
  else if (init_scope == gfx::ImageUsageScope::WriteDepthStencilAttachment)
  {
    barrier =
        gfx::ImageMemoryBarrier{.src_stages = gfx::PipelineStages::TopOfPipe,
                                .dst_stages = gfx::PipelineStages::LateFragmentTests,
                                .src_access = gfx::Access::None,
                                .dst_access = gfx::Access::DepthStencilAttachmentWrite,
                                .old_layout = gfx::ImageLayout::Undefined,
                                .new_layout = gfx::ImageLayout::DepthStencilAttachmentOptimal};
  }

  if (has_bits(scope, gfx::ImageUsageScope::Undefined))
  {
    scope &= ~gfx::ImageUsageScope::Undefined;
    barriers
        .push(gfx::ImageMemoryBarrier{.src_stages = gfx::PipelineStages::TopOfPipe,
                                      .dst_stages = dst_stages,
                                      .src_access = gfx::Access::None,
                                      .dst_access = dst_access,
                                      .old_layout = gfx::ImageLayout::Undefined,
                                      .new_layout = gfx::ImageLayout::ColorAttachmentOptimal})
        .unwrap();
  }
}

inline void acquire_image(gfx::ImageUsageScope scope, gfx::PipelineStages dst_stages,
                          gfx::Access dst_access, gfx::ImageLayout new_layout,
                          stx::Array<gfx::ImageMemoryBarrier, MAX_SCOPE_BARRIERS> &barriers)
{
  if (has_bits(scope, gfx::ImageUsageScope::TransferSrc | gfx::ImageUsageScope::TransferDst))
  {
    barriers
        .push(gfx::ImageMemoryBarrier{.src_stages = gfx::PipelineStages::Transfer,
                                      .dst_stages = dst_stages,
                                      .src_access =
                                          gfx::Access::TransferRead | gfx::Access::TransferWrite,
                                      .dst_access = dst_access,
                                      .old_layout = gfx::ImageLayout::General,
                                      .new_layout = new_layout})
        .unwrap();
  }
  else if (has_bits(scope, gfx::ImageUsageScope::TransferSrc))
  {
    barriers
        .push(gfx::ImageMemoryBarrier{.src_stages = gfx::PipelineStages::Transfer,
                                      .dst_stages = dst_stages,
                                      .src_access = gfx::Access::TransferRead,
                                      .dst_access = dst_access,
                                      .old_layout = gfx::ImageLayout::TransferSrcOptimal,
                                      .new_layout = new_layout})
        .unwrap();
  }
  else if (has_bits(scope, gfx::ImageUsageScope::TransferDst))
  {
    barriers
        .push(gfx::ImageMemoryBarrier{.src_stages = gfx::PipelineStages::Transfer,
                                      .dst_stages = dst_stages,
                                      .src_access = gfx::Access::TransferWrite,
                                      .dst_access = dst_access,
                                      .old_layout = gfx::ImageLayout::TransferDstOptimal,
                                      .new_layout = new_layout})
        .unwrap();
  }

  // if scope has compute shader write then it will always be transitioned to general
  if (has_bits(scope, gfx::ImageUsageScope::ComputeShaderStorage))
  {
    barriers
        .push(gfx::ImageMemoryBarrier{.src_stages = gfx::PipelineStages::ComputeShader,
                                      .dst_stages = dst_stages,
                                      .src_access =
                                          gfx::Access::ShaderWrite | gfx::Access::ShaderRead,
                                      .dst_access = dst_access,
                                      .old_layout = gfx::ImageLayout::General,
                                      .new_layout = new_layout})
        .unwrap();
  }
  else if (has_bits(scope, gfx::ImageUsageScope::ComputeShaderSampled))
  {
    barriers
        .push(gfx::ImageMemoryBarrier{.src_stages = gfx::PipelineStages::ComputeShader,
                                      .dst_stages = dst_stages,
                                      .src_access = gfx::Access::ShaderRead,
                                      .dst_access = dst_access,
                                      .old_layout = gfx::ImageLayout::ShaderReadOnlyOptimal,
                                      .new_layout = new_layout})
        .unwrap();
  }

  if (has_bits(scope, gfx::ImageUsageScope::VertexShaderSampled))
  {
    barriers
        .push(gfx::ImageMemoryBarrier{.src_stages = gfx::PipelineStages::VertexShader,
                                      .dst_stages = dst_stages,
                                      .src_access = gfx::Access::ShaderRead,
                                      .dst_access = dst_access,
                                      .old_layout = gfx::ImageLayout::ShaderReadOnlyOptimal,
                                      .new_layout = new_layout})
        .unwrap();
  }

  if (has_bits(scope, gfx::ImageUsageScope::FragmentShaderSampled))
  {
    barriers
        .push(gfx::ImageMemoryBarrier{.src_stages = gfx::PipelineStages::FragmentShader,
                                      .dst_stages = dst_stages,
                                      .src_access = gfx::Access::ShaderRead,
                                      .dst_access = dst_access,
                                      .old_layout = gfx::ImageLayout::ShaderReadOnlyOptimal,
                                      .new_layout = new_layout})
        .unwrap();
  }

  if (has_bits(scope, gfx::ImageUsageScope::InputAttachment))
  {
    barriers
        .push(gfx::ImageMemoryBarrier{.src_stages = gfx::PipelineStages::FragmentShader,
                                      .dst_stages = dst_stages,
                                      .src_access = gfx::Access::InputAttachmentRead,
                                      .dst_access = dst_access,
                                      .old_layout = gfx::ImageLayout::ShaderReadOnlyOptimal,
                                      .new_layout = new_layout})
        .unwrap();
  }

  if (has_bits(scope, gfx::ImageUsageScope::ReadColorAttachment |
                          gfx::ImageUsageScope::WriteColorAttachment))
  {
    barriers
        .push(gfx::ImageMemoryBarrier{.src_stages = gfx::PipelineStages::ColorAttachmentOutput,
                                      .dst_stages = dst_stages,
                                      .src_access = gfx::Access::ColorAttachmentRead |
                                                    gfx::Access::ColorAttachmentWrite,
                                      .dst_access = dst_access,
                                      .old_layout = gfx::ImageLayout::ColorAttachmentOptimal,
                                      .new_layout = new_layout})
        .unwrap();
  }
  else if (has_bits(scope, gfx::ImageUsageScope::ReadColorAttachment))
  {
    barriers
        .push(gfx::ImageMemoryBarrier{.src_stages = gfx::PipelineStages::ColorAttachmentOutput,
                                      .dst_stages = dst_stages,
                                      .src_access = gfx::Access::ColorAttachmentRead,
                                      .dst_access = dst_access,
                                      .old_layout = gfx::ImageLayout::ColorAttachmentOptimal,
                                      .new_layout = new_layout})
        .unwrap();
  }
  else if (has_bits(scope, gfx::ImageUsageScope::WriteColorAttachment))
  {
    barriers
        .push(gfx::ImageMemoryBarrier{.src_stages = gfx::PipelineStages::ColorAttachmentOutput,
                                      .dst_stages = dst_stages,
                                      .src_access = gfx::Access::ColorAttachmentWrite,
                                      .dst_access = dst_access,
                                      .old_layout = gfx::ImageLayout::ColorAttachmentOptimal,
                                      .new_layout = new_layout})
        .unwrap();
  }

  if (has_bits(scope, gfx::ImageUsageScope::ReadDepthStencilAttachment |
                          gfx::ImageUsageScope::WriteDepthStencilAttachment))
  {
    barriers
        .push(gfx::ImageMemoryBarrier{.src_stages = gfx::PipelineStages::EarlyFragmentTests |
                                                    gfx::PipelineStages::LateFragmentTests,
                                      .dst_stages = dst_stages,
                                      .src_access = gfx::Access::DepthStencilAttachmentRead |
                                                    gfx::Access::DepthStencilAttachmentWrite,
                                      .dst_access = dst_access,
                                      .old_layout = gfx::ImageLayout::DepthStencilAttachmentOptimal,
                                      .new_layout = new_layout})
        .unwrap();
  }
  else if (has_bits(scope, gfx::ImageUsageScope::ReadDepthStencilAttachment))
  {
    barriers
        .push(gfx::ImageMemoryBarrier{.src_stages = gfx::PipelineStages::EarlyFragmentTests,
                                      .dst_stages = dst_stages,
                                      .src_access = gfx::Access::DepthStencilAttachmentRead,
                                      .dst_access = dst_access,
                                      .old_layout = gfx::ImageLayout::DepthStencilReadOnlyOptimal,
                                      .new_layout = new_layout})
        .unwrap();
  }
  else if (has_bits(scope, gfx::ImageUsageScope::WriteDepthStencilAttachment))
  {
    barriers
        .push(gfx::ImageMemoryBarrier{.src_stages = gfx::PipelineStages::LateFragmentTests,
                                      .dst_stages = dst_stages,
                                      .src_access = gfx::Access::DepthStencilAttachmentWrite,
                                      .dst_access = dst_access,
                                      .old_layout = gfx::ImageLayout::DepthStencilAttachmentOptimal,
                                      .new_layout = new_layout})
        .unwrap();
  }
}

// release side-effects to operations that are allowed to have concurrent non-mutating access on the resource
inline void release_image(gfx::ImageUsageScope scope, gfx::PipelineStages src_stages,
                          gfx::Access src_access, gfx::ImageLayout old_layout,
                          stx::Array<gfx::ImageMemoryBarrier, MAX_SCOPE_BARRIERS> &barriers)
{
  // only shader-sampled images can run parallel to other command views
  // only transitioned to Shader read only if it is not used as storage at the same stage
  //
  // for all non-shader-read-only-optimal usages, an acquire must be performed
  //
  if (has_bits(scope, gfx::ImageUsageScope::ComputeShaderSampled) &&
      !has_bits(scope, gfx::ImageUsageScope::ComputeShaderStorage))
  {
    barriers
        .push(gfx::ImageMemoryBarrier{.src_stages = src_stages,
                                      .dst_stages = gfx::PipelineStages::ComputeShader,
                                      .src_access = src_access,
                                      .dst_access = gfx::Access::ShaderRead,
                                      .old_layout = old_layout,
                                      .new_layout = gfx::ImageLayout::ShaderReadOnlyOptimal})
        .unwrap();
  }

  if (has_any_bit(scope, gfx::ImageUsageScope::VertexShaderSampled |
                             gfx::ImageUsageScope::FragmentShaderSampled |
                             gfx::ImageUsageScope::InputAttachment))
  {
    gfx::PipelineStages dst_stages = gfx::PipelineStages::None;

    if (has_bits(scope, gfx::ImageUsageScope::VertexShaderSampled))
    {
      dst_stages |= gfx::PipelineStages::VertexShader;
    }

    if (has_any_bit(scope, gfx::ImageUsageScope::FragmentShaderSampled |
                               gfx::ImageUsageScope::InputAttachment))
    {
      dst_stages |= gfx::PipelineStages::FragmentShader;
    }

    barriers
        .push(gfx::ImageMemoryBarrier{.src_stages = src_stages,
                                      .dst_stages = dst_stages,
                                      .src_access = src_access,
                                      .dst_access = gfx::Access::ShaderRead,
                                      .old_layout = old_layout,
                                      .new_layout = gfx::ImageLayout::ShaderReadOnlyOptimal})
        .unwrap();
  }
}

// apply to both src and dst since they require layout transitions
constexpr ImageScope transfer_image_scope(gfx::ImageUsageScope scope)
{
  gfx::ImageLayout    layout = gfx::ImageLayout::Undefined;
  gfx::PipelineStages stages = gfx::PipelineStages::Transfer;
  gfx::Access         access = gfx::Access::None;

  if (has_bits(scope, gfx::ImageUsageScope::TransferSrc | gfx::ImageUsageScope::TransferDst))
  {
    access = gfx::Access::TransferRead | gfx::Access::TransferWrite;
    layout = gfx::ImageLayout::General;
  }
  else if (has_bits(scope, gfx::ImageUsageScope::TransferSrc))
  {
    access = gfx::Access::TransferRead;
    layout = gfx::ImageLayout::TransferSrcOptimal;
  }
  else if (has_bits(scope, gfx::ImageUsageScope::TransferDst))
  {
    access = gfx::Access::TransferWrite;
    layout = gfx::ImageLayout::TransferDstOptimal;
  }

  return ImageScope{.stages = stages, .access = access, .layout = layout};
}

constexpr ImageScope compute_storage_image_scope(gfx::ImageUsageScope scope)
{
  gfx::ImageLayout    layout = gfx::ImageLayout::Undefined;
  gfx::PipelineStages stages = gfx::PipelineStages::ComputeShader;
  gfx::Access         access = gfx::Access::None;

  if (has_bits(scope, gfx::ImageUsageScope::ComputeShaderSampled |
                          gfx::ImageUsageScope::ComputeShaderStorage))
  {
    access = gfx::Access::ShaderRead | gfx::Access::ShaderWrite;
    layout = gfx::ImageLayout::General;
  }
  else if (has_bits(scope, gfx::ImageUsageScope::ComputeShaderSampled))
  {
    access = gfx::Access::ShaderRead;
    layout = gfx::ImageLayout::ShaderReadOnlyOptimal;
  }
  else if (has_bits(scope, gfx::ImageUsageScope::ComputeShaderStorage))
  {
    access = gfx::Access::ShaderWrite;
    layout = gfx::ImageLayout::General;
  }

  return ImageScope{.stages = stages, .access = access, .layout = layout};
}

constexpr ImageScope color_attachment_image_scope(gfx::ImageUsageScope scope)
{
  gfx::ImageLayout    layout = gfx::ImageLayout::ColorAttachmentOptimal;
  gfx::PipelineStages stages = gfx::PipelineStages::ColorAttachmentOutput;
  gfx::Access         access = gfx::Access::None;

  if (has_bits(scope, gfx::ImageUsageScope::ReadColorAttachment))
  {
    access |= gfx::Access::ColorAttachmentRead;
  }

  if (has_bits(scope, gfx::ImageUsageScope::WriteColorAttachment))
  {
    access |= gfx::Access::ColorAttachmentWrite;
  }

  return ImageScope{.stages = stages, .access = access, .layout = layout};
}

constexpr ImageScope depth_stencil_attachment_image_scope(gfx::ImageUsageScope scope)
{
  gfx::ImageLayout    layout = gfx::ImageLayout::Undefined;
  gfx::PipelineStages stages = gfx::PipelineStages::None;
  gfx::Access         access = gfx::Access::None;

  if (has_bits(scope, gfx::ImageUsageScope::ReadDepthStencilAttachment |
                          gfx::ImageUsageScope::WriteDepthStencilAttachment))
  {
    stages = gfx::PipelineStages::EarlyFragmentTests | gfx::PipelineStages::LateFragmentTests;
    access = gfx::Access::DepthStencilAttachmentRead | gfx::Access::DepthStencilAttachmentWrite;
    layout = gfx::ImageLayout::DepthStencilAttachmentOptimal;
  }
  else if (has_bits(scope, gfx::ImageUsageScope::ReadDepthStencilAttachment))
  {
    stages = gfx::PipelineStages::EarlyFragmentTests;
    access = gfx::Access::DepthStencilAttachmentRead;
    layout = gfx::ImageLayout::DepthStencilReadOnlyOptimal;
  }
  else if (has_bits(scope, gfx::ImageUsageScope::WriteDepthStencilAttachment))
  {
    stages = gfx::PipelineStages::LateFragmentTests;
    access = gfx::Access::DepthStencilAttachmentWrite;
    layout = gfx::ImageLayout::DepthStencilAttachmentOptimal;
  }

  return ImageScope{.stages = stages, .access = access, .layout = layout};
}

constexpr ImageScope color_attachment_image_scope(gfx::RenderPassAttachment const &attachment)
{
  gfx::ImageLayout layout = gfx::ImageLayout::ColorAttachmentOptimal;
  gfx::Access      access = gfx::Access::None;

  if (attachment.load_op == gfx::LoadOp::Clear || attachment.load_op == gfx::LoadOp::DontCare ||
      attachment.store_op == gfx::StoreOp::Store)
  {
    access |= gfx::Access::ColorAttachmentWrite;
  }

  if (attachment.load_op == gfx::LoadOp::Load)
  {
    access |= gfx::Access::ColorAttachmentRead;
  }

  return ImageScope{
      .stages = gfx::PipelineStages::ColorAttachmentOutput, .access = access, .layout = layout};
}

constexpr ImageScope
    depth_stencil_attachment_image_scope(gfx::RenderPassAttachment const &attachment)
{
  gfx::ImageLayout    layout = gfx::ImageLayout::Undefined;
  gfx::Access         access = gfx::Access::None;
  gfx::PipelineStages stages = gfx::PipelineStages::None;

  if (attachment.load_op == gfx::LoadOp::Clear || attachment.load_op == gfx::LoadOp::DontCare ||
      attachment.store_op == gfx::StoreOp::Store || attachment.store_op == gfx::StoreOp::DontCare ||
      attachment.stencil_load_op == gfx::LoadOp::Clear ||
      attachment.stencil_load_op == gfx::LoadOp::DontCare ||
      attachment.stencil_store_op == gfx::StoreOp::Store ||
      attachment.stencil_store_op == gfx::StoreOp::DontCare)
  {
    access |= gfx::Access::DepthStencilAttachmentWrite;
    stages |= gfx::PipelineStages::LateFragmentTests;
  }

  if (attachment.load_op == gfx::LoadOp::Load || attachment.stencil_load_op == gfx::LoadOp::Load)
  {
    access |= gfx::Access::DepthStencilAttachmentRead;
    stages |= gfx::PipelineStages::EarlyFragmentTests;
  }

  if (has_bits(access, gfx::Access::ColorAttachmentWrite))
  {
    layout = gfx::ImageLayout::DepthStencilAttachmentOptimal;
  }
  else if (has_bits(access, gfx::Access::ColorAttachmentRead))
  {
    layout = gfx::ImageLayout::DepthStencilReadOnlyOptimal;
  }

  return ImageScope{.stages = stages, .access = access, .layout = layout};
}

void CommandBuffer::fill_buffer(gfx::Buffer buffer, u64 offset, u64 size, u32 data)
{
  hook->fill_buffer(buffer, offset, size, data);

  gfx::BufferMemoryBarrier barriers[1];
  u8                       num_barriers = 0;

  if (graph->buffers[buffer].sync_scope.sync(gfx::MemoryAccess::Write,
                                             gfx::PipelineStages::Transfer, barriers[0]))
  {
    barriers[0].buffer = graph->buffers[buffer].handle;
    num_barriers++;
  }

  if (num_barriers > 0)
  {
    graph->driver->cmd_insert_barriers(handle, stx::Span{barriers, num_barriers}, {});
  }

  graph->driver->cmd_fill_buffer(handle, buffer, offset, size, data);
}

void CommandBuffer::copy_buffer(gfx::Buffer src, gfx::Buffer dst,
                                stx::Span<gfx::BufferCopy const> copies)
{
  hook->copy_buffer(src, dst, copies);

  gfx::BufferMemoryBarrier barriers[2];
  u8                       num_barriers = 0;

  if (src != dst)
  {
    if (graph->buffers[src].sync_scope.sync(gfx::MemoryAccess::Read, gfx::PipelineStages::Transfer,
                                            barriers[num_barriers]))
    {
      barriers[num_barriers].buffer = graph->buffers[src].handle;
      num_barriers++;
    }

    if (graph->buffers[dst].sync_scope.sync(gfx::MemoryAccess::Write, gfx::PipelineStages::Transfer,
                                            barriers[num_barriers]))
    {
      barriers[num_barriers].buffer = graph->buffers[dst].handle;
      num_barriers++;
    }
  }
  else
  {
    if (graph->buffers[src].sync_scope.sync(gfx::MemoryAccess::Read | gfx::MemoryAccess::Write,
                                            gfx::PipelineStages::Transfer, barriers[num_barriers]))
    {
      barriers[num_barriers].buffer = graph->buffers[src].handle;
      num_barriers++;
    }
  }

  if (num_barriers > 0)
  {
    graph->driver->cmd_insert_barriers(handle, stx::Span{barriers, num_barriers}, {});
  }

  graph->driver->cmd_copy_buffer(handle, src, dst, copies);
}

void CommandBuffer::update_buffer(stx::Span<u8 const> src, u64 dst_offset, gfx::Buffer dst)
{
  hook->update_buffer(src, dst_offset, dst);

  gfx::BufferMemoryBarrier barriers[1];
  u8                       num_barriers = 0;

  if (graph->buffers[dst].sync_scope.sync(gfx::MemoryAccess::Write, gfx::PipelineStages::Transfer,
                                          barriers[0]))
  {
    barriers[0].buffer = graph->buffers[dst].handle;
    num_barriers++;
  }

  if (num_barriers > 0)
  {
    graph->driver->cmd_insert_barriers(handle, stx::Span{barriers, num_barriers}, {});
  }

  graph->driver->cmd_update_buffer(handle, src, dst_offset, dst);
}

void CommandBuffer::copy_image(gfx::Image src, gfx::Image dst,
                               stx::Span<gfx::ImageCopy const> copies)
{
  hook->copy_image(src, dst, copies);

  gfx::ImageMemoryBarrier pre_barriers[20];
  gfx::ImageMemoryBarrier post_barriers[20];
  u8                      num_barriers = 0;

  if (src != dst)
  {
    if (graph->images[src].sync_scope.sync(gfx::MemoryAccess::Read, gfx::PipelineStages::Transfer,
                                           gfx::ImageLayout::TransferSrcOptimal,
                                           barriers[num_barriers]))
    {
      barriers[num_barriers].image   = graph->images[src].handle;
      barriers[num_barriers].aspects = graph->images[src].desc.aspects;
      num_barriers++;
    }

    if (graph->images[dst].sync_scope.sync(gfx::MemoryAccess::Write, gfx::PipelineStages::Transfer,
                                           gfx::ImageLayout::TransferDstOptimal,
                                           barriers[num_barriers]))
    {
      barriers[num_barriers].image   = graph->images[dst].handle;
      barriers[num_barriers].aspects = graph->images[dst].desc.aspects;
      num_barriers++;
    }
  }
  else
  {
    barriers[num_barriers].image             = graph->images[src].handle;
    barriers[num_barriers].aspects           = graph->images[src].desc.aspects;
    barriers[num_barriers].old_layout        = gfx::ImageLayout::ShaderReadOnlyOptimal;
    barriers[num_barriers].new_layout        = gfx::ImageLayout::General;
    barriers[num_barriers].first_array_layer = 0;
    barriers[num_barriers].num_array_layers  = gfx::REMAINING_ARRAY_LAYERS;
    barriers[num_barriers].first_mip_level   = 0;
    barriers[num_barriers].num_mip_levels    = gfx::REMAINING_MIP_LEVELS;
    barriers[num_barriers].src_stages        = gfx::;
    barriers[num_barriers].src_access        = ;
    barriers[num_barriers].dst_stages        = gfx::PipelineStages::Transfer;
    barriers[num_barriers].dst_access = gfx::Access::TransferWrite | gfx::Access::TransferRead;
    num_barriers++;
  }

  graph->driver->cmd_copy_image(handle, src, dst, copies);

  if (num_barriers > 0)
  {
    graph->driver->cmd_insert_barriers(handle, {}, stx::Span{barriers, num_barriers});
  }
}

void CommandBuffer::copy_buffer_to_image(gfx::Buffer src, gfx::Image dst,
                                         stx::Span<gfx::BufferImageCopy const> copies)
{
  gfx::BufferMemoryBarrier buffer_memory_barriers[1];
  u8                       num_buffer_memory_barriers = 0;
  gfx::ImageMemoryBarrier  image_memory_barriers[1];
  u8                       num_image_memory_barriers = 0;

  if (graph->buffers[src].sync_scope.sync(gfx::MemoryAccess::Read, gfx::PipelineStages::Transfer,
                                          buffer_memory_barriers[num_buffer_memory_barriers]))
  {
    buffer_memory_barriers[num_buffer_memory_barriers].buffer = graph->buffers[src].handle;
    num_buffer_memory_barriers++;
  }

  if (graph->images[dst].sync_scope.sync(gfx::MemoryAccess::Write, gfx::PipelineStages::Transfer,
                                         gfx::ImageLayout::TransferDstOptimal,
                                         image_memory_barriers[num_image_memory_barriers]))
  {
    image_memory_barriers[num_image_memory_barriers].image   = graph->images[dst].handle;
    image_memory_barriers[num_image_memory_barriers].aspects = graph->images[dst].desc.aspects;
    num_image_memory_barriers++;
  }

  if (num_buffer_memory_barriers > 0 || num_image_memory_barriers > 0)
  {
    graph->driver->cmd_insert_barriers(
        handle, stx::Span{buffer_memory_barriers, num_buffer_memory_barriers},
        stx::Span{image_memory_barriers, num_image_memory_barriers});
  }

  graph->driver->cmd_copy_buffer_to_image(handle, src, dst, copies);
}

void CommandBuffer::blit_image(gfx::Image src, gfx::Image dst,
                               stx::Span<gfx::ImageBlit const> blits, gfx::Filter filter)
{
  hook->blit_image(src, dst, blits, filter);

  gfx::ImageMemoryBarrier barriers[2];
  u8                      num_barriers = 0;

  if (src != dst)
  {
    if (graph->images[src].sync_scope.sync(gfx::MemoryAccess::Read, gfx::PipelineStages::Transfer,
                                           gfx::ImageLayout::TransferSrcOptimal,
                                           barriers[num_barriers]))
    {
      barriers[num_barriers].image   = graph->images[src].handle;
      barriers[num_barriers].aspects = graph->images[src].desc.aspects;
      num_barriers++;
    }

    if (graph->images[dst].sync_scope.sync(gfx::MemoryAccess::Write, gfx::PipelineStages::Transfer,
                                           gfx::ImageLayout::TransferDstOptimal,
                                           barriers[num_barriers]))
    {
      barriers[num_barriers].image   = graph->images[dst].handle;
      barriers[num_barriers].aspects = graph->images[dst].desc.aspects;
      num_barriers++;
    }
  }
  else
  {
    if (graph->images[src].sync_scope.sync(gfx::MemoryAccess::Read | gfx::MemoryAccess::Write,
                                           gfx::PipelineStages::Transfer, gfx::ImageLayout::General,
                                           barriers[num_barriers]))
    {
      barriers[num_barriers].image   = graph->images[src].handle;
      barriers[num_barriers].aspects = graph->images[src].desc.aspects;
      num_barriers++;
    }
  }

  if (num_barriers > 0)
  {
    graph->driver->cmd_insert_barriers(handle, {}, stx::Span{barriers, num_barriers});
  }

  graph->driver->cmd_blit_image(handle, src, dst, blits, filter);
}

void CommandBuffer::begin_render_pass(
    gfx::Framebuffer framebuffer, gfx::RenderPass render_pass, IRect render_area,
    stx::Span<gfx::Color const>        color_attachments_clear_values,
    stx::Span<gfx::DepthStencil const> depth_stencil_attachments_clear_values)
{
  hook->begin_render_pass(framebuffer, render_pass, render_area, color_attachments_clear_values,
                          depth_stencil_attachments_clear_values);

  gfx::RenderPassDesc const &renderpass_desc = graph->render_passes[render_pass].desc;
  gfx::ImageMemoryBarrier    barriers[gfx::MAX_COLOR_ATTACHMENTS + 1];
  u32                        num_barriers = 0;

  for (u32 i = 0; i < renderpass_desc.num_color_attachments; i++)
  {
    gfx::Image image =
        graph->image_views[graph->framebuffers[framebuffer].desc.color_attachments[i]].desc.image;
    gfx::ImageResource const &resource = graph->images[image];

    if (graph->images[image].sync_scope.sync(
            renderpass_desc.color_attachments[i].get_color_image_access(), barrier))
    {
      barrier.image          = resource.handle;
      barrier.aspects        = resource.desc.aspects;
      barriers[num_barriers] = barrier;
      num_barriers++;
    }
  }

  {
    gfx::Image image =
        graph->image_views[graph->framebuffers[framebuffer].desc.depth_stencil_attachment]
            .desc.image;
    gfx::ImageResource const &resource = graph->images[image];

    if (graph->images[image].state.sync(
            renderpass_desc.depth_stencil_attachment.get_depth_stencil_image_access(), barrier))
    {
      barrier.image          = resource.handle;
      barrier.aspects        = resource.desc.aspects;
      barriers[num_barriers] = barrier;
      num_barriers++;
    }
  }

  if (num_barriers > 0)
  {
    graph->driver->cmd_insert_barriers(handle, {}, stx::Span{barriers, num_barriers});
  }

  graph->driver->cmd_begin_render_pass(
      handle, graph->framebuffers[framebuffer].handle, graph->render_passes[render_pass].handle,
      render_area, color_attachments_clear_values, depth_stencil_attachments_clear_values);
}

void CommandBuffer::end_render_pass()
{
  hook->end_render_pass();
  graph->driver->cmd_end_render_pass(handle);
}

inline void generate_descriptor_barriers(gfx::DescriptorSetBindings const &bindings, Graph &graph,
                                         stx::Vec<gfx::ImageMemoryBarrier>  &image_barriers,
                                         stx::Vec<gfx::BufferMemoryBarrier> &buffer_barriers)
{
  // todo(lamarrr): add pipeline bind point
  hook->push_descriptor_set(set, bindings);

  for (gfx::CombinedImageSamplerBinding const &binding : bindings.combined_image_samplers)
  {
    graph.images[graph.image_views[binding.image_view].desc.image].bindings =
        gfx::ImageBindings::None;
  }

  for (gfx::InputAttachmentBinding const &binding : bindings.input_attachments)
  {
    graph.images[graph.image_views[binding.image_view].desc.image].bindings =
        gfx::ImageBindings::None;
  }

  for (gfx::SampledImageBinding const &binding : bindings.sampled_images)
  {
    graph.images[graph.image_views[binding.image_view].desc.image].bindings =
        gfx::ImageBindings::None;
  }

  for (gfx::StorageBufferBinding const &binding : bindings.storage_buffers)
  {
    graph.buffers[binding.buffer].bindings = gfx::BufferBindings::None;
  }

  for (gfx::StorageImageBinding const &binding : bindings.storage_images)
  {
    graph.images[graph.image_views[binding.image_view].desc.image].bindings =
        gfx::ImageBindings::None;
  }

  for (gfx::StorageTexelBufferBinding const &binding : bindings.storage_texel_buffers)
  {
    graph.buffers[graph.buffer_views[binding.buffer_view].desc.buffer].bindings =
        gfx::BufferBindings::None;
  }

  for (gfx::UniformBufferBinding const &binding : bindings.uniform_buffers)
  {
    graph.buffers[binding.buffer].bindings = gfx::BufferBindings::None;
  }

  for (gfx::UniformTexelBufferBinding const &binding : bindings.uniform_texel_buffers)
  {
    graph.buffers[graph.buffer_views[binding.buffer_view].desc.buffer].bindings =
        gfx::BufferBindings::None;
  }

  /////////////////////////////////

  for (gfx::CombinedImageSamplerBinding const &binding : bindings.combined_image_samplers)
  {
    graph.images[graph.image_views[binding.image_view].desc.image].bindings |=
        gfx::ImageBindings::Sampled;
  }

  for (gfx::InputAttachmentBinding const &binding : bindings.input_attachments)
  {
    graph.images[graph.image_views[binding.image_view].desc.image].bindings |=
        gfx::ImageBindings::InputAttachment;
  }

  for (gfx::SampledImageBinding const &binding : bindings.sampled_images)
  {
    graph.images[graph.image_views[binding.image_view].desc.image].bindings |=
        gfx::ImageBindings::Sampled;
  }

  for (gfx::StorageBufferBinding const &binding : bindings.storage_buffers)
  {
    graph.buffers[binding.buffer].bindings |= gfx::BufferBindings::Storage;
  }

  for (gfx::StorageImageBinding const &binding : bindings.storage_images)
  {
    graph.images[graph.image_views[binding.image_view].desc.image].bindings |=
        gfx::ImageBindings::Storage;
  }

  for (gfx::StorageTexelBufferBinding const &binding : bindings.storage_texel_buffers)
  {
    graph.buffers[graph.buffer_views[binding.buffer_view].desc.buffer].bindings |=
        gfx::BufferBindings::StorageTexel;
  }

  for (gfx::UniformBufferBinding const &binding : bindings.uniform_buffers)
  {
    graph.buffers[binding.buffer].bindings |= gfx::BufferBindings::Uniform;
  }

  for (gfx::UniformTexelBufferBinding const &binding : bindings.uniform_texel_buffers)
  {
    graph.buffers[graph.buffer_views[binding.buffer_view].desc.buffer].bindings |=
        gfx::BufferBindings::UniformTexel;
  }

  //////////////////////////////////////////////////////////////////

  for (gfx::CombinedImageSamplerBinding const &binding : bindings.combined_image_samplers)
  {
    graph.images[graph.image_views[binding.image_view].desc.image].bindings |=
        gfx::ImageBindings::Sampled;
  }

  for (gfx::InputAttachmentBinding const &binding : bindings.input_attachments)
  {
    graph.images[graph.image_views[binding.image_view].desc.image].bindings |=
        gfx::ImageBindings::InputAttachment;
  }

  for (gfx::SampledImageBinding const &binding : bindings.sampled_images)
  {
    graph.images[graph.image_views[binding.image_view].desc.image].bindings |=
        gfx::ImageBindings::Sampled;
  }

  for (gfx::StorageBufferBinding const &binding : bindings.storage_buffers)
  {
    graph.buffers[binding.buffer].bindings |= gfx::BufferBindings::Storage;
  }

  for (gfx::StorageImageBinding const &binding : bindings.storage_images)
  {
    graph.images[graph.image_views[binding.image_view].desc.image].bindings |=
        gfx::ImageBindings::Storage;
  }

  for (gfx::StorageTexelBufferBinding const &binding : bindings.storage_texel_buffers)
  {
    graph.buffers[graph.buffer_views[binding.buffer_view].desc.buffer].bindings |=
        gfx::BufferBindings::StorageTexel;
  }

  for (gfx::UniformBufferBinding const &binding : bindings.uniform_buffers)
  {
    graph.buffers[binding.buffer].bindings |= gfx::BufferBindings::Uniform;
  }

  for (gfx::UniformTexelBufferBinding const &binding : bindings.uniform_texel_buffers)
  {
    graph.buffers[graph.buffer_views[binding.buffer_view].desc.buffer].bindings |=
        gfx::BufferBindings::UniformTexel;
  }

  gfx::ImageMemoryBarrier  image_memory_barrier;
  gfx::BufferMemoryBarrier buffer_memory_barrier;

  for (gfx::CombinedImageSamplerBinding const &binding : bindings.combined_image_samplers)
  {
    if (graph->images[graph->image_views[binding.image_view].desc.image].state.sync(
            gfx::ImageAccess{.stages = gfx::PipelineStages::AllCommands,
                             .access = gfx::Access::ShaderRead,
                             .layout = gfx::ImageLayout::ShaderReadOnlyOptimal},
            image_memory_barrier))
    {
      image_memory_barrier.image =
          graph->images[graph->image_views[binding.image_view].desc.image].handle;
      image_memory_barrier.aspects =
          graph->images[graph->image_views[binding.image_view].desc.image].desc.aspects;
      context.descriptor_set_barriers[set].image.push_inplace(image_memory_barrier).unwrap();
    }
  }

  for (gfx::SampledImageBinding const &binding : bindings.sampled_images)
  {
    if (graph->images[graph->image_views[binding.image_view].desc.image].state.sync(
            gfx::ImageAccess{.stages = gfx::PipelineStages::AllCommands,
                             .access = gfx::Access::ShaderRead,
                             .layout = gfx::ImageLayout::ShaderReadOnlyOptimal},
            image_memory_barrier))
    {
      image_memory_barrier.image =
          graph->images[graph->image_views[binding.image_view].desc.image].handle;
      image_memory_barrier.aspects =
          graph->images[graph->image_views[binding.image_view].desc.image].desc.aspects;
      context.descriptor_set_barriers[set].image.push_inplace(image_memory_barrier).unwrap();
    }
  }

  for (gfx::StorageImageBinding const &binding : bindings.storage_images)
  {
    if (graph->images[graph->image_views[binding.image_view].desc.image].state.sync(
            gfx::ImageAccess{.stages = gfx::PipelineStages::AllCommands,
                             .access = gfx::Access::ShaderWrite,
                             .layout = gfx::ImageLayout::General},
            image_memory_barrier))
    {
      image_memory_barrier.image =
          graph->images[graph->image_views[binding.image_view].desc.image].handle;
      image_memory_barrier.aspects =
          graph->images[graph->image_views[binding.image_view].desc.image].desc.aspects;
      context.descriptor_set_barriers[set].image.push_inplace(image_memory_barrier).unwrap();
    }
  }

  for (gfx::UniformTexelBufferBinding const &binding : bindings.uniform_texel_buffers)
  {
    if (graph->buffers[graph->buffer_views[binding.buffer_view].desc.buffer].state.sync(
            gfx::BufferAccess{.stages = gfx::PipelineStages::AllCommands,
                              .access = gfx::Access::ShaderRead},
            buffer_memory_barrier))
    {
      buffer_memory_barrier.buffer =
          graph->buffers[graph->buffer_views[binding.buffer_view].desc.buffer].handle;
      context.descriptor_set_barriers[set].buffer.push_inplace(buffer_memory_barrier).unwrap();
    }
  }

  for (gfx::StorageTexelBufferBinding const &binding : bindings.storage_texel_buffers)
  {
    if (graph->buffers[graph->buffer_views[binding.buffer_view].desc.buffer].state.sync(
            gfx::BufferAccess{.stages = gfx::PipelineStages::AllCommands,
                              .access = gfx::Access::ShaderWrite},
            buffer_memory_barrier))
    {
      buffer_memory_barrier.buffer =
          graph->buffers[graph->buffer_views[binding.buffer_view].desc.buffer].handle;
      context.descriptor_set_barriers[set].buffer.push_inplace(buffer_memory_barrier).unwrap();
    }
  }

  for (gfx::UniformBufferBinding const &binding : bindings.uniform_buffers)
  {
    if (graph->buffers[binding.buffer].state.sync(
            gfx::BufferAccess{.stages = gfx::PipelineStages::AllCommands,
                              .access = gfx::Access::ShaderRead},
            buffer_memory_barrier))
    {
      buffer_memory_barrier.buffer = graph->buffers[binding.buffer].handle;
      context.descriptor_set_barriers[set].buffer.push_inplace(buffer_memory_barrier).unwrap();
    }
  }

  for (gfx::StorageBufferBinding const &binding : bindings.storage_buffers)
  {
    if (graph->buffers[binding.buffer].state.sync(
            gfx::BufferAccess{.stages = gfx::PipelineStages::AllCommands,
                              .access = gfx::Access::ShaderWrite},
            buffer_memory_barrier))
    {
      buffer_memory_barrier.buffer = graph->buffers[binding.buffer].handle;
      context.descriptor_set_barriers[set].buffer.push_inplace(buffer_memory_barrier).unwrap();
    }
  }

  for (gfx::InputAttachmentBinding const &binding : bindings.input_attachments)
  {
    if (graph->images[graph->image_views[binding.image_view].desc.image].state.sync(
            gfx::ImageAccess{.stages = gfx::PipelineStages::AllCommands,
                             .access = gfx::Access::ShaderRead,
                             .layout = gfx::ImageLayout::ShaderReadOnlyOptimal},
            image_memory_barrier))
    {
      image_memory_barrier.image =
          graph->images[graph->image_views[binding.image_view].desc.image].handle;
      image_memory_barrier.aspects =
          graph->images[graph->image_views[binding.image_view].desc.image].desc.aspects;
      context.descriptor_set_barriers[set].image.push_inplace(image_memory_barrier).unwrap();
    }
  }

  graph->driver->cmd_push_descriptor_set(handle, set, bindings);
}

void CommandBuffer::dispatch(gfx::ComputePipeline pipeline, u32 group_count_x, u32 group_count_y,
                             u32                                         group_count_z,
                             stx::Span<gfx::DescriptorSetBindings const> bindings,
                             stx::Span<u8 const>                         push_constants_data)
{
}

void CommandBuffer::dispatch_indirect(gfx::ComputePipeline pipeline, gfx::Buffer buffer, u64 offset,
                                      stx::Span<gfx::DescriptorSetBindings const> bindings,
                                      stx::Span<u8 const> push_constants_data)
{
}

void CommandBuffer::draw(gfx::GraphicsPipeline pipeline, gfx::RenderState const &state,
                         stx::Span<gfx::Buffer const> vertex_buffers, gfx::Buffer index_buffer,
                         u32 first_index, u32 num_indices, u32 vertex_offset, u32 first_instance,
                         u32 num_instances, stx::Span<gfx::DescriptorSetBindings const> bindings,
                         stx::Span<u8 const> push_constants_data)
{
}

void CommandBuffer::draw_indirect(gfx::GraphicsPipeline pipeline, gfx::RenderState const &state,
                                  stx::Span<gfx::Buffer const> vertex_buffers,
                                  gfx::Buffer index_buffer, gfx::Buffer buffer, u64 offset,
                                  u32 draw_count, u32 stride,
                                  stx::Span<gfx::DescriptorSetBindings const> bindings,
                                  stx::Span<u8 const>                         push_constants_data)
{
}

void CommandBuffer::on_execution_complete_fn(stx::UniqueFn<void()> &&fn)
{
  hook->on_execution_complete_fn(fn);
  completion_tasks.push(std::move(fn)).unwrap();
}

constexpr bool is_renderpass_compatible(RenderPassDesc const &a, RenderPassDesc const &b)
{
  return true;
}
/*
Buffer Graph::create_buffer(BufferDesc const &desc)
{
  return buffers.push(BufferResource{.state = BufferState{.desc = desc}, .handle =
driver->create_buffer(desc)});
}

BufferView Graph::create_buffer_view(BufferViewDesc const &desc)
{
  return buffer_views.push(BufferViewResource{.desc = desc, .handle =
driver->create_buffer_view(desc)});
}

Image Graph::create_image(ImageDesc const &desc)
{
  return images.push(ImageResource{.state = ImageState{.desc = desc}, .handle =
driver->create_image(desc)});
}

ImageView Graph::create_image_view(ImageViewDesc const &desc)
{
  return image_views.push(ImageViewResource{.desc = desc, .handle =
driver->create_image_view(desc)});
}

RenderPass Graph::create_render_pass(RenderPassDesc const &desc)
{
  return render_passes.push(RenderPassResource{.desc = desc, .handle =
driver->create_render_pass(desc)});
}

Framebuffer Graph::create_framebuffer(FramebufferDesc const &desc)
{
  return framebuffers.push(FramebufferResource{.desc= desc,.handle =
driver->create_framebuffer(desc)});
}

ComputePipeline Graph::create_compute_pipeline(ComputePipelineDesc const &desc)
{
  return compute_pipelines.push(ComputePipelineResource{.desc= desc,.handle =
driver->create_compute_pipeline(desc)});
}

GraphicsPipeline Graph::create_graphics_pipeline(GraphicsPipelineDesc const &desc)
{
  return graphics_pipelines.push(GraphicsPipelineResource{.desc=
desc,.handle=driver->create_graphics_pipeline(desc)});
}

BufferDesc const &Graph::get_desc(Buffer buffer) const
{
  return buffers[buffer].desc;
}

BufferViewDesc const &Graph::get_desc(BufferView buffer_view) const
{
  return buffer_views[buffer_view];
}

ImageDesc const &Graph::get_desc(Image image) const
{
  return images[image].desc;
}

ImageViewDesc const &Graph::get_desc(ImageView image_view) const
{
  return image_views[image_view];
}

RenderPassDesc const &Graph::get_desc(RenderPass render_pass) const
{
  return render_passes[render_pass];
}

FramebufferDesc const &Graph::get_desc(Framebuffer framebuffer) const
{
  return framebuffers[framebuffer];
}

ComputePipelineDesc const &Graph::get_desc(ComputePipeline compute_pipeline) const
{
  return compute_pipelines[compute_pipeline];
}

GraphicsPipelineDesc const &Graph::get_desc(GraphicsPipeline graphics_pipeline) const
{
  return graphics_pipelines[graphics_pipeline];
}

BufferState &Graph::get_state(Buffer buffer)
{
  return buffers[buffer];
}

ImageState &Graph::get_state(Image image)
{
  return images[image];
}

void Graph::release(Buffer buffer)
{}

void Graph::release(Image image)
{}

void Graph::release(ImageView image_view)
{}

void Graph::release(RenderPass render_pass)
{}

void Graph::release(Framebuffer framebuffer)
{}

void Graph::release(ComputePipeline compute_pipeline)
{}

void Graph::release(GraphicsPipeline graphics_pipeline)
{}

void validate_resources(Graph &graph)
{
  // TODO(lamarrr): we need a logger
  for (BufferState const &buffer : graph.buffers)
  {
    ASH_CHECK(buffer.desc.usages != BufferUsages::None);
    ASH_CHECK(buffer.desc.size > 0);
    ASH_CHECK(buffer.desc.properties != MemoryProperties::None);
    ASH_CHECK(graph.ctx.device_info.memory_heaps.has_memory(buffer.desc.properties));
  }

  for (ImageState const &image : graph.images)
  {
    ASH_CHECK(image.desc.extent.is_visible());
    ASH_CHECK(image.desc.usages != ImageUsages::None);
    ASH_CHECK(image.desc.mips >= 1);
    ASH_CHECK(image.desc.format != Format::Undefined);
    ASH_CHECK(image.desc.aspects != ImageAspects::None);
  }

  for (ImageViewDesc const &image_view : graph.image_views)
  {
    ASH_CHECK(image_view.aspects != ImageAspects::None);
    ASH_CHECK(image_view.view_format != Format::Undefined);
    ASH_CHECK(graph.images.is_valid(image_view.image));
    ASH_CHECK(image_view.num_mip_levels >= 1);
    ImageDesc const resource = graph.get_desc(image_view.image);
    ASH_CHECK((resource.aspects | image_view.aspects) != ImageAspects::None);
    ASH_CHECK(image_view.first_mip_level < resource.mips);
    ASH_CHECK((image_view.first_mip_level + image_view.num_mip_levels) <= resource.mips);
  }

  // for (RenderPassDesc const &render_pass : graph.render_passes)
  // {
  //   for (RenderPassAttachment const &attachment : render_pass.color_attachments)
  //   {
  //     ASH_CHECK(attachment.format != Format::Undefined);
  //   }
  //   for (RenderPassAttachment const &attachment : render_pass.depth_stencil_attachments)
  //   {
  //     ASH_CHECK(attachment.format != Format::Undefined);
  //   }
  // }

  // // check buffer views

  // for (FramebufferDesc const &framebuffer : graph.framebuffers)
  // {
  //   ASH_CHECK(graph.render_passes.is_valid(framebuffer.renderpass));
  //   ASH_CHECK(framebuffer.extent.is_visible());
  //   RenderPassDesc const render_pass_desc = graph.get_desc(framebuffer.renderpass);
  //   ASH_CHECK(framebuffer.color_attachments.size() == render_pass_desc.color_attachments.size());
  //   ASH_CHECK(framebuffer.depth_stencil_attachments.size() ==
render_pass_desc.depth_stencil_attachments.size());
  //   for (usize i = 0; i < framebuffer.color_attachments.size(); i++)
  //   {
  //     ImageView attachment = framebuffer.color_attachments[i];
  //     ASH_CHECK(graph.image_views.is_valid(attachment));
  //     ImageViewDesc const attachment_desc = graph.get_desc(attachment);
  //     ASH_CHECK((attachment_desc.aspects & ImageAspects::Color) != ImageAspects::None);
  //     ASH_CHECK(attachment_desc.num_mip_levels >= 1);
  //     ASH_CHECK(render_pass_desc.color_attachments[i].format == attachment_desc.view_format);
  //     ImageDesc const image_desc = graph.get_desc(attachment_desc.image);
  //     ASH_CHECK((image_desc.usages & ImageUsages::ColorAttachment) != ImageUsages::None);
  //     ASH_CHECK(image_desc.extent.width >= framebuffer.extent.width);
  //     ASH_CHECK(image_desc.extent.height >= framebuffer.extent.height);
  //   }
  //   for (usize i = 0; i < framebuffer.depth_stencil_attachments.size(); i++)
  //   {
  //     ImageView attachment = framebuffer.depth_stencil_attachments[i];
  //     ASH_CHECK(graph.image_views.is_valid(attachment));
  //     ImageViewDesc const attachment_desc = graph.get_desc(attachment);
  //     ASH_CHECK((attachment_desc.aspects & (ImageAspects::Depth | ImageAspects::Stencil)) !=
ImageAspects::None);
  //     ASH_CHECK(attachment_desc.num_mip_levels >= 1);
  //     ASH_CHECK(render_pass_desc.depth_stencil_attachments[i].format ==
attachment_desc.view_format);
  //     ImageDesc const image_desc = graph.get_desc(attachment_desc.image);
  //     ASH_CHECK((image_desc.usages & ImageUsages::DepthStencilAttachment) != ImageUsages::None);
  //     ASH_CHECK(image_desc.extent.width >= framebuffer.extent.width);
  //     ASH_CHECK(image_desc.extent.height >= framebuffer.extent.height);
  //   }
  // }
}

void CmdValidator::copy_buffer(Graph &graph, Buffer src, Buffer dst, stx::Span<BufferCopy const>
copies)
{
  ASH_CHECK(!copies.is_empty());
  ASH_CHECK(graph.buffers.is_valid(src));
  ASH_CHECK(graph.buffers.is_valid(dst));
  BufferDesc const src_desc = graph.get_desc(src);
  BufferDesc const dst_desc = graph.get_desc(dst);
  ASH_CHECK((src_desc.usages & BufferUsages::TransferSrc) != BufferUsages::None);
  ASH_CHECK((dst_desc.usages & BufferUsages::TransferDst) != BufferUsages::None);
  for (BufferCopy const &copy : copies)
  {
    ASH_CHECK(src_desc.size > copy.src_offset);
    ASH_CHECK(src_desc.size >= (copy.src_offset + copy.size));
    ASH_CHECK(dst_desc.size > copy.dst_offset);
    ASH_CHECK(dst_desc.size >= (copy.dst_offset + copy.size));
  }
}

void CmdValidator::copy_host_buffer(Graph &graph, stx::Span<u8 const> src, Buffer dst,
stx::Span<BufferCopy const> copies)
{
  ASH_CHECK(!copies.is_empty());
  ASH_CHECK(!src.is_empty());
  ASH_CHECK(graph.buffers.is_valid(dst));
  BufferDesc const dst_desc = graph.get_desc(dst);
  ASH_CHECK((dst_desc.usages & BufferUsages::TransferDst) != BufferUsages::None);
  for (BufferCopy const &copy : copies)
  {
    ASH_CHECK(src.size() > copy.src_offset);
    ASH_CHECK(src.size() >= (copy.src_offset + copy.size));
    ASH_CHECK(dst_desc.size > copy.dst_offset);
    ASH_CHECK(dst_desc.size >= (copy.dst_offset + copy.size));
  }
}

void CmdValidator::copy_image(Graph &graph, Image src, Image dst, stx::Span<ImageCopy const> copies)
{
  ASH_CHECK(!copies.is_empty());
  ASH_CHECK(src != Image::None);
  ASH_CHECK(graph.images.is_valid(src));
  ASH_CHECK(graph.images.is_valid(dst));
  ImageDesc const src_desc = graph.get_desc(src);
  ImageDesc const dst_desc = graph.get_desc(dst);
  ASH_CHECK((src_desc.usages & ImageUsages::TransferSrc) != ImageUsages::None);
  ASH_CHECK((dst_desc.usages & ImageUsages::TransferDst) != ImageUsages::None);

  for (ImageCopy const &copy : copies)
  {
    ASH_CHECK(copy.src_aspects != ImageAspects::None);
    ASH_CHECK(copy.dst_aspects != ImageAspects::None);
    ASH_CHECK((copy.src_aspects | src_desc.aspects) != ImageAspects::None);
    ASH_CHECK((copy.dst_aspects | dst_desc.aspects) != ImageAspects::None);
    ASH_CHECK(copy.src_mip_level < src_desc.mips);
    ASH_CHECK(copy.dst_mip_level < dst_desc.mips);
    // ASH_CHECK((URect{.offset = {}, .extent = src_desc.extent}.contains(copy.src_area)));
    // ASH_CHECK((URect{.offset = {}, .extent =
dst_desc.extent}.contains(copy.src_area.with_offset(copy.dst_offset))));
  }
}

void CmdValidator::copy_buffer_to_image(Graph &graph, Buffer src, Image dst,
stx::Span<BufferImageCopy const> copies)
{
  ASH_CHECK(!copies.is_empty());
  ASH_CHECK(graph.buffers.is_valid(src));
  ASH_CHECK(graph.images.is_valid(dst));
  BufferDesc const src_desc = graph.get_desc(src);
  ImageDesc const  dst_desc = graph.get_desc(dst);
  ASH_CHECK((src_desc.usages & BufferUsages::TransferSrc) != BufferUsages::None);
  ASH_CHECK((dst_desc.usages & ImageUsages::TransferDst) != ImageUsages::None);
  for (BufferImageCopy const &copy : copies)
  {
    ASH_CHECK(copy.buffer_image_height > 0);
    ASH_CHECK(copy.buffer_row_length > 0);
    ASH_CHECK(copy.buffer_offset < src_desc.size);
    ASH_CHECK(copy.image_mip_level < dst_desc.mips);
    ASH_CHECK(copy.image_aspects != ImageAspects::None);
    ASH_CHECK((copy.image_aspects | dst_desc.aspects) != ImageAspects::None);
    // ASH_CHECK((URect{.offset = {}, .extent = dst_desc.extent}.contains(copy.image_area)));
  }
}

void CmdValidator::blit_image(Graph &graph, Image src, Image dst, stx::Span<ImageBlit const> blits,
Filter filter)
{
  ASH_CHECK(!blits.is_empty());
  ASH_CHECK(graph.images.is_valid(src));
  ASH_CHECK(graph.images.is_valid(dst));
  ImageDesc const src_desc = graph.get_desc(src);
  ImageDesc const dst_desc = graph.get_desc(dst);
  ASH_CHECK((src_desc.usages & ImageUsages::TransferSrc) != ImageUsages::None);
  ASH_CHECK((dst_desc.usages & ImageUsages::TransferDst) != ImageUsages::None);
  for (ImageBlit const &blit : blits)
  {
    ASH_CHECK(blit.src_aspects != ImageAspects::None);
    ASH_CHECK(blit.dst_aspects != ImageAspects::None);
    ASH_CHECK((blit.src_aspects | src_desc.aspects) != ImageAspects::None);
    ASH_CHECK((blit.dst_aspects | dst_desc.aspects) != ImageAspects::None);
    ASH_CHECK(blit.src_mip_level < src_desc.mips);
    ASH_CHECK(blit.dst_mip_level < dst_desc.mips);
    // ASH_CHECK((URect{.offset = {}, .extent = src_desc.extent}.contains(blit.src_area)));
    // ASH_CHECK((URect{.offset = {}, .extent = dst_desc.extent}.contains(blit.dst_area)));
  }
}

void CmdValidator::begin_render_pass(Graph &graph, Framebuffer framebuffer, RenderPass render_pass,
IRect render_area, stx::Span<Color const> color_attachments_clear_values, stx::Span<DepthStencil
const> depth_stencil_attachments_clear_values)
{
  ASH_CHECK(graph.render_passes.is_valid(render_pass));
  ASH_CHECK(graph.framebuffers.is_valid(framebuffer));
  FramebufferDesc const framebuffer_desc = graph.get_desc(framebuffer);
  // ASH_CHECK(framebuffer_desc.color_attachments.size() == color_attachments_clear_values.size());
  // ASH_CHECK(framebuffer_desc.depth_stencil_attachments.size() ==
depth_stencil_attachments_clear_values.size());
  // TODO(lamarrr): check renderpass compatibility
}

void CmdValidator::end_render_pass(Graph &graph)
{}

void CmdValidator::push_constants(Graph &graph, stx::Span<u8 const> constants)
{
  ASH_CHECK(constants.size_bytes() <= 128);
}

void CmdValidator::bind_compute_pipeline(Graph &graph, ComputePipeline pipeline)
{
}

void CmdValidator::bind_graphics_pipeline(Graph &graph, GraphicsPipeline pipeline)
{
}

void CmdValidator::bind_vertex_buffers(Graph &graph, stx::Span<Buffer const> vertex_buffers,
stx::Span<u64 const> vertex_buffer_offsets, Buffer index_buffer, u64 index_buffer_offset)
{}

void CmdValidator::set_scissor(Graph &graph, IRect scissor)
{}

void CmdValidator::set_viewport(Graph &graph, Viewport viewport)
{}

void CmdValidator::compute(Graph &graph, u32 base_group_x, u32 group_count_x, u32 base_group_y, u32
group_count_y, u32 base_group_z, u32 group_count_z)
{}

void CmdValidator::compute_indirect(Graph &graph, Buffer buffer, u64 offset)
{}

void CmdValidator::draw_indexed(Graph &graph, u32 index_count, u32 instance_count, u32 first_index,
i32 vertex_offset, u32 first_instance)
{
}

void CmdValidator::draw_indexed_indirect(Graph &graph, Buffer buffer, u64 offset, u32 draw_count,
u32 stride)
{}

void CmdBarrierGenerator::copy_buffer(Graph &graph, BarrierInserter &inserter, Buffer src, Buffer
dst, stx::Span<BufferCopy const> copies)
{
  BufferState &src_state = graph.get_state(src);
  BufferState &dst_state = graph.get_state(dst);

  BufferMemoryBarrier src_barrier{.buffer          = src,
                                       .offset          = 0,
                                       .size            = src_state.desc.size,
                                       .src_stage_mask  = src_state.stages,
                                       .dst_stage_mask  = PipelineStages::Transfer,
                                       .src_access_mask = src_state.access_mask,
                                       .dst_access_mask = Access::TransferRead};
  BufferMemoryBarrier dst_barrier{.buffer          = dst,
                                       .offset          = 0,
                                       .size            = dst_state.desc.size,
                                       .src_stage_mask  = dst_state.stages,
                                       .dst_stage_mask  = PipelineStages::Transfer,
                                       .src_access_mask = dst_state.access_mask,
                                       .dst_access_mask = Access::TransferRead};

  inserter.insert(src_barrier);
  inserter.insert(dst_barrier);

  src_state.access_mask = Access::TransferRead;
  dst_state.access_mask = Access::TransferWrite;
  src_state.stages      = PipelineStages::Transfer;
  dst_state.stages      = PipelineStages::Transfer;
}

void CmdBarrierGenerator::copy_host_buffer(Graph &graph, BarrierInserter &inserter, stx::Span<u8
const> src, Buffer dst, stx::Span<BufferCopy const> copies)
{
  BufferState &dst_state = graph.get_state(dst);

  BufferMemoryBarrier dst_barrier{.buffer          = dst,
                                       .offset          = 0,
                                       .size            = dst_state.desc.size,
                                       .src_stage_mask  = dst_state.stages,
                                       .dst_stage_mask  = PipelineStages::Transfer,
                                       .src_access_mask = dst_state.access_mask,
                                       .dst_access_mask = Access::TransferRead};

  inserter.insert(dst_barrier);

  dst_state.access_mask = Access::TransferWrite;
  dst_state.stages      = PipelineStages::Transfer;
}

void CmdBarrierGenerator::copy_image(Graph &graph, BarrierInserter &inserter, Image src, Image dst,
stx::Span<ImageCopy const> copies)
{
  ImageState &src_state = graph.get_state(src);
  ImageState &dst_state = graph.get_state(dst);

  ImageMemoryBarrier src_barrier{.image           = src,
                                      .first_mip_level = 0,
                                      .num_mip_levels  = REMAINING_MIP_LEVELS,
                                      .aspects         = src_state.desc.aspects,
                                      .old_layout      = src_state.layout,
                                      .new_layout      = ImageLayout::TransferSrcOptimal,
                                      .src_stage_mask  = src_state.stages,
                                      .dst_stage_mask  = PipelineStages::Transfer,
                                      .src_access_mask = src_state.access_mask,
                                      .dst_access_mask = Access::TransferRead};
  ImageMemoryBarrier dst_barrier{.image           = dst,
                                      .first_mip_level = 0,
                                      .num_mip_levels  = REMAINING_MIP_LEVELS,
                                      .aspects         = dst_state.desc.aspects,
                                      .old_layout      = dst_state.layout,
                                      .new_layout      = ImageLayout::TransferDstOptimal,
                                      .src_stage_mask  = dst_state.stages,
                                      .dst_stage_mask  = PipelineStages::Transfer,
                                      .src_access_mask = dst_state.access_mask,
                                      .dst_access_mask = Access::TransferWrite};

  inserter.insert(src_barrier);
  inserter.insert(dst_barrier);

  src_state.access_mask = Access::TransferRead;
  dst_state.access_mask = Access::TransferWrite;
  src_state.stages      = PipelineStages::Transfer;
  dst_state.stages      = PipelineStages::Transfer;
  src_state.layout      = ImageLayout::TransferSrcOptimal;
  dst_state.layout      = ImageLayout::TransferDstOptimal;
}

void CmdBarrierGenerator::copy_buffer_to_image(Graph &graph, BarrierInserter &inserter, Buffer src,
Image dst, stx::Span<BufferImageCopy const> copies)
{
  // get latest expected state
  // convert to transfer dst layout and transfer write access with transfer stage
  // leave as-is. the next usage should conver it back to how it is needed if necessary
  BufferState &src_state = graph.get_state(src);
  ImageState  &dst_state = graph.get_state(dst);

  BufferMemoryBarrier src_barrier{.buffer          = src,
                                       .offset          = 0,
                                       .size            = WHOLE_SIZE,
                                       .src_stage_mask  = src_state.stages,
                                       .dst_stage_mask  = PipelineStages::Transfer,
                                       .src_access_mask = src_state.access_mask,
                                       .dst_access_mask = Access::TransferRead};
  ImageMemoryBarrier  dst_barrier{.image           = dst,
                                       .first_mip_level = 0,
                                       .num_mip_levels  = REMAINING_MIP_LEVELS,
                                       .aspects         = dst_state.desc.aspects,
                                       .old_layout      = dst_state.layout,
                                       .new_layout      = ImageLayout::TransferDstOptimal,
                                       .src_stage_mask  = dst_state.stages,
                                       .dst_stage_mask  = PipelineStages::Transfer,
                                       .src_access_mask = dst_state.access_mask,
                                       .dst_access_mask = Access::TransferWrite};

  inserter.insert(src_barrier);
  inserter.insert(dst_barrier);

  src_state.access_mask = Access::TransferRead;
  dst_state.access_mask = Access::TransferWrite;
  src_state.stages      = PipelineStages::Transfer;
  dst_state.stages      = PipelineStages::Transfer;
  dst_state.layout      = ImageLayout::TransferDstOptimal;
}

void CmdBarrierGenerator::blit_image(Graph &graph, BarrierInserter &inserter, Image src, Image dst,
stx::Span<ImageBlit const> blits, Filter filter)
{
  // check RID
  // check current layout and usage, and all accessors
  // check all memory aliasing
  // convert layout to transfer dst and write access with whatever access type is needed for
blitting
  // update barrier tracker
  // on next usage in command buffer, get the last usage and update accordingly
  ImageState &src_state = graph.get_state(src);
  ImageState &dst_state = graph.get_state(dst);

  ImageMemoryBarrier src_barrier{.image           = src,
                                      .first_mip_level = 0,
                                      .num_mip_levels  = REMAINING_MIP_LEVELS,
                                      .aspects         = src_state.desc.aspects,
                                      .old_layout      = src_state.layout,
                                      .new_layout      = ImageLayout::TransferSrcOptimal,
                                      .src_stage_mask  = src_state.stages,
                                      .dst_stage_mask  = PipelineStages::Transfer,
                                      .src_access_mask = src_state.access_mask,
                                      .dst_access_mask = Access::TransferRead};
  ImageMemoryBarrier dst_barrier{.image           = dst,
                                      .first_mip_level = 0,
                                      .num_mip_levels  = REMAINING_MIP_LEVELS,
                                      .aspects         = dst_state.desc.aspects,
                                      .old_layout      = dst_state.layout,
                                      .new_layout      = ImageLayout::TransferDstOptimal,
                                      .src_stage_mask  = dst_state.stages,
                                      .dst_stage_mask  = PipelineStages::Transfer,
                                      .src_access_mask = dst_state.access_mask,
                                      .dst_access_mask = Access::TransferWrite};

  inserter.insert(src_barrier);
  inserter.insert(dst_barrier);

  src_state.access_mask = Access::TransferRead;
  dst_state.access_mask = Access::TransferWrite;
  src_state.stages      = PipelineStages::Transfer;
  dst_state.stages      = PipelineStages::Transfer;
  src_state.layout      = ImageLayout::TransferSrcOptimal;
  dst_state.layout      = ImageLayout::TransferDstOptimal;
}

void CmdBarrierGenerator::begin_render_pass(Graph &graph, BarrierInserter &inserter, Framebuffer
framebuffer, RenderPass render_pass, IRect render_area, stx::Span<Color const>
color_attachments_clear_values, stx::Span<DepthStencil const>
depth_stencil_attachments_clear_values)
{
  // TODO(lamarrr): in-between barriers access??? i.e. in shader?
  //
  // TODO(lamarrr): should we make the renderpass not convert the layouts?
  //
  // FramebufferDesc framebuffer_desc = graph.get_desc(framebuffer);
  // for (ImageView attachment : framebuffer_desc.color_attachments)
  // {
  //   ImageState &state = graph.get_state(graph.get_desc(attachment).image);                 //
TODO(lamarrr): color attachment may not be written to depending on the renderpass ops
  //   state.access_mask = Access::ColorAttachmentRead | Access::ColorAttachmentWrite;        //
LoadOp and StoreOp
  //   state.stages      = PipelineStages::ColorAttachmentOutput |
PipelineStages::EarlyFragmentTests;
  //   state.layout      = ImageLayout::ColorAttachmentOptimal;
  // }

  // for (ImageView attachment : framebuffer_desc.depth_stencil_attachments)
  // {
  //   ImageState &state = graph.get_state(graph.get_desc(attachment).image);
  //   state.access_mask = Access::DepthStencilAttachmentRead | Access::DepthStencilAttachmentWrite;
  //   state.stages      = PipelineStages::ColorAttachmentOutput |
PipelineStages::EarlyFragmentTests;
  //   state.layout      = ImageLayout::DepthStencilAttachmentOptimal;
  // }
}

void CmdBarrierGenerator::end_render_pass(Graph &graph, BarrierInserter &inserter)
{
  // TODO(lamarrr): post-renderpass state transitions
  // TODO(lamarrr): DontCare generates write access to the attachment in renderpasses
}
void CmdBarrierGenerator::bind_compute_pipeline(Graph &graph, BarrierInserter &inserter,
ComputePipeline pipeline)
{}
void CmdBarrierGenerator::bind_graphics_pipeline(Graph &graph, BarrierInserter &inserter,
GraphicsPipeline pipeline)
{}
void CmdBarrierGenerator::bind_vertex_buffers(Graph &graph, BarrierInserter &inserter,
stx::Span<Buffer const> vertex_buffers, stx::Span<u64 const> vertex_buffer_offsets, Buffer
index_buffer, u64 index_buffer_offset)
{}
// TODO(lamarrr): remove unnecessary barriers: i.e. newly created resources with None access and
None states. and double-reads
//
// some scenarios reset the state of the barriers i.e. when we wait for fences on a swapchain, all
resources in that frame would not be in use
//
// accumulate states until they are no longer needed
// use previous frame's states and barriers states
// detect unused resources
// insert fences, barriers, and whatnot
// smart aliasing image memory barriers with aliasing will enable us to perform better
// store current usage so the next usage will know how to access
//
// render passes perform layout and transitions neccessary

PipelineStages to_pipeline_stage(ShaderStages stages)
{
  PipelineStages out = PipelineStages::None;

  if ((stages & ShaderStages::Vertex) != ShaderStages::None)
  {
    out |= PipelineStages::VertexShader;
  }

  if ((stages & ShaderStages::Geometry) != ShaderStages::None)
  {
    out |= PipelineStages::GeometryShader;
  }

  if ((stages & ShaderStages::Fragment) != ShaderStages::None)
  {
    out |= PipelineStages::FragmentShader;
  }

  if ((stages & ShaderStages::Compute) != ShaderStages::None)
  {
    out |= PipelineStages::ComputeShader;
  }

  if ((stages & ShaderStages::AllGraphics) != ShaderStages::None)
  {
    out |= PipelineStages::AllGraphics;
  }

  if ((stages & ShaderStages::All) != ShaderStages::None)
  {
    out |= PipelineStages::AllCommands;
  }

  return out;
}

void CmdBarrierGenerator::set_bind_group(Graph &graph, BarrierInserter &inserter, BindGroup
bind_group)
{
  BindGroupDesc const &desc = graph.get_desc(bind_group);
  for (u32 i = 0; i < desc.num_bindings; i++)
  {
    DescriptorBinding const &binding = desc.bindings[i];
    BindGroupEntry const     entry;        // TODO(lamarrr)   = desc.layout[i];
    PipelineStages           pipeline_stages = to_pipeline_stage(entry.stages);
    switch (binding.type)
    {
      case DescriptorType::CombinedImageSampler:
      {
        Image            image = graph.get_desc(binding.combined_image_sampler.image_view).image;
        ImageDesc const &desc  = graph.get_desc(image);
        ImageState      &state = graph.get_state(image);

        inserter.insert(ImageMemoryBarrier{
            .image           = image,
            .first_mip_level = 0,
            .num_mip_levels  = REMAINING_MIP_LEVELS,
            .aspects         = desc.aspects,
            .old_layout      = state.layout,
            .new_layout      = ImageLayout::ShaderReadOnlyOptimal,
            .src_stage_mask  = state.stages,
            .dst_stage_mask  = pipeline_stages,
            .src_access_mask = state.access_mask,
            .dst_access_mask = Access::ShaderSampledRead});
      }
      break;

      case DescriptorType::InputAttachment:
      {
      }
      break;

      case DescriptorType::SampledImage:
      {
        Image            image = graph.get_desc(binding.sampled_image.image_view).image;
        ImageDesc const &desc  = graph.get_desc(image);
        ImageState      &state = graph.get_state(image);

        inserter.insert(ImageMemoryBarrier{
            .image           = image,
            .first_mip_level = 0,
            .num_mip_levels  = REMAINING_MIP_LEVELS,
            .aspects         = desc.aspects,
            .old_layout      = state.layout,
            .new_layout      = ImageLayout::ShaderReadOnlyOptimal,
            .src_stage_mask  = state.stages,
            .dst_stage_mask  = pipeline_stages,
            .src_access_mask = state.access_mask,
            .dst_access_mask = Access::ShaderSampledRead});
      }
      break;

      case DescriptorType::Sampler:
      {
      }
      break;

      case DescriptorType::StorageBuffer:
      {
        BufferState &state = graph.get_state(binding.storage_buffer.buffer);

        inserter.insert(BufferMemoryBarrier{
            .buffer          = binding.storage_buffer.buffer,
            .offset          = 0,
            .size            = WHOLE_SIZE,
            .src_stage_mask  = state.stages,
            .dst_stage_mask  = pipeline_stages,
            .src_access_mask = state.access_mask,
            .dst_access_mask = Access::ShaderStorageWrite});
      }
      break;

      case DescriptorType::StorageImage:
      {
        Image            image = graph.get_desc(binding.storage_image.image_view).image;
        ImageDesc const &desc  = graph.get_desc(image);
        ImageState      &state = graph.get_state(image);

        inserter.insert(ImageMemoryBarrier{
            .image           = image,
            .first_mip_level = 0,
            .num_mip_levels  = REMAINING_MIP_LEVELS,
            .aspects         = desc.aspects,
            .old_layout      = state.layout,
            .new_layout      = ImageLayout::General,
            .src_stage_mask  = state.stages,
            .dst_stage_mask  = pipeline_stages,
            .src_access_mask = state.access_mask,
            .dst_access_mask = Access::ShaderStorageWrite});
      }
      break;

      case DescriptorType::StorageTexelBuffer:
      {
        BufferViewDesc const &subdesc = graph.get_desc(binding.storage_texel_buffer.buffer_view);
        BufferDesc const     &desc    = graph.get_desc(subdesc.buffer);
        BufferState          &state   = graph.get_state(subdesc.buffer);

        inserter.insert(BufferMemoryBarrier{
            .buffer          = subdesc.buffer,
            .offset          = 0,
            .size            = WHOLE_SIZE,
            .src_stage_mask  = state.stages,
            .dst_stage_mask  = pipeline_stages,
            .src_access_mask = state.access_mask,
            .dst_access_mask = Access::ShaderStorageWrite});
      }
      break;

      case DescriptorType::UniformBuffer:
      {
        BufferDesc const &desc  = graph.get_desc(binding.uniform_buffer.buffer);
        BufferState      &state = graph.get_state(binding.uniform_buffer.buffer);

        inserter.insert(BufferMemoryBarrier{
            .buffer          = binding.uniform_buffer.buffer,
            .offset          = 0,
            .size            = WHOLE_SIZE,
            .src_stage_mask  = state.stages,
            .dst_stage_mask  = pipeline_stages,
            .src_access_mask = state.access_mask,
            .dst_access_mask = Access::UniformRead});
      }
      break;

      case DescriptorType::UniformTexelBuffer:
      {
        BufferViewDesc const &subdesc = graph.get_desc(binding.uniform_texel_buffer.buffer_view);
        BufferDesc const     &desc    = graph.get_desc(subdesc.buffer);
        BufferState          &state   = graph.get_state(subdesc.buffer);

        inserter.insert(BufferMemoryBarrier{
            .buffer          = binding.uniform_buffer.buffer,
            .offset          = 0,
            .size            = WHOLE_SIZE,
            .src_stage_mask  = state.stages,
            .dst_stage_mask  = pipeline_stages,
            .src_access_mask = state.access_mask,
            .dst_access_mask = Access::UniformRead});
      }
      break;

      default:
        break;
    }
  }
}
void CmdBarrierGenerator::compute(Graph &graph, BarrierInserter &inserter, u32 base_group_x, u32
group_count_x, u32 base_group_y, u32 group_count_y, u32 base_group_z, u32 group_count_z)
{}

void CmdBarrierGenerator::compute_indirect(Graph &graph, BarrierInserter &inserter, Buffer buffer,
u64 offset)
{}

void CmdBarrierGenerator::draw_indexed(Graph &graph, BarrierInserter &inserter, u32 index_count, u32
instance_count, u32 first_index, i32 vertex_offset, u32 first_instance)
{}

void CmdBarrierGenerator::draw_indexed_indirect(Graph &graph, BarrierInserter &inserter, Buffer
buffer, u64 offset, u32 draw_count, u32 stride)
{}
*/

}        // namespace rcg
}        // namespace ash
