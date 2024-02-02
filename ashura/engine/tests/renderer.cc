#include "ashura/engine/renderer.h"
#include "ashura/engine/widget.h"
#include "ashura/engine/system.h"
#include "gtest/gtest.h"

ash::Logger panic_logger;

TEST(RendererTest, Scene)
{
  ash::Scene        scene{};
  ash::RenderServer server{};
  server.frustum_cull_().unwrap();
  server.transform_();
}
