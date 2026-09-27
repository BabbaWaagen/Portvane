#include "NmapLocator.h"
#include "NmapRunner.h"

#include <QSignalSpy>
#include <QTest>

class TestNmapRunner : public QObject
{
    Q_OBJECT

private slots:
    void splitsCompleteLines();
    void keepsUnfinishedLine();
    void joinsLineSplitAcrossChunks();
    void flushesUnfinishedLine();
    void reportsMissingProgram();
    void scansLocalhost();
};

void TestNmapRunner::splitsCompleteLines()
{
    QByteArray buffer("first\r\nsecond\n");

    const QStringList lines = takeCompleteLines(buffer);

    QCOMPARE(lines, QStringList({QStringLiteral("first"), QStringLiteral("second")}));
    QVERIFY(buffer.isEmpty());
}

void TestNmapRunner::keepsUnfinishedLine()
{
    QByteArray buffer("done\nhalf");

    const QStringList lines = takeCompleteLines(buffer);

    QCOMPARE(lines, QStringList({QStringLiteral("done")}));
    QCOMPARE(buffer, QByteArray("half"));
}

// Output arrives in arbitrary chunks. A line split across two chunks must come
// out exactly once and in one piece, the way readOutput() feeds the buffer.
void TestNmapRunner::joinsLineSplitAcrossChunks()
{
    QByteArray buffer;

    buffer += QByteArray("Nmap scan rep");
    const QStringList afterFirstChunk = takeCompleteLines(buffer);
    buffer += QByteArray("ort for localhost\n");
    const QStringList afterSecondChunk = takeCompleteLines(buffer);

    QVERIFY(afterFirstChunk.isEmpty());
    QCOMPARE(afterSecondChunk, QStringList({QStringLiteral("Nmap scan report for localhost")}));
    QVERIFY(buffer.isEmpty());
}

// nmap's last output may lack a line ending. It stays in the buffer until the
// process ends, and then must still come out as a line.
void TestNmapRunner::flushesUnfinishedLine()
{
    QByteArray buffer("Nmap done: 1 IP address\r\nlast line");

    const QStringList complete = takeCompleteLines(buffer);
    const QString rest = takeUnfinishedLine(buffer);

    QCOMPARE(complete, QStringList({QStringLiteral("Nmap done: 1 IP address")}));
    QCOMPARE(rest, QStringLiteral("last line"));
    QVERIFY(buffer.isEmpty());
}

// A failed start leaves no XML file, so it must end in scanFailed and never
// in a scanFinished that would look like an empty scan.
void TestNmapRunner::reportsMissingProgram()
{
    NmapRunner runner;
    QSignalSpy finished(&runner, &NmapRunner::scanFinished);
    QSignalSpy failed(&runner, &NmapRunner::scanFailed);

    runner.start(QStringLiteral("portvane-no-such-program"), {});

    QTRY_COMPARE(failed.count(), 1);
    QCOMPARE(finished.count(), 0);
    QVERIFY(!runner.isRunning());
}

// Needs a real nmap, so it is skipped where none is installed (e.g. in CI).
void TestNmapRunner::scansLocalhost()
{
    const std::optional<QString> nmapPath = findNmap(QString());
    if (!nmapPath)
        QSKIP("nmap is not installed");

    NmapRunner runner;
    QSignalSpy lines(&runner, &NmapRunner::outputLine);
    QSignalSpy finished(&runner, &NmapRunner::scanFinished);
    QSignalSpy failed(&runner, &NmapRunner::scanFailed);

    runner.start(*nmapPath, {QStringLiteral("-sT"), QStringLiteral("-p"), QStringLiteral("1"),
                             QStringLiteral("127.0.0.1")});

    QTRY_VERIFY_WITH_TIMEOUT(finished.count() + failed.count() > 0, 60000);
    QCOMPARE(failed.count(), 0);
    QVERIFY(finished.at(0).at(0).toByteArray().contains("<nmaprun"));

    // The live output must be nmap's readable text, never the XML.
    QVERIFY(lines.count() > 0);
    for (const QList<QVariant> &signalArgs : lines)
        QVERIFY(!signalArgs.at(0).toString().contains(QStringLiteral("<?xml")));
}

QTEST_GUILESS_MAIN(TestNmapRunner)
#include "test_nmaprunner.moc"
