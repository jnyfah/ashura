#pragma once
#include "ashura/primitives.h"

namespace ash
{
namespace gfx
{

// glslangValidator -V -x shader.vert -o shader.vert.spv
static constexpr u32 const vertex_shader_code[] = {
    0x07230203, 0x00010000, 0x0008000b, 0x00000031, 0x00000000, 0x00020011, 0x00000001, 0x0006000b,
    0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e, 0x00000000, 0x0003000e, 0x00000000, 0x00000001,
    0x000b000f, 0x00000000, 0x00000004, 0x6e69616d, 0x00000000, 0x0000000d, 0x0000001b, 0x0000002a,
    0x0000002b, 0x0000002d, 0x0000002f, 0x00030003, 0x00000002, 0x000001c2, 0x00040005, 0x00000004,
    0x6e69616d, 0x00000000, 0x00060005, 0x0000000b, 0x505f6c67, 0x65567265, 0x78657472, 0x00000000,
    0x00060006, 0x0000000b, 0x00000000, 0x505f6c67, 0x7469736f, 0x006e6f69, 0x00070006, 0x0000000b,
    0x00000001, 0x505f6c67, 0x746e696f, 0x657a6953, 0x00000000, 0x00070006, 0x0000000b, 0x00000002,
    0x435f6c67, 0x4470696c, 0x61747369, 0x0065636e, 0x00070006, 0x0000000b, 0x00000003, 0x435f6c67,
    0x446c6c75, 0x61747369, 0x0065636e, 0x00030005, 0x0000000d, 0x00000000, 0x00060005, 0x00000012,
    0x68737550, 0x736e6f43, 0x746e6174, 0x00000000, 0x00060006, 0x00000012, 0x00000000, 0x6e617274,
    0x726f6673, 0x0000006d, 0x00060005, 0x00000014, 0x68737570, 0x6e6f635f, 0x6e617473, 0x00000074,
    0x00050005, 0x0000001b, 0x705f6e69, 0x7469736f, 0x006e6f69, 0x00040005, 0x0000002a, 0x5f74756f,
    0x00007675, 0x00040005, 0x0000002b, 0x755f6e69, 0x00000076, 0x00050005, 0x0000002d, 0x5f74756f,
    0x6f6c6f63, 0x00000072, 0x00050005, 0x0000002f, 0x635f6e69, 0x726f6c6f, 0x00000000, 0x00050048,
    0x0000000b, 0x00000000, 0x0000000b, 0x00000000, 0x00050048, 0x0000000b, 0x00000001, 0x0000000b,
    0x00000001, 0x00050048, 0x0000000b, 0x00000002, 0x0000000b, 0x00000003, 0x00050048, 0x0000000b,
    0x00000003, 0x0000000b, 0x00000004, 0x00030047, 0x0000000b, 0x00000002, 0x00040048, 0x00000012,
    0x00000000, 0x00000005, 0x00050048, 0x00000012, 0x00000000, 0x00000023, 0x00000000, 0x00050048,
    0x00000012, 0x00000000, 0x00000007, 0x00000010, 0x00030047, 0x00000012, 0x00000002, 0x00040047,
    0x0000001b, 0x0000001e, 0x00000000, 0x00040047, 0x0000002a, 0x0000001e, 0x00000000, 0x00040047,
    0x0000002b, 0x0000001e, 0x00000001, 0x00040047, 0x0000002d, 0x0000001e, 0x00000001, 0x00040047,
    0x0000002f, 0x0000001e, 0x00000002, 0x00020013, 0x00000002, 0x00030021, 0x00000003, 0x00000002,
    0x00030016, 0x00000006, 0x00000020, 0x00040017, 0x00000007, 0x00000006, 0x00000004, 0x00040015,
    0x00000008, 0x00000020, 0x00000000, 0x0004002b, 0x00000008, 0x00000009, 0x00000001, 0x0004001c,
    0x0000000a, 0x00000006, 0x00000009, 0x0006001e, 0x0000000b, 0x00000007, 0x00000006, 0x0000000a,
    0x0000000a, 0x00040020, 0x0000000c, 0x00000003, 0x0000000b, 0x0004003b, 0x0000000c, 0x0000000d,
    0x00000003, 0x00040015, 0x0000000e, 0x00000020, 0x00000001, 0x0004002b, 0x0000000e, 0x0000000f,
    0x00000000, 0x00040017, 0x00000010, 0x00000006, 0x00000003, 0x00040018, 0x00000011, 0x00000010,
    0x00000003, 0x0003001e, 0x00000012, 0x00000011, 0x00040020, 0x00000013, 0x00000009, 0x00000012,
    0x0004003b, 0x00000013, 0x00000014, 0x00000009, 0x00040020, 0x00000015, 0x00000009, 0x00000011,
    0x00040017, 0x00000019, 0x00000006, 0x00000002, 0x00040020, 0x0000001a, 0x00000001, 0x00000019,
    0x0004003b, 0x0000001a, 0x0000001b, 0x00000001, 0x0004002b, 0x00000006, 0x0000001d, 0x00000000,
    0x0004002b, 0x00000006, 0x00000022, 0x3f800000, 0x00040020, 0x00000027, 0x00000003, 0x00000007,
    0x00040020, 0x00000029, 0x00000003, 0x00000019, 0x0004003b, 0x00000029, 0x0000002a, 0x00000003,
    0x0004003b, 0x0000001a, 0x0000002b, 0x00000001, 0x0004003b, 0x00000027, 0x0000002d, 0x00000003,
    0x00040020, 0x0000002e, 0x00000001, 0x00000007, 0x0004003b, 0x0000002e, 0x0000002f, 0x00000001,
    0x00050036, 0x00000002, 0x00000004, 0x00000000, 0x00000003, 0x000200f8, 0x00000005, 0x00050041,
    0x00000015, 0x00000016, 0x00000014, 0x0000000f, 0x0004003d, 0x00000011, 0x00000017, 0x00000016,
    0x00040054, 0x00000011, 0x00000018, 0x00000017, 0x0004003d, 0x00000019, 0x0000001c, 0x0000001b,
    0x00050051, 0x00000006, 0x0000001e, 0x0000001c, 0x00000000, 0x00050051, 0x00000006, 0x0000001f,
    0x0000001c, 0x00000001, 0x00060050, 0x00000010, 0x00000020, 0x0000001e, 0x0000001f, 0x0000001d,
    0x00050091, 0x00000010, 0x00000021, 0x00000018, 0x00000020, 0x00050051, 0x00000006, 0x00000023,
    0x00000021, 0x00000000, 0x00050051, 0x00000006, 0x00000024, 0x00000021, 0x00000001, 0x00050051,
    0x00000006, 0x00000025, 0x00000021, 0x00000002, 0x00070050, 0x00000007, 0x00000026, 0x00000023,
    0x00000024, 0x00000025, 0x00000022, 0x00050041, 0x00000027, 0x00000028, 0x0000000d, 0x0000000f,
    0x0003003e, 0x00000028, 0x00000026, 0x0004003d, 0x00000019, 0x0000002c, 0x0000002b, 0x0003003e,
    0x0000002a, 0x0000002c, 0x0004003d, 0x00000007, 0x00000030, 0x0000002f, 0x0003003e, 0x0000002d,
    0x00000030, 0x000100fd, 0x00010038};

// glslangValidator -V -x shader.frag -o shader.frag.spv
static constexpr u32 const fragment_shader_code[] = {
    0x07230203, 0x00010000, 0x0008000b, 0x00000018, 0x00000000, 0x00020011, 0x00000001, 0x0006000b,
    0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e, 0x00000000, 0x0003000e, 0x00000000, 0x00000001,
    0x0008000f, 0x00000004, 0x00000004, 0x6e69616d, 0x00000000, 0x00000009, 0x0000000b, 0x00000014,
    0x00030010, 0x00000004, 0x00000007, 0x00030003, 0x00000002, 0x000001c2, 0x00040005, 0x00000004,
    0x6e69616d, 0x00000000, 0x00050005, 0x00000009, 0x5f74756f, 0x6f6c6f63, 0x00000072, 0x00050005,
    0x0000000b, 0x635f6e69, 0x726f6c6f, 0x00000000, 0x00060005, 0x00000010, 0x6e696b73, 0x7865745f,
    0x65727574, 0x00000000, 0x00040005, 0x00000014, 0x755f6e69, 0x00000076, 0x00040047, 0x00000009,
    0x0000001e, 0x00000000, 0x00040047, 0x0000000b, 0x0000001e, 0x00000001, 0x00040047, 0x00000010,
    0x00000022, 0x00000000, 0x00040047, 0x00000010, 0x00000021, 0x00000000, 0x00040047, 0x00000014,
    0x0000001e, 0x00000000, 0x00020013, 0x00000002, 0x00030021, 0x00000003, 0x00000002, 0x00030016,
    0x00000006, 0x00000020, 0x00040017, 0x00000007, 0x00000006, 0x00000004, 0x00040020, 0x00000008,
    0x00000003, 0x00000007, 0x0004003b, 0x00000008, 0x00000009, 0x00000003, 0x00040020, 0x0000000a,
    0x00000001, 0x00000007, 0x0004003b, 0x0000000a, 0x0000000b, 0x00000001, 0x00090019, 0x0000000d,
    0x00000006, 0x00000001, 0x00000000, 0x00000000, 0x00000000, 0x00000001, 0x00000000, 0x0003001b,
    0x0000000e, 0x0000000d, 0x00040020, 0x0000000f, 0x00000000, 0x0000000e, 0x0004003b, 0x0000000f,
    0x00000010, 0x00000000, 0x00040017, 0x00000012, 0x00000006, 0x00000002, 0x00040020, 0x00000013,
    0x00000001, 0x00000012, 0x0004003b, 0x00000013, 0x00000014, 0x00000001, 0x00050036, 0x00000002,
    0x00000004, 0x00000000, 0x00000003, 0x000200f8, 0x00000005, 0x0004003d, 0x00000007, 0x0000000c,
    0x0000000b, 0x0004003d, 0x0000000e, 0x00000011, 0x00000010, 0x0004003d, 0x00000012, 0x00000015,
    0x00000014, 0x00050057, 0x00000007, 0x00000016, 0x00000011, 0x00000015, 0x00050085, 0x00000007,
    0x00000017, 0x0000000c, 0x00000016, 0x0003003e, 0x00000009, 0x00000017, 0x000100fd, 0x00010038};

static constexpr u32 const sdf_glyph_vertex_shader_code[] = {
    0x07230203, 0x00010000, 0x0008000b, 0x0000002d, 0x00000000, 0x00020011, 0x00000001, 0x0006000b,
    0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e, 0x00000000, 0x0003000e, 0x00000000, 0x00000001,
    0x0009000f, 0x00000000, 0x00000004, 0x6e69616d, 0x00000000, 0x0000000d, 0x0000001b, 0x0000002a,
    0x0000002b, 0x00030003, 0x00000002, 0x000001c2, 0x00040005, 0x00000004, 0x6e69616d, 0x00000000,
    0x00060005, 0x0000000b, 0x505f6c67, 0x65567265, 0x78657472, 0x00000000, 0x00060006, 0x0000000b,
    0x00000000, 0x505f6c67, 0x7469736f, 0x006e6f69, 0x00070006, 0x0000000b, 0x00000001, 0x505f6c67,
    0x746e696f, 0x657a6953, 0x00000000, 0x00070006, 0x0000000b, 0x00000002, 0x435f6c67, 0x4470696c,
    0x61747369, 0x0065636e, 0x00070006, 0x0000000b, 0x00000003, 0x435f6c67, 0x446c6c75, 0x61747369,
    0x0065636e, 0x00030005, 0x0000000d, 0x00000000, 0x00050005, 0x00000012, 0x70796c47, 0x6f725068,
    0x00007370, 0x00060006, 0x00000012, 0x00000000, 0x6e617274, 0x726f6673, 0x0000006d, 0x00060006,
    0x00000012, 0x00000001, 0x63696874, 0x73656e6b, 0x00000073, 0x00060006, 0x00000012, 0x00000002,
    0x74666f73, 0x7373656e, 0x00000000, 0x00080006, 0x00000012, 0x00000003, 0x6c74756f, 0x5f656e69,
    0x63696874, 0x73656e6b, 0x00000073, 0x00080006, 0x00000012, 0x00000004, 0x6c74756f, 0x5f656e69,
    0x74666f73, 0x7373656e, 0x00000000, 0x00050006, 0x00000012, 0x00000005, 0x6f6c6f63, 0x00000072,
    0x00070006, 0x00000012, 0x00000006, 0x6c74756f, 0x5f656e69, 0x6f6c6f63, 0x00000072, 0x00040005,
    0x00000014, 0x706f7270, 0x00000073, 0x00050005, 0x0000001b, 0x705f6e69, 0x7469736f, 0x006e6f69,
    0x00040005, 0x0000002a, 0x5f74756f, 0x00007675, 0x00040005, 0x0000002b, 0x755f6e69, 0x00000076,
    0x00050048, 0x0000000b, 0x00000000, 0x0000000b, 0x00000000, 0x00050048, 0x0000000b, 0x00000001,
    0x0000000b, 0x00000001, 0x00050048, 0x0000000b, 0x00000002, 0x0000000b, 0x00000003, 0x00050048,
    0x0000000b, 0x00000003, 0x0000000b, 0x00000004, 0x00030047, 0x0000000b, 0x00000002, 0x00040048,
    0x00000012, 0x00000000, 0x00000005, 0x00050048, 0x00000012, 0x00000000, 0x00000023, 0x00000000,
    0x00050048, 0x00000012, 0x00000000, 0x00000007, 0x00000010, 0x00050048, 0x00000012, 0x00000001,
    0x00000023, 0x00000030, 0x00050048, 0x00000012, 0x00000002, 0x00000023, 0x00000034, 0x00050048,
    0x00000012, 0x00000003, 0x00000023, 0x00000038, 0x00050048, 0x00000012, 0x00000004, 0x00000023,
    0x0000003c, 0x00050048, 0x00000012, 0x00000005, 0x00000023, 0x00000040, 0x00050048, 0x00000012,
    0x00000006, 0x00000023, 0x00000050, 0x00030047, 0x00000012, 0x00000002, 0x00040047, 0x0000001b,
    0x0000001e, 0x00000000, 0x00040047, 0x0000002a, 0x0000001e, 0x00000000, 0x00040047, 0x0000002b,
    0x0000001e, 0x00000001, 0x00020013, 0x00000002, 0x00030021, 0x00000003, 0x00000002, 0x00030016,
    0x00000006, 0x00000020, 0x00040017, 0x00000007, 0x00000006, 0x00000004, 0x00040015, 0x00000008,
    0x00000020, 0x00000000, 0x0004002b, 0x00000008, 0x00000009, 0x00000001, 0x0004001c, 0x0000000a,
    0x00000006, 0x00000009, 0x0006001e, 0x0000000b, 0x00000007, 0x00000006, 0x0000000a, 0x0000000a,
    0x00040020, 0x0000000c, 0x00000003, 0x0000000b, 0x0004003b, 0x0000000c, 0x0000000d, 0x00000003,
    0x00040015, 0x0000000e, 0x00000020, 0x00000001, 0x0004002b, 0x0000000e, 0x0000000f, 0x00000000,
    0x00040017, 0x00000010, 0x00000006, 0x00000003, 0x00040018, 0x00000011, 0x00000010, 0x00000003,
    0x0009001e, 0x00000012, 0x00000011, 0x00000006, 0x00000006, 0x00000006, 0x00000006, 0x00000007,
    0x00000007, 0x00040020, 0x00000013, 0x00000009, 0x00000012, 0x0004003b, 0x00000013, 0x00000014,
    0x00000009, 0x00040020, 0x00000015, 0x00000009, 0x00000011, 0x00040017, 0x00000019, 0x00000006,
    0x00000002, 0x00040020, 0x0000001a, 0x00000001, 0x00000019, 0x0004003b, 0x0000001a, 0x0000001b,
    0x00000001, 0x0004002b, 0x00000006, 0x0000001d, 0x00000000, 0x0004002b, 0x00000006, 0x00000022,
    0x3f800000, 0x00040020, 0x00000027, 0x00000003, 0x00000007, 0x00040020, 0x00000029, 0x00000003,
    0x00000019, 0x0004003b, 0x00000029, 0x0000002a, 0x00000003, 0x0004003b, 0x0000001a, 0x0000002b,
    0x00000001, 0x00050036, 0x00000002, 0x00000004, 0x00000000, 0x00000003, 0x000200f8, 0x00000005,
    0x00050041, 0x00000015, 0x00000016, 0x00000014, 0x0000000f, 0x0004003d, 0x00000011, 0x00000017,
    0x00000016, 0x00040054, 0x00000011, 0x00000018, 0x00000017, 0x0004003d, 0x00000019, 0x0000001c,
    0x0000001b, 0x00050051, 0x00000006, 0x0000001e, 0x0000001c, 0x00000000, 0x00050051, 0x00000006,
    0x0000001f, 0x0000001c, 0x00000001, 0x00060050, 0x00000010, 0x00000020, 0x0000001e, 0x0000001f,
    0x0000001d, 0x00050091, 0x00000010, 0x00000021, 0x00000018, 0x00000020, 0x00050051, 0x00000006,
    0x00000023, 0x00000021, 0x00000000, 0x00050051, 0x00000006, 0x00000024, 0x00000021, 0x00000001,
    0x00050051, 0x00000006, 0x00000025, 0x00000021, 0x00000002, 0x00070050, 0x00000007, 0x00000026,
    0x00000023, 0x00000024, 0x00000025, 0x00000022, 0x00050041, 0x00000027, 0x00000028, 0x0000000d,
    0x0000000f, 0x0003003e, 0x00000028, 0x00000026, 0x0004003d, 0x00000019, 0x0000002c, 0x0000002b,
    0x0003003e, 0x0000002a, 0x0000002c, 0x000100fd, 0x00010038};

static constexpr u32 const sdf_glyph_fragment_shader_code[] = {
    0x07230203, 0x00010000, 0x0008000b, 0x00000052, 0x00000000, 0x00020011, 0x00000001, 0x0006000b,
    0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e, 0x00000000, 0x0003000e, 0x00000000, 0x00000001,
    0x0007000f, 0x00000004, 0x00000004, 0x6e69616d, 0x00000000, 0x00000010, 0x00000040, 0x00030010,
    0x00000004, 0x00000007, 0x00030003, 0x00000002, 0x000001c2, 0x00040005, 0x00000004, 0x6e69616d,
    0x00000000, 0x00060005, 0x00000008, 0x6e676973, 0x645f6465, 0x61747369, 0x0065636e, 0x00050005,
    0x0000000c, 0x5f666473, 0x74786574, 0x00657275, 0x00040005, 0x00000010, 0x755f6e69, 0x00000076,
    0x00040005, 0x00000017, 0x68706c61, 0x00000061, 0x00050005, 0x0000001b, 0x70796c47, 0x6f725068,
    0x00007370, 0x00060006, 0x0000001b, 0x00000000, 0x6e617274, 0x726f6673, 0x0000006d, 0x00060006,
    0x0000001b, 0x00000001, 0x63696874, 0x73656e6b, 0x00000073, 0x00060006, 0x0000001b, 0x00000002,
    0x74666f73, 0x7373656e, 0x00000000, 0x00080006, 0x0000001b, 0x00000003, 0x6c74756f, 0x5f656e69,
    0x63696874, 0x73656e6b, 0x00000073, 0x00080006, 0x0000001b, 0x00000004, 0x6c74756f, 0x5f656e69,
    0x74666f73, 0x7373656e, 0x00000000, 0x00050006, 0x0000001b, 0x00000005, 0x6f6c6f63, 0x00000072,
    0x00070006, 0x0000001b, 0x00000006, 0x6c74756f, 0x5f656e69, 0x6f6c6f63, 0x00000072, 0x00040005,
    0x0000001d, 0x706f7270, 0x00000073, 0x00040005, 0x00000030, 0x6c74756f, 0x00656e69, 0x00050005,
    0x00000040, 0x5f74756f, 0x6f6c6f63, 0x00000072, 0x00040047, 0x0000000c, 0x00000022, 0x00000000,
    0x00040047, 0x0000000c, 0x00000021, 0x00000000, 0x00040047, 0x00000010, 0x0000001e, 0x00000000,
    0x00040048, 0x0000001b, 0x00000000, 0x00000005, 0x00050048, 0x0000001b, 0x00000000, 0x00000023,
    0x00000000, 0x00050048, 0x0000001b, 0x00000000, 0x00000007, 0x00000010, 0x00050048, 0x0000001b,
    0x00000001, 0x00000023, 0x00000030, 0x00050048, 0x0000001b, 0x00000002, 0x00000023, 0x00000034,
    0x00050048, 0x0000001b, 0x00000003, 0x00000023, 0x00000038, 0x00050048, 0x0000001b, 0x00000004,
    0x00000023, 0x0000003c, 0x00050048, 0x0000001b, 0x00000005, 0x00000023, 0x00000040, 0x00050048,
    0x0000001b, 0x00000006, 0x00000023, 0x00000050, 0x00030047, 0x0000001b, 0x00000002, 0x00040047,
    0x00000040, 0x0000001e, 0x00000000, 0x00020013, 0x00000002, 0x00030021, 0x00000003, 0x00000002,
    0x00030016, 0x00000006, 0x00000020, 0x00040020, 0x00000007, 0x00000007, 0x00000006, 0x00090019,
    0x00000009, 0x00000006, 0x00000001, 0x00000000, 0x00000000, 0x00000000, 0x00000001, 0x00000000,
    0x0003001b, 0x0000000a, 0x00000009, 0x00040020, 0x0000000b, 0x00000000, 0x0000000a, 0x0004003b,
    0x0000000b, 0x0000000c, 0x00000000, 0x00040017, 0x0000000e, 0x00000006, 0x00000002, 0x00040020,
    0x0000000f, 0x00000001, 0x0000000e, 0x0004003b, 0x0000000f, 0x00000010, 0x00000001, 0x00040017,
    0x00000012, 0x00000006, 0x00000004, 0x00040015, 0x00000014, 0x00000020, 0x00000000, 0x0004002b,
    0x00000014, 0x00000015, 0x00000000, 0x0004002b, 0x00000006, 0x00000018, 0x3f800000, 0x00040017,
    0x00000019, 0x00000006, 0x00000003, 0x00040018, 0x0000001a, 0x00000019, 0x00000003, 0x0009001e,
    0x0000001b, 0x0000001a, 0x00000006, 0x00000006, 0x00000006, 0x00000006, 0x00000012, 0x00000012,
    0x00040020, 0x0000001c, 0x00000009, 0x0000001b, 0x0004003b, 0x0000001c, 0x0000001d, 0x00000009,
    0x00040015, 0x0000001e, 0x00000020, 0x00000001, 0x0004002b, 0x0000001e, 0x0000001f, 0x00000001,
    0x00040020, 0x00000020, 0x00000009, 0x00000006, 0x0004002b, 0x0000001e, 0x00000024, 0x00000002,
    0x0004002b, 0x0000001e, 0x00000031, 0x00000003, 0x0004002b, 0x0000001e, 0x00000034, 0x00000004,
    0x00040020, 0x0000003f, 0x00000003, 0x00000012, 0x0004003b, 0x0000003f, 0x00000040, 0x00000003,
    0x0004002b, 0x0000001e, 0x00000041, 0x00000006, 0x00040020, 0x00000042, 0x00000009, 0x00000012,
    0x0004002b, 0x0000001e, 0x00000046, 0x00000005, 0x00050036, 0x00000002, 0x00000004, 0x00000000,
    0x00000003, 0x000200f8, 0x00000005, 0x0004003b, 0x00000007, 0x00000008, 0x00000007, 0x0004003b,
    0x00000007, 0x00000017, 0x00000007, 0x0004003b, 0x00000007, 0x00000030, 0x00000007, 0x0004003d,
    0x0000000a, 0x0000000d, 0x0000000c, 0x0004003d, 0x0000000e, 0x00000011, 0x00000010, 0x00050057,
    0x00000012, 0x00000013, 0x0000000d, 0x00000011, 0x00050051, 0x00000006, 0x00000016, 0x00000013,
    0x00000000, 0x0003003e, 0x00000008, 0x00000016, 0x00050041, 0x00000020, 0x00000021, 0x0000001d,
    0x0000001f, 0x0004003d, 0x00000006, 0x00000022, 0x00000021, 0x00050083, 0x00000006, 0x00000023,
    0x00000018, 0x00000022, 0x00050041, 0x00000020, 0x00000025, 0x0000001d, 0x00000024, 0x0004003d,
    0x00000006, 0x00000026, 0x00000025, 0x00050083, 0x00000006, 0x00000027, 0x00000023, 0x00000026,
    0x00050041, 0x00000020, 0x00000028, 0x0000001d, 0x0000001f, 0x0004003d, 0x00000006, 0x00000029,
    0x00000028, 0x00050083, 0x00000006, 0x0000002a, 0x00000018, 0x00000029, 0x00050041, 0x00000020,
    0x0000002b, 0x0000001d, 0x00000024, 0x0004003d, 0x00000006, 0x0000002c, 0x0000002b, 0x00050081,
    0x00000006, 0x0000002d, 0x0000002a, 0x0000002c, 0x0004003d, 0x00000006, 0x0000002e, 0x00000008,
    0x0008000c, 0x00000006, 0x0000002f, 0x00000001, 0x00000031, 0x00000027, 0x0000002d, 0x0000002e,
    0x0003003e, 0x00000017, 0x0000002f, 0x00050041, 0x00000020, 0x00000032, 0x0000001d, 0x00000031,
    0x0004003d, 0x00000006, 0x00000033, 0x00000032, 0x00050041, 0x00000020, 0x00000035, 0x0000001d,
    0x00000034, 0x0004003d, 0x00000006, 0x00000036, 0x00000035, 0x00050083, 0x00000006, 0x00000037,
    0x00000033, 0x00000036, 0x00050041, 0x00000020, 0x00000038, 0x0000001d, 0x00000031, 0x0004003d,
    0x00000006, 0x00000039, 0x00000038, 0x00050041, 0x00000020, 0x0000003a, 0x0000001d, 0x00000034,
    0x0004003d, 0x00000006, 0x0000003b, 0x0000003a, 0x00050081, 0x00000006, 0x0000003c, 0x00000039,
    0x0000003b, 0x0004003d, 0x00000006, 0x0000003d, 0x00000008, 0x0008000c, 0x00000006, 0x0000003e,
    0x00000001, 0x00000031, 0x00000037, 0x0000003c, 0x0000003d, 0x0003003e, 0x00000030, 0x0000003e,
    0x00050041, 0x00000042, 0x00000043, 0x0000001d, 0x00000041, 0x0004003d, 0x00000012, 0x00000044,
    0x00000043, 0x0008004f, 0x00000019, 0x00000045, 0x00000044, 0x00000044, 0x00000000, 0x00000001,
    0x00000002, 0x00050041, 0x00000042, 0x00000047, 0x0000001d, 0x00000046, 0x0004003d, 0x00000012,
    0x00000048, 0x00000047, 0x0008004f, 0x00000019, 0x00000049, 0x00000048, 0x00000048, 0x00000000,
    0x00000001, 0x00000002, 0x0004003d, 0x00000006, 0x0000004a, 0x00000030, 0x00060050, 0x00000019,
    0x0000004b, 0x0000004a, 0x0000004a, 0x0000004a, 0x0008000c, 0x00000019, 0x0000004c, 0x00000001,
    0x0000002e, 0x00000045, 0x00000049, 0x0000004b, 0x0004003d, 0x00000006, 0x0000004d, 0x00000017,
    0x00050051, 0x00000006, 0x0000004e, 0x0000004c, 0x00000000, 0x00050051, 0x00000006, 0x0000004f,
    0x0000004c, 0x00000001, 0x00050051, 0x00000006, 0x00000050, 0x0000004c, 0x00000002, 0x00070050,
    0x00000012, 0x00000051, 0x0000004e, 0x0000004f, 0x00000050, 0x0000004d, 0x0003003e, 0x00000040,
    0x00000051, 0x000100fd, 0x00010038};

}        // namespace gfx
}        // namespace ash