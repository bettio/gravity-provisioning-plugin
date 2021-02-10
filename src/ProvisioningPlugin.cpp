/*
 *
 */

#include "ProvisioningPlugin.h"

#include "Provisioning.h"

#include <QtCore/QLoggingCategory>

#include <HemeraCore/CommonOperations>

#include <GravitySupermassive/GalaxyManager>

Q_LOGGING_CATEGORY(LOG_PROVISIONINGPLUGIN, "ProvisioningPlugin")

using namespace Gravity;

ProvisioningPlugin::ProvisioningPlugin()
    : Gravity::Plugin()
{
    setName(QStringLiteral("Hemera Provisioning"));
}

ProvisioningPlugin::~ProvisioningPlugin()
{
}

void ProvisioningPlugin::unload()
{
    qCDebug(LOG_PROVISIONINGPLUGIN) << "Provisioning plugin unloaded";
    setUnloaded();
}

void ProvisioningPlugin::load()
{
    m_provisioning = new Provisioning(galaxyManager());

    connect(m_provisioning->init(), &Hemera::Operation::finished, this, [this] (Hemera::Operation *op){
        if (op->isError()) {
            qCDebug(LOG_PROVISIONINGPLUGIN) << "Error in loading Provisioning plugin: " << op->errorName() << op->errorMessage();
        } else {
            qCDebug(LOG_PROVISIONINGPLUGIN) << "Provisioning mode plugin loaded";
            setLoaded();
        }
    });
}

