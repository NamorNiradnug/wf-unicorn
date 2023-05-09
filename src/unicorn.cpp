#include <iostream>

#include <wayfire/option-wrapper.hpp>
#include <wayfire/output.hpp>
#include <wayfire/plugin.hpp>
#include <wayfire/plugins/common/cairo-util.hpp>
#include <wayfire/render-manager.hpp>
#include <wayfire/util/duration.hpp>

#include <librsvg/rsvg.h>

cairo_surface_t *LoadSVG(const char *path, int surface_width, int surface_height)
{
    auto *handle = rsvg_handle_new_from_file(path, nullptr);

    auto *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, surface_width, surface_height);
    auto *cr = cairo_create(surface);

    RsvgRectangle viewport = {
        .x = 0.0,
        .y = 0.0,
        .width = static_cast<double>(surface_width),
        .height = static_cast<double>(surface_height),
    };

    if (handle != nullptr)
    {
        rsvg_handle_render_document(handle, cr, &viewport, nullptr);
        g_object_unref(handle);
    }

    cairo_destroy(cr);

    return surface;
}

class wf_unicorn_t : public wf::plugin_interface_t
{
    wf::option_wrapper_t<wf::activatorbinding_t> launch_unicorn_activator{"wf-unicorn/unicorn_activator"};
    wf::option_wrapper_t<std::string> unicorn_svg_path{"wf-unicorn/unicorn_path"};
    wf::option_wrapper_t<int> unicorn_size{"wf-unicorn/unicorn_size"};

    wf::animation::simple_animation_t unicorn_position_animation = wf::animation::simple_animation_t();

    wf::geometry_t unicorn_rect;

    wf::activator_callback toggle_unicorn = [this](auto) noexcept -> bool {
        unicorn_visible = !unicorn_visible;
        if (unicorn_visible)
        {
            if (!output->activate_plugin(grab_interface))
            {
                return false;
            }
            if (!hook_set)
            {
                output->render->add_effect(&draw_unicorn, wf::OUTPUT_EFFECT_OVERLAY);
                hook_set = true;
            }
        }
        else
        {
            output->deactivate_plugin(grab_interface);
            if (hook_set)
            {
                output->render->rem_effect(&draw_unicorn);
                hook_set = false;
            }
        }

        output->render->damage(unicorn_rect);
        return true;
    };

    wf::effect_hook_t draw_unicorn = [this]() noexcept {
        const auto frambuffer = output->render->get_target_framebuffer();
        OpenGL::render_begin(frambuffer);
        OpenGL::render_texture(wf::texture_t{unicorn_texture.tex}, frambuffer, unicorn_rect, glm::vec4(1.0f),
                               OpenGL::TEXTURE_TRANSFORM_INVERT_Y);
        OpenGL::render_end();
    };

    void reload_unicorn_texture()
    {
        auto *const unicorn_surface = LoadSVG(unicorn_svg_path.value().c_str(), unicorn_size, unicorn_size);
        OpenGL::render_begin();
        cairo_surface_upload_to_texture(unicorn_surface, unicorn_texture);
        OpenGL::render_end();
        cairo_surface_destroy(unicorn_surface);

        unicorn_rect.width = unicorn_texture.width;
        unicorn_rect.height = unicorn_texture.height;

        if (unicorn_visible && hook_set)
        {
            output->render->damage(unicorn_rect);
        }
    }

    wf::simple_texture_t unicorn_texture;

    bool unicorn_visible = false;
    bool hook_set = false;

  public:
    void init() override
    {
        grab_interface->name = "wf-unicorn";
        grab_interface->capabilities = 0;

        reload_unicorn_texture();

        unicorn_rect.x = 0;
        unicorn_rect.y = 0;

        output->add_activator(launch_unicorn_activator, &toggle_unicorn);

        launch_unicorn_activator.set_callback([this] {
            output->rem_binding(&toggle_unicorn);
            output->add_activator(launch_unicorn_activator, &toggle_unicorn);
        });

        unicorn_svg_path.set_callback([this] { reload_unicorn_texture(); });
        unicorn_size.set_callback([this] { reload_unicorn_texture(); });
    }

    void fini() override
    {
        output->rem_binding(&toggle_unicorn);
        output->render->damage_whole();
    }
};

DECLARE_WAYFIRE_PLUGIN(wf_unicorn_t)
