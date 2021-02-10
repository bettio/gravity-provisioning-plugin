#ifndef PROVISIONING_H
#define PROVISIONING_H

#include <HemeraCore/AsyncInitObject>

class ProvisioningConsumer;
class ProvisioningEventsProducer;

namespace Gravity {
class GalaxyManager;
}

class Provisioning : public Hemera::AsyncInitObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Provisioning)

    public:
        explicit Provisioning(Gravity::GalaxyManager *manager, QObject *parent = nullptr);
        virtual ~Provisioning();

        virtual bool isValid() { return true; }

        void receiveInstallRPMUrl(const QString &url);

    protected:
        virtual void initImpl() Q_DECL_OVERRIDE Q_DECL_FINAL;

    private:
        Gravity::GalaxyManager *m_galaxyManager;
        ProvisioningConsumer *m_consumer;
        ProvisioningEventsProducer *m_eventsProducer;
};

#endif
