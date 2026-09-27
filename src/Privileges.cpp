#include "Privileges.h"

#include <QCoreApplication>
#include <QStringList>

#ifdef Q_OS_WIN
#include <QDir>
#include <QFileInfo>
#include <windows.h>
#elif defined(Q_OS_LINUX)
#include <unistd.h>
#endif

#ifdef Q_OS_WIN
static bool isElevated()
{
    HANDLE token = nullptr;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token))
        return false;

    TOKEN_ELEVATION elevation = {};
    DWORD size = 0;
    const bool ok = GetTokenInformation(token, TokenElevation, &elevation,
                                        static_cast<DWORD>(sizeof(elevation)), &size)
        != 0;
    CloseHandle(token);
    return ok && elevation.TokenIsElevated != 0;
}

// Npcap's installer always puts its DLLs here, whatever other options are
// picked, so checking for the file is enough.
static bool isNpcapInstalled()
{
    const QString systemRoot = qEnvironmentVariable("SystemRoot");
    if (systemRoot.isEmpty())
        return false;
    return QFileInfo::exists(QDir(systemRoot).filePath(QStringLiteral("System32/Npcap/wpcap.dll")));
}
#endif

Privileges detectPrivileges()
{
    Privileges privileges;
#ifdef Q_OS_WIN
    privileges.isPrivileged = isElevated();
    privileges.hasPacketCapture = isNpcapInstalled();
#elif defined(Q_OS_LINUX)
    // Only root counts. A user given CAP_NET_RAW could also run these scans,
    // but that is not detected: greying out an option that would work is
    // better than offering one that fails.
    privileges.isPrivileged = geteuid() == 0;
    privileges.hasPacketCapture = true;
#endif
    return privileges;
}

bool privilegedOptionsAllowed(const Privileges &privileges)
{
    return privileges.isPrivileged && privileges.hasPacketCapture;
}

// The name of the rights nmap needs, in the words the user's OS uses.
static QString privilegedUserName()
{
#ifdef Q_OS_WIN
    return QCoreApplication::translate("Privileges", "Administrator");
#else
    return QCoreApplication::translate("Privileges", "root");
#endif
}

QString missingPrivilegesText(const Privileges &privileges)
{
    QStringList problems;
    if (!privileges.isPrivileged)
        problems << QCoreApplication::translate("Privileges", "Portvane is not running as %1.")
                        .arg(privilegedUserName());
    if (!privileges.hasPacketCapture)
        problems << QCoreApplication::translate("Privileges", "Npcap is not installed.");
    if (problems.isEmpty())
        return QString();

    return QCoreApplication::translate("Privileges", "Needs raw network access. %1")
        .arg(problems.join(QLatin1Char(' ')));
}
