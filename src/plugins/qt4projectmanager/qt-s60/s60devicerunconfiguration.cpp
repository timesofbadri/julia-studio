#include "s60devicerunconfiguration.h"

#include "qt4project.h"
#include "qtversionmanager.h"
#include "profilereader.h"
#include "s60manager.h"
#include "s60devices.h"

#include <coreplugin/icore.h>
#include <coreplugin/messagemanager.h>
#include <utils/qtcassert.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/project.h>

using namespace ProjectExplorer;
using namespace Qt4ProjectManager::Internal;

// ======== S60DeviceRunConfiguration
S60DeviceRunConfiguration::S60DeviceRunConfiguration(Project *project, const QString &proFilePath)
    : RunConfiguration(project),
    m_proFilePath(proFilePath),
    m_cachedTargetInformationValid(false)
{
    if (!m_proFilePath.isEmpty())
        setName(tr("%1 on Device").arg(QFileInfo(m_proFilePath).completeBaseName()));
    else
        setName(tr("QtS60DeviceRunConfiguration"));

    connect(project, SIGNAL(activeBuildConfigurationChanged()),
            this, SLOT(invalidateCachedTargetInformation()));
}

S60DeviceRunConfiguration::~S60DeviceRunConfiguration()
{
}

QString S60DeviceRunConfiguration::type() const
{
    return "Qt4ProjectManager.DeviceRunConfiguration";
}

bool S60DeviceRunConfiguration::isEnabled() const
{
    Qt4Project *pro = qobject_cast<Qt4Project*>(project());
    QTC_ASSERT(pro, return false);
    ToolChain::ToolChainType type = pro->toolChainType(pro->activeBuildConfiguration());
    return type == ToolChain::GCCE; //TODO || type == ToolChain::ARMV5
}

QWidget *S60DeviceRunConfiguration::configurationWidget()
{
    return new S60DeviceRunConfigurationWidget(this);
}

void S60DeviceRunConfiguration::save(PersistentSettingsWriter &writer) const
{
    const QDir projectDir = QFileInfo(project()->file()->fileName()).absoluteDir();
    writer.saveValue("ProFile", projectDir.relativeFilePath(m_proFilePath));
    RunConfiguration::save(writer);
}

void S60DeviceRunConfiguration::restore(const PersistentSettingsReader &reader)
{
    RunConfiguration::restore(reader);
    const QDir projectDir = QFileInfo(project()->file()->fileName()).absoluteDir();
    m_proFilePath = projectDir.filePath(reader.restoreValue("ProFile").toString());
}

QString S60DeviceRunConfiguration::basePackageFilePath() const
{
    const_cast<S60DeviceRunConfiguration *>(this)->updateTarget();
    return m_baseFileName;
}

