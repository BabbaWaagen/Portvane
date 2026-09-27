#pragma once

#include <QByteArray>
#include <QObject>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QTemporaryDir>

#include <memory>

// Removes every complete line from the front of buffer and returns them
// without their line endings. An unfinished last line stays in buffer, so the
// next chunk of output can complete it.
QStringList takeCompleteLines(QByteArray &buffer);

// Empties buffer and returns what was in it as one line. Used when the process
// has ended, so a last line without a line ending isn't lost.
QString takeUnfinishedLine(QByteArray &buffer);

// Runs one nmap scan at a time.
//
// nmap writes its XML results to a temp file (-oX), while its normal,
// human-readable output is streamed line by line through outputLine(). The XML
// file is only read after the process has finished.
class NmapRunner : public QObject
{
    Q_OBJECT

public:
    explicit NmapRunner(QObject *parent = nullptr);
    ~NmapRunner() override;

    bool isRunning() const;

    // Starts nmap with the given arguments plus "-oX <temp file>".
    void start(const QString &nmapPath, const QStringList &arguments);

signals:
    // The exact command line that was started, including the -oX part.
    void commandStarted(const QString &commandLine);
    void outputLine(const QString &line);
    void scanFinished(const QByteArray &xml);
    void scanFailed(const QString &message);

private:
    void readOutput();
    void onFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onErrorOccurred(QProcess::ProcessError error);

    // Declared before m_process so it is destroyed after it: nmap must be
    // gone before its output folder is deleted.
    std::unique_ptr<QTemporaryDir> m_tempDir;
    QString m_xmlPath;
    QByteArray m_outputBuffer;
    QProcess m_process;
};
