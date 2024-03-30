#pragma once

#include "ashura/renderer/renderer.h"
#include "ashura/std/types.h"

namespace ash
{

enum class ShaderType : u8
{
  Compute  = 0,
  Vertex   = 1,
  Fragment = 2,
  Mesh     = 3
};

// how to merge error types?
enum class ShaderCompileError : i32
{
  None                  = 0,
  OutOfMemory           = 1,
  IOError               = 2,
  CompileFailed         = 3,
  LinkFailed            = 4,
  SpirvConversionFailed = 5,
};

ShaderCompileError
    compile_shader(Logger &logger, Vec<u32> &spirv, Span<char const> file,
                   ShaderType type, Span<char const> preamble,
                   Span<char const>             entry_point,
                   Span<Span<char const> const> system_directories,
                   Span<Span<char const> const> local_directories);

}        // namespace ash