void S60DeviceRunConfiguration::updateTarget()
{
    if (m_cachedTargetInformationValid)
        return;
    Qt4Project *pro = static_cast<Qt4Project *>(project());
    Qt4PriFileNode * priFileNode = static_cast<Qt4Project *>(project())->rootProjectNode()->findProFileFor(m_proFilePath);
    if (!priFileNode) {
        m_baseFileName = QString::null;
        m_cachedTargetInformationValid = true;
        emit targetInformationChanged();
        return;
    }
    QtVersion *qtVersion = pro->qtVersion(pro->activeBuildConfiguration());
    ProFileReader *reader = priFileNode->createProFileReader();
    reader->setCumulative(false);
    reader->setQtVersion(qtVersion);

    // Find out what flags we pass on to qmake, this code is duplicated in the qmake step
    QtVersion::QmakeBuildConfig defaultBuildConfiguration = qtVersion->defaultBuildConfig();
    QtVersion::QmakeBuildConfig projectBuildConfiguration = QtVersion::QmakeBuildConfig(pro->qmakeStep()->value(pro->activeBuildConfiguration(), "buildConfiguration").toInt());
    QStringList addedUserConfigArguments;
    QStringList removedUserConfigArguments;
    if ((defaultBuildConfiguration & QtVersion::BuildAll) && !(projectBuildConfiguration & QtVersion::BuildAll))
        removedUserConfigArguments << "debug_and_release";
    if (!(defaultBuildConfiguration & QtVersion::BuildAll) && (projectBuildConfiguration & QtVersion::BuildAll))
        addedUserConfigArguments << "debug_and_release";
    if ((defaultBuildConfiguration & QtVersion::DebugBuild) && !(projectBuildConfiguration & QtVersion::DebugBuild))
        addedUserConfigArguments << "release";
    if (!(defaultBuildConfiguration & QtVersion::DebugBuild) && (projectBuildConfiguration & QtVersion::DebugBuild))
        addedUserConfigArguments << "debug";

    reader->setUserConfigCmdArgs(addedUserConfigArguments, removedUserConfigArguments);

    if (!reader->readProFile(m_proFilePath)) {
        delete reader;
        Core::ICore::instance()->messageManager()->printToOutputPane(tr("Could not parse %1. The QtS60 Device run configuration %2 can not be started.").arg(m_proFilePath).arg(name()));
        return;
    }

    // Extract data
    const QDir baseProjectDirectory = QFileInfo(project()->file()->fileName()).absoluteDir();
    const QString relSubDir = baseProjectDirectory.relativeFilePath(QFileInfo(m_proFilePath).path());
    const QDir baseBuildDirectory = project()->buildDirectory(project()->activeBuildConfiguration());
    const QString baseDir = baseBuildDirectory.absoluteFilePath(relSubDir);

    // Directory
    QString m_workingDir;
    if (reader->contains("DESTDIR")) {
        m_workingDir = reader->value("DESTDIR");
        if (QDir::isRelativePath(m_workingDir)) {
            m_workingDir = baseDir + QLatin1Char('/') + m_workingDir;
        }
    } else {
        m_workingDir = baseDir;
    }

    m_baseFileName = QDir::cleanPath(m_workingDir + QLatin1Char('/') + reader->value("TARGET"));

    if (pro->toolChainType(pro->activeBuildConfiguration()) == ToolChain::GCCE)
        m_baseFileName += "_gcce";
    else
        m_baseFileName += "_armv5";
    if (projectBuildConfiguration & QtVersion::DebugBuild)
        m_baseFileName += "_udeb";
    else
        m_baseFileName += "_rel";

    delete reader;
    m_cachedTargetInformationValid = true;
    emit targetInformationChanged();
}

void S60DeviceRunConfiguration::invalidateCachedTargetInformation()
{
    m_cachedTargetInformationValid = false;
    emit targetInformationChanged();
}

// ======== S60DeviceRunConfigurationWidget

S60DeviceRunConfigurationWidget::S60DeviceRunConfigurationWidget(S60DeviceRunConfiguration *runConfiguration,
                                                                     QWidget *parent)
    : QWidget(parent),
    m_runConfiguration(runConfiguration)
{
    QFormLayout *toplayout = new QFormLayout();
    toplayout->setMargin(0);
    setLayout(toplayout);

    QLabel *nameLabel = new QLabel(tr("Name:"));
    m_nameLineEdit = new QLineEdit(m_runConfiguration->name());
    nameLabel->setBuddy(m_nameLineEdit);
    toplayout->addRow(nameLabel, m_nameLineEdit);

    m_sisxFileLabel = new QLabel(m_runConfiguration->basePackageFilePath() + ".sisx");
    toplayout->addRow(tr("Install File:"), m_sisxFileLabel);

    connect(m_nameLineEdit, SIGNAL(textEdited(QString)),
        this, SLOT(nameEdited(QString)));
    connect(m_runConfiguration, SIGNAL(targetInformationChanged()),
            this, SLOT(updateTargetInformation()));
}

void S60DeviceRunConfigurationWidget::nameEdited(const QString &text)
{
    m_runConfiguration->setName(text);
}

void S60DeviceRunConfigurationWidget::updateTargetInformation()
{
    m_sisxFileLabel->setText(m_runConfiguration->basePackageFilePath() + ".sisx");
}

// ======== S60DeviceRunConfigurationFactory

S60DeviceRunConfigurationFactory::S60DeviceRunConfigurationFactory(QObject *parent)
    : IRunConfigurationFactory(parent)
{
}

S60DeviceRunConfigurationFactory::~S60DeviceRunConfigurationFactory()
{
}

bool S60DeviceRunConfigurationFactory::canRestore(const QString &type) const
{
    return type == "Qt4ProjectManager.DeviceRunConfiguration";
}

QStringList S60DeviceRunConfigurationFactory::availableCreationTypes(Project *pro) const
{
    Qt4Project *qt4project = qobject_cast<Qt4Project *>(pro);
    if (qt4project) {
        QStringList applicationProFiles;
        QList<Qt4ProFileNode *> list = qt4project->applicationProFiles();
        foreach (Qt4ProFileNode * node, list) {
            applicationProFiles.append("QtS60DeviceRunConfiguration." + node->path());
        }
        return applicationProFiles;
    } else {
        return QStringList();
    }
}

