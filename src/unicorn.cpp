#include <iostream>

#include <wayfire/option-wrapper.hpp>
#include <wayfire/output.hpp>
#include <wayfire/plugin.hpp>
#include <wayfire/render-manager.hpp>

class wf_unicorn_t : public wf::plugin_interface_t
{
    wf::option_wrapper_t<wf::activatorbinding_t> launch_unicorn_activator{"wf-unicorn/unicorn_activator"};

    wf::activator_callback launch_unicorn = [](auto) noexcept -> bool { return true; };

  public:
    void init() override
    {
        grab_interface->name = "wf-unicorn";
        grab_interface->capabilities = wf::CAPABILITY_MANAGE_COMPOSITOR;
        output->add_activator(launch_unicorn_activator, &launch_unicorn);
    }

    void fini() override
    {
        grab_interface->ungrab();
        output->deactivate_plugin(grab_interface);

        output->rem_binding(&launch_unicorn);

        output->render->damage_whole();
    }
};

DECLARE_WAYFIRE_PLUGIN(wf_unicorn_t)
