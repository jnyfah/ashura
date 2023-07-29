#include "SDL3/SDL.h"
#include "ashura/app.h"
#include "ashura/text.h"
#include "ashura/uuid.h"
#include "ashura/widget.h"
#include "ashura/widget_tree.h"
#include "ashura/widgets/box.h"
#include "ashura/widgets/checkbox.h"
#include "ashura/widgets/flex.h"
#include "ashura/widgets/image.h"
#include "ashura/widgets/radio.h"
#include "ashura/widgets/slider.h"
#include "ashura/widgets/stats.h"
#include "ashura/widgets/text.h"
#include "stx/try_some.h"

using namespace ash;

int main(int argc, char **argv)
{
  ASH_CHECK(SDL_Init(SDL_INIT_EVERYTHING) == 0);

  FontSpec fonts[] = {{.name = "Roboto", .path = R"(C:\Users\Basit\Documents\workspace\oss\ashura\ashura\assets\fonts\Roboto\Roboto-Regular.ttf)"},
                      {.name = "RobotoMono", .path = R"(C:\Users\Basit\Desktop\JetBrainsMono-2.304\fonts\ttf\JetBrainsMono-Regular.ttf)"},
                      {.name = "MaterialIcons", .path = R"(C:\Users\Basit\Documents\workspace\oss\ashura\ashura\assets\fonts\MaterialIcons\MaterialIcons-Regular.ttf)"},
                      {.name = "IBMPlexSans", .path = R"(C:\Users\Basit\Desktop\IBM_Plex_Sans_Arabic\IBMPlexSansArabic-Regular.ttf)"}};

  CanvasPipelineSpec pipelines[] = {
      {.name = DEFAULT_SHAPE_PIPELINE, .vertex_shader = gfx::vertex_shader_code, .fragment_shader = gfx::fragment_shader_code},
      {.name = DEFAULT_GLYPH_PIPELINE, .vertex_shader = gfx::glyph_vertex_shader_code, .fragment_shader = gfx::glyph_fragment_shader_code}};

  AppConfig cfg{.enable_validation_layers = true, .fonts = fonts, .pipelines = pipelines};

  // TODO(lamarrr): slider layout reflow
  char const text[] = R"(يَـٰٓأَيُّهَ ١)يَـٰٓأَيُّهَ ١)يَـٰٓأَيُّهَ ١)يَـٰٓأَيُّهَ ١)يَـٰٓأَيُّهَ ١)يَـٰٓأَيُّهَ ١)يَـٰٓأَيُّهَ ١)يَـٰٓأَيُّهَ ١)يَـٰٓأَيُّهَ ١)يَـٰٓأَيُّهَ ١)يَـٰٓأَيُّهَ ١)يَـٰٓأَيُّهَ ١)يَـٰٓأَيُّهَ ١)يَـٰٓأَيُّهَ ١)يَـٰٓأَيُّهَ ١)يَـٰٓأَيُّهَ ١)يَـٰٓأَيُّهَ
   ١)يَـٰٓأَيُّهَ ١)يَـٰٓأَيُّهَ ١)يَـٰٓأَيُّهَ ١)يَـٰٓأَيُّهَ ١)يَـٰٓأَيُّهَ ١)يَـٰٓأَيُّهَ ١)يَـٰٓأَيُّهَ
    ١)يَـٰٓأَيُّهَ ١)يَـٰٓأَيُّهَ ١)يَـٰٓأَيُّهَ ١)يَـٰٓأَيُّهَ ١)يَـٰٓأَيُّهَ ١)يَـٰٓأَيُّهَ ١)يَـٰٓأَيُّهَ ١)يَـٰٓأَيُّهَ ١)يَـٰٓأَيُّهَ ١)يَـٰٓأَيُّهَ ١)";
  RadioState state{8};
  App        app{std::move(cfg),
          Flex{
              FlexProps{},
              Image{ImageProps{.source = FileImageSource{.path = R"(C:\Users\Basit\Desktop\pimping.png)"}, .aspect_ratio = stx::Some(1.0f), .resize_on_load = true}},
              Text{"cruelty_free", TextStyle{.font = "MaterialIcons", .font_height = 25, .foreground_color = material::BLACK, .background_color = colors::WHITE, .line_height = 1.0f}},
              Text{std::string_view{(char *) text, sizeof(text)}, TextStyle{.font = "IBMPlexSans", .font_height = 20, .foreground_color = material::BLACK, .background_color = colors::WHITE}},
              CheckBox{},
              Slider{stx::fn::rc::make_static([](Slider &slider, Context &ctx, f32 value) {
                ctx.text_scale_factor = value * 5;
              })},
              StatsWidget{},
              /* Box{BoxProps{.width            = constraint::absolute(200),
                                   .height           = constraint::absolute(200),
                                   .background_color = colors::WHITE,
                                   .border_thickness = 2,
                                   .border_color     = colors::GREEN,
                                   .border_radius    = {5, 5, 5, 5}},
                   Text{"Click Me!", TextStyle{.foreground_color = colors::RED}}},*/
              Radio(5, state),
              Radio(6, state),
              Radio(8, state),
              Text{"verified", TextStyle{.font = "MaterialIcons", .foreground_color = colors::YELLOW}},
              Text{R"(I didn't wanna say anything, but this game seems lame)", TextStyle{.font = "Roboto", .font_height = 30, .foreground_color = material::BLUE_500, .background_color = material::GRAY_100}},
              Text{R"(I didn't wanna say anything, but this game seems lame)", TextStyle{.font = "Roboto", .font_height = 30, .foreground_color = material::BLUE_500, .background_color = material::GRAY_100}},
              Text{"explicit", TextStyle{.font = "MaterialIcons", .foreground_color = colors::GREEN}},
              Image{ImageProps{.source = FileImageSource{.path = R"(C:\Users\Basit\Desktop\wallpaperflare.com_wallpaper.jpg)"}, .border_radius = vec4{20, 20, 20, 20}, .aspect_ratio = stx::Some(2.0f), .resize_on_load = true}}}};

  timepoint last_tick = Clock::now();
  while (true)
  {
    timepoint present = Clock::now();
    app.tick(present - last_tick);
    last_tick = present;
  }

  SDL_Quit();
  return 0;
}
