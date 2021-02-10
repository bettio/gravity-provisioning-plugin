#include "Provisioning.h"

#include "provisioningconsumer.h"
#include "provisioningconfig.h"
#include "ProvisioningEventsProducer.h"

#include <GravitySupermassive/GalaxyManager>
#include <GravitySupermassive/StarSequence>

#include <HemeraCore/CommonOperations>
#include <HemeraCore/Fingerprints>
#include <HemeraCore/Literals>
#include <HemeraCore/NetworkDownloadOperation>

#include <QtCore/QDir>
#include <QtCore/QFileSystemWatcher>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonValue>
#include <QtCore/QLoggingCategory>
#include <QtCore/QTemporaryFile>
#include <QtCore/QTimer>
#include <QtCore/QUrl>

#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusObjectPath>
#include <QtDBus/QDBusPendingCall>
#include <QtDBus/QDBusPendingReply>

#include <sys/sysinfo.h>
#include <unistd.h>

#include <zypp/ZConfig.h>

#define BACKEND_SERVICE QStringLiteral("com.ispirata.Hemera.SoftwareManager.Backend")
#define BACKEND_INTERFACE QStringLiteral("com.ispirata.Hemera.SoftwareManager.Backend")
#define BACKEND_PATH QStringLiteral("/com/ispirata/Hemera/SoftwareManager/Backend")

Q_LOGGING_CATEGORY(LOG_PROVISIONING, "Provisioning")

Provisioning::Provisioning(Gravity::GalaxyManager *manager, QObject *parent)
    : Hemera::AsyncInitObject(parent)
    , m_galaxyManager(manager)
    , m_consumer(new ProvisioningConsumer(this))
    , m_eventsProducer(new ProvisioningEventsProducer(this))
{
}

Provisioning::~Provisioning()
{
}

void Provisioning::initImpl()
{
    setReady();
}

void Provisioning::receiveInstallRPMUrl(const QString &url)
{
    qCDebug(LOG_PROVISIONING) << "Received new RPM package: " << url;

    QUrl fileUrl = QUrl::fromUserInput(url);

    if (!fileUrl.isValid() || (fileUrl.scheme() == QStringLiteral("file")) || fileUrl.fileName().isEmpty()) {
        qCWarning(LOG_PROVISIONING) << "install RPM url is not valid: " << url;
    }

    // big file, keep it in /var/tmp
    QTemporaryFile *temp = new QTemporaryFile(QStringLiteral("/var/tmp/provisioningXXXXXX_%1").arg(fileUrl.fileName()));
    if (!temp->open()) {
        qCWarning(LOG_PROVISIONING) << "Cannot open temporary file: " << temp->fileName();
    }

    Hemera::NetworkDownloadOperation *op = new Hemera::NetworkDownloadOperation(QUrl(url), temp, nullptr, this);

    QString networkUrlString = url;

    connect(op, &Hemera::Operation::started, this, [this, networkUrlString] {
        QJsonObject obj;
        obj.insert(QStringLiteral("type"), QStringLiteral("package_download_started"));
        obj.insert(QStringLiteral("url"), networkUrlString);
        obj.insert(QStringLiteral("success"), true);
        m_eventsProducer->streamInstallRPMUrlEvent(QString::fromLatin1(QJsonDocument(obj).toJson()));
    });

    connect(op, &Hemera::Operation::finished, this, [this, op, temp, networkUrlString] {
        if (op->isError()) {
            qCWarning(LOG_PROVISIONING) << "Failed to download file: " << op->errorName();
            qCWarning(LOG_PROVISIONING) << "Error message: " << op->errorMessage();
            return;

        } else {
            qCDebug(LOG_PROVISIONING) << "Downloaded to: " << temp->fileName();

            QJsonObject objDF;
            objDF.insert(QStringLiteral("type"), QStringLiteral("package_download_finished"));
            objDF.insert(QStringLiteral("url"), networkUrlString);
            objDF.insert(QStringLiteral("success"), true);
            m_eventsProducer->streamInstallRPMUrlEvent(QString::fromLatin1(QJsonDocument(objDF).toJson()));

            // We interact straight with the backend here. We need local file installation first, and most of all using Hemera::SoftwareManagement
            // would cause a deadlock, given the DBus server is in this very process...
            QDBusMessage call = QDBusMessage::createMethodCall(BACKEND_SERVICE, BACKEND_PATH, BACKEND_INTERFACE, QStringLiteral("installLocalPackage"));
            QVariantList args = QVariantList() << temp->fileName();
            call.setArguments(args);
            QDBusPendingReply< void > reply = QDBusConnection::systemBus().asyncCall(call);
            QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
            connect(watcher, &QDBusPendingCallWatcher::finished, [this, watcher, temp, networkUrlString] {
                QJsonObject obj;
                obj.insert(QStringLiteral("type"), QStringLiteral("packageinstall"));
                obj.insert(QStringLiteral("url"), networkUrlString);
                obj.insert(QStringLiteral("success"), !watcher->isError());

                if (watcher->isError()) {
                    obj.insert(QStringLiteral("errorName"), watcher->error().name());
                    obj.insert(QStringLiteral("errorMessage"), watcher->error().message());
                    qWarning() << "Could not install package!" << watcher->error();
                }

                qCDebug(LOG_PROVISIONING) << QJsonDocument(obj).toJson();
                m_eventsProducer->streamInstallRPMUrlEvent(QString::fromLatin1(QJsonDocument(obj).toJson()));

                watcher->deleteLater();
                temp->deleteLater();
            });
        }
    });
}