QString S60DeviceRunConfigurationFactory::displayNameForType(const QString &type) const
{
    QString fileName = type.mid(QString("QtS60DeviceRunConfiguration.").size());
    return tr("%1 on Device").arg(QFileInfo(fileName).completeBaseName());
}

QSharedPointer<RunConfiguration> S60DeviceRunConfigurationFactory::create(Project *project, const QString &type)
{
    Qt4Project *p = qobject_cast<Qt4Project *>(project);
    Q_ASSERT(p);
    if (type.startsWith("QtS60DeviceRunConfiguration.")) {
        QString fileName = type.mid(QString("QtS60DeviceRunConfiguration.").size());
        return QSharedPointer<RunConfiguration>(new S60DeviceRunConfiguration(p, fileName));
    }
    Q_ASSERT(type == "Qt4ProjectManager.DeviceRunConfiguration");
    // The right path is set in restoreSettings
    QSharedPointer<RunConfiguration> rc(new S60DeviceRunConfiguration(p, QString::null));
    return rc;
}

// ======== S60DeviceRunConfigurationRunner

S60DeviceRunConfigurationRunner::S60DeviceRunConfigurationRunner(QObject *parent)
    : IRunConfigurationRunner(parent)
{
}

bool S60DeviceRunConfigurationRunner::canRun(QSharedPointer<RunConfiguration> runConfiguration, const QString &mode)
{
    return (mode == ProjectExplorer::Constants::RUNMODE)
            && (!runConfiguration.dynamicCast<S60DeviceRunConfiguration>().isNull());
}

RunControl* S60DeviceRunConfigurationRunner::run(QSharedPointer<RunConfiguration> runConfiguration, const QString &mode)
{
    QSharedPointer<S60DeviceRunConfiguration> rc = runConfiguration.dynamicCast<S60DeviceRunConfiguration>();
    Q_ASSERT(!rc.isNull());
    Q_ASSERT(mode == ProjectExplorer::Constants::RUNMODE);

    S60DeviceRunControl *runControl = new S60DeviceRunControl(rc);
    return runControl;
}

// ======== S60DeviceRunControl

S60DeviceRunControl::S60DeviceRunControl(QSharedPointer<RunConfiguration> runConfiguration)
    : RunControl(runConfiguration)
{
    m_makesis = new QProcess(this);
    connect(m_makesis, SIGNAL(readyReadStandardError()),
            this, SLOT(readStandardError()));
    connect(m_makesis, SIGNAL(readyReadStandardOutput()),
            this, SLOT(readStandardOutput()));
    connect(m_makesis, SIGNAL(error(QProcess::ProcessError)),
            this, SLOT(makesisProcessFailed()));
    connect(m_makesis, SIGNAL(finished(int,QProcess::ExitStatus)),
            this, SLOT(makesisProcessFinished()));
    m_signsis = new QProcess(this);
    connect(m_signsis, SIGNAL(readyReadStandardError()),
            this, SLOT(readStandardError()));
    connect(m_signsis, SIGNAL(readyReadStandardOutput()),
            this, SLOT(readStandardOutput()));
    connect(m_signsis, SIGNAL(error(QProcess::ProcessError)),
            this, SLOT(signsisProcessFailed()));
    connect(m_signsis, SIGNAL(finished(int,QProcess::ExitStatus)),
            this, SLOT(signsisProcessFinished()));
    m_install = new QProcess(this);
    connect(m_install, SIGNAL(readyReadStandardError()),
            this, SLOT(readStandardError()));
    connect(m_install, SIGNAL(readyReadStandardOutput()),
            this, SLOT(readStandardOutput()));
    connect(m_install, SIGNAL(error(QProcess::ProcessError)),
            this, SLOT(installProcessFailed()));
    connect(m_install, SIGNAL(finished(int,QProcess::ExitStatus)),
            this, SLOT(installProcessFinished()));
}

