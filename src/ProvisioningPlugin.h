/*
 *
 */

#ifndef GRAVITY_PROVISIONINGPLUGIN_H
#define GRAVITY_PROVISIONINGPLUGIN_H

#include <gravityplugin.h>

namespace Hemera {
    class Operation;
}

class Provisioning;

namespace Gravity {

struct Orbit;

class ProvisioningPlugin : public Gravity::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.ispirata.Hemera.GravityCenter.Plugin")
    Q_CLASSINFO("D-Bus Interface", "com.ispirata.Hemera.GravityCenter.Plugins.Provisioning")
    Q_INTERFACES(Gravity::Plugin)

    public:
        explicit ProvisioningPlugin();
        virtual ~ProvisioningPlugin();

    protected:
        virtual void unload() override final;
        virtual void load() override final;

    private:
        Provisioning *m_provisioning;
};
}

#endif // GRAVITY_PROVISIONING_H
