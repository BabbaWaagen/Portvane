#include "NmapRunner.h"

#include "CommandLine.h"

#include <QDir>
#include <QFile>

QStringList takeCompleteLines(QByteArray &buffer)
{
    QStringList lines;
    qsizetype newline = buffer.indexOf('\n');
    while (newline != -1) {
        QByteArray line = buffer.left(newline);
        if (line.endsWith('\r'))
            line.chop(1);
        lines << QString::fromLocal8Bit(line);
        buffer.remove(0, newline + 1);
        newline = buffer.indexOf('\n');
    }
    return lines;
}

QString takeUnfinishedLine(QByteArray &buffer)
{
    QByteArray line = buffer;
    buffer.clear();
    if (line.endsWith('\r'))
        line.chop(1);
    return QString::fromLocal8Bit(line);
}

NmapRunner::NmapRunner(QObject *parent)
    : QObject(parent)
{
    // nmap prints warnings and errors on stderr. Merging keeps them in the
    // live output, where the user is already looking.
    m_process.setProcessChannelMode(QProcess::MergedChannels);

    connect(&m_process, &QProcess::readyReadStandardOutput, this, &NmapRunner::readOutput);
    connect(&m_process, &QProcess::finished, this, &NmapRunner::onFinished);
    connect(&m_process, &QProcess::errorOccurred, this, &NmapRunner::onErrorOccurred);
}

NmapRunner::~NmapRunner()
{
    // QProcess's destructor kills a running process and emits finished().
    // Disconnect first so that signal never reaches this half-destroyed object.
    // The wait below only happens when the window is closing mid-scan, and
    // only for the moment nmap needs to die after kill(); QProcess's own
    // destructor would do the same wait, plus print a warning.
    m_process.disconnect(this);
    if (m_process.state() != QProcess::NotRunning) {
        m_process.kill();
        m_process.waitForFinished();
    }
}

bool NmapRunner::isRunning() const
{
    return m_process.state() != QProcess::NotRunning;
}

void NmapRunner::start(const QString &nmapPath, const QStringList &arguments)
{
    if (isRunning()) {
        emit scanFailed(tr("A scan is already running."));
        return;
    }

    m_tempDir = std::make_unique<QTemporaryDir>();
    if (!m_tempDir->isValid()) {
        const QString error = m_tempDir->errorString();
        m_tempDir.reset();
        emit scanFailed(tr("Could not create a temporary folder: %1").arg(error));
        return;
    }

    m_xmlPath = QDir::toNativeSeparators(m_tempDir->filePath(QStringLiteral("scan.xml")));
    const QStringList fullArguments = arguments + QStringList{QStringLiteral("-oX"), m_xmlPath};

    m_outputBuffer.clear();
    emit commandStarted(
        joinCommandLine(QStringList{QDir::toNativeSeparators(nmapPath)} + fullArguments));
    m_process.start(nmapPath, fullArguments);
}

void NmapRunner::readOutput()
{
    m_outputBuffer += m_process.readAllStandardOutput();
    for (const QString &line : takeCompleteLines(m_outputBuffer))
        emit outputLine(line);
}

void NmapRunner::onFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    readOutput();
    if (!m_outputBuffer.isEmpty())
        emit outputLine(takeUnfinishedLine(m_outputBuffer));

    QString error;
    QByteArray xml;
    if (exitStatus == QProcess::CrashExit) {
        error = tr("nmap crashed.");
    } else if (exitCode != 0) {
        error = tr("nmap exited with code %1. See the output for details.").arg(exitCode);
    } else {
        QFile file(m_xmlPath);
        if (file.open(QIODevice::ReadOnly))
            xml = file.readAll();
        else
            error = tr("Could not read nmap's XML output: %1").arg(file.errorString());
        // An empty file would later parse as a scan with zero hosts.
        if (error.isEmpty() && xml.isEmpty())
            error = tr("nmap's XML output is empty.");
    }
    m_tempDir.reset();

    if (error.isEmpty())
        emit scanFinished(xml);
    else
        emit scanFailed(error);
}

void NmapRunner::onErrorOccurred(QProcess::ProcessError error)
{
    // Only a failed start needs handling here. A crash is followed by
    // finished(), and the other errors come from calls this class doesn't make.
    if (error != QProcess::FailedToStart)
        return;

    m_tempDir.reset();
    emit scanFailed(tr("Could not start nmap: %1").arg(m_process.errorString()));
}
