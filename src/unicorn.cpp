#include <iostream>

#include <wayfire/option-wrapper.hpp>
#include <wayfire/output.hpp>
#include <wayfire/plugin.hpp>
#include <wayfire/plugins/common/cairo-util.hpp>
#include <wayfire/plugins/common/geometry-animation.hpp>
#include <wayfire/render-manager.hpp>

#include <utility>

extern "C"
{
#include <librsvg/rsvg.h>
}

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

    wf::option_wrapper_t<int> unicorn_speed{"wf-unicorn/unicorn_speed"};

    wf::animation::simple_animation_t unicorn_x = wf::animation::simple_animation_t(unicorn_speed);
    wf::animation::simple_animation_t unicorn_y = wf::animation::simple_animation_t(unicorn_speed);

    wf::geometry_t unicorn_region() const
    {
        return {.x = static_cast<int>(unicorn_x - unicorn_texture.width / 2),
                .y = static_cast<int>(unicorn_y - unicorn_texture.height / 2),
                .width = unicorn_texture.width,
                .height = unicorn_texture.height};
    }

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
                output->render->add_effect(&pre_hook, wf::OUTPUT_EFFECT_PRE);
                output->render->add_effect(&draw_unicorn, wf::OUTPUT_EFFECT_OVERLAY);
                hook_set = true;
            }
        }
        else
        {
            unicorn_x.animate(-unicorn_size);
            unicorn_y.animate(-unicorn_size);
        }

        output->render->damage(unicorn_region());
        return true;
    };

    void set_unicorn_destination(double x, double y)
    {
        if (x != unicorn_x || y != unicorn_y)
        {
            unicorn_x.animate(x);
            unicorn_y.animate(y);
        }
    }

    wf::effect_hook_t pre_hook = [this]() noexcept {
        if (unicorn_visible)
        {
            const auto [cursor_x, cursor_y] = output->get_cursor_position();
            set_unicorn_destination(cursor_x, cursor_y);
        }
        if (unicorn_x.running() || unicorn_y.running())
        {
            output->render->damage_whole();
        }
    };

    wf::effect_hook_t draw_unicorn = [this]() noexcept {
        const auto frambuffer = output->render->get_target_framebuffer();
        uint32_t bits = OpenGL::TEXTURE_TRANSFORM_INVERT_Y;
        if (output->get_cursor_position().x > unicorn_x)
        {
            bits |= OpenGL::TEXTURE_TRANSFORM_INVERT_X;
        }
        OpenGL::render_begin(frambuffer);
        OpenGL::render_texture(wf::texture_t{unicorn_texture.tex}, frambuffer, unicorn_region(), glm::vec4(1.0f), bits);
        OpenGL::render_end();
    };

    void reload_unicorn_texture()
    {
        auto *const unicorn_surface = LoadSVG(unicorn_svg_path.value().c_str(), unicorn_size, unicorn_size);
        OpenGL::render_begin();
        cairo_surface_upload_to_texture(unicorn_surface, unicorn_texture);
        OpenGL::render_end();
        cairo_surface_destroy(unicorn_surface);

        if (unicorn_visible && hook_set)
        {
            output->render->damage(unicorn_region());
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

        unicorn_x.set(-unicorn_size, -unicorn_size);
        unicorn_y.set(-unicorn_size, -unicorn_size);

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
