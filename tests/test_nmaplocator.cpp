#include "NmapLocator.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QTest>

class TestNmapLocator : public QObject
{
    Q_OBJECT

private slots:
    void validOverrideIsUsed();
    void invalidOverrideIsNotFound();
    void autoDetectReturnsExecutableOrNothing();
};

// The test binary itself is a real executable on every OS, so it stands in
// for nmap without needing nmap installed.
void TestNmapLocator::validOverrideIsUsed()
{
    const QString self = QCoreApplication::applicationFilePath();

    const std::optional<QString> result = findNmap(self);

    QVERIFY(result.has_value());
    QCOMPARE(*result, self);
}

void TestNmapLocator::invalidOverrideIsNotFound()
{
    const QString missing = QDir::temp().filePath(QStringLiteral("portvane-no-such-nmap"));

    QVERIFY(!findNmap(missing).has_value());
}

// Whether nmap is installed depends on the machine, so only check that any
// result points at a real executable.
void TestNmapLocator::autoDetectReturnsExecutableOrNothing()
{
    const std::optional<QString> result = findNmap(QString());

    if (result.has_value()) {
        const QFileInfo info(*result);
        QVERIFY(info.isFile());
        QVERIFY(info.isExecutable());
    }
}

QTEST_GUILESS_MAIN(TestNmapLocator)
#include "test_nmaplocator.moc"