void S60DeviceRunControl::start()
{
    QSharedPointer<S60DeviceRunConfiguration> rc = runConfiguration().dynamicCast<S60DeviceRunConfiguration>();
    Q_ASSERT(!rc.isNull());

    Qt4Project *project = qobject_cast<Qt4Project *>(runConfiguration()->project());

    m_baseFileName = rc->basePackageFilePath();
    m_workingDirectory = QFileInfo(m_baseFileName).absolutePath();
    m_qtDir = project->qtVersion(project->activeBuildConfiguration())->path();

    emit started();

    emit addToOutputWindow(this, tr("Creating %1.sisx ...").arg(QDir::toNativeSeparators(m_baseFileName)));

    Q_ASSERT(project);
    m_toolsDirectory = S60Manager::instance()->devices()->deviceForId(
            S60Manager::instance()->deviceIdFromDetectionSource(
            project->qtVersion(project->activeBuildConfiguration())
            ->autodetectionSource())).epocRoot
            + "/epoc32/tools";
    QString makesisTool = m_toolsDirectory + "/makesis.exe";
    QString packageFile = QFileInfo(m_baseFileName + ".pkg").fileName();
    m_makesis->setWorkingDirectory(m_workingDirectory);
    emit addToOutputWindow(this, tr("%1 %2").arg(QDir::toNativeSeparators(makesisTool), packageFile));
    m_makesis->start(makesisTool, QStringList()
        << packageFile,
        QIODevice::ReadOnly);
}

void S60DeviceRunControl::stop()
{
    // TODO
}

bool S60DeviceRunControl::isRunning() const
{
    return m_makesis->state() != QProcess::NotRunning;
}

void S60DeviceRunControl::readStandardError()
{
    QProcess *process = static_cast<QProcess *>(sender());
    QByteArray data = process->readAllStandardError();
    emit addToOutputWindowInline(this, QString::fromLocal8Bit(data.constData(), data.length()));
}

void S60DeviceRunControl::readStandardOutput()
{
    QProcess *process = static_cast<QProcess *>(sender());
    QByteArray data = process->readAllStandardOutput();
    emit addToOutputWindowInline(this, QString::fromLocal8Bit(data.constData(), data.length()));
}

void S60DeviceRunControl::makesisProcessFailed()
{
    processFailed("makesis.exe", m_makesis->error());
}

void S60DeviceRunControl::makesisProcessFinished()
{
    if (m_makesis->exitStatus() != 0) {
        error(this, tr("An error occurred while creating the package."));
        emit finished();
        return;
    }
    QString signsisTool = m_toolsDirectory + "/signsis.exe";
    QString sisFile = QFileInfo(m_baseFileName + ".sis").fileName();
    QString sisxFile = QFileInfo(m_baseFileName + ".sisx").fileName();
    QStringList arguments;
    arguments << sisFile
            << sisxFile << QDir::toNativeSeparators(m_qtDir + "/selfsigned.cer")
            << QDir::toNativeSeparators(m_qtDir + "/selfsigned.key");
    m_signsis->setWorkingDirectory(m_workingDirectory);
    emit addToOutputWindow(this, tr("%1 %2").arg(QDir::toNativeSeparators(signsisTool), arguments.join(tr(" "))));
    m_signsis->start(signsisTool, arguments, QIODevice::ReadOnly);
}

void S60DeviceRunControl::signsisProcessFailed()
{
    processFailed("signsis.exe", m_signsis->error());
}

void S60DeviceRunControl::signsisProcessFinished()
{
    if (m_signsis->exitStatus() != 0) {
        error(this, tr("An error occurred while creating the package."));
        emit finished();
        return;
    }
    QString applicationInstaller = "cmd.exe";
    QStringList arguments;
    arguments << "/C" << QDir::toNativeSeparators(m_baseFileName + ".sisx");
    m_install->setWorkingDirectory(m_workingDirectory);
    emit addToOutputWindow(this, tr("%1 %2").arg(QDir::toNativeSeparators(applicationInstaller), arguments.join(tr(" "))));
    m_install->start(applicationInstaller, arguments, QIODevice::ReadOnly);
}

void S60DeviceRunControl::installProcessFailed()
{
    processFailed("ApplicationInstaller", m_install->error());
}

void S60DeviceRunControl::installProcessFinished()
{
    emit addToOutputWindow(this, tr("Finished."));
    emit finished();
}

void S60DeviceRunControl::processFailed(const QString &program, QProcess::ProcessError errorCode)
{
    QString errorString;
    switch (errorCode) {
    case QProcess::FailedToStart:
        errorString = tr("Failed to start %1.");
        break;
    case QProcess::Crashed:
        errorString = tr("%1 has unexpectedly finished.");
        break;
    default:
        errorString = tr("Some error has occurred while running %1.");
    }
    error(this, errorString.arg(program));
}
