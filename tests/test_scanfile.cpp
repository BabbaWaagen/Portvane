#include "NmapXmlParser.h"
#include "ScanFile.h"

#include <QDir>
#include <QTemporaryDir>
#include <QTest>

class TestScanFile : public QObject
{
    Q_OBJECT

private slots:
    void savedScanLoadsBackUnchanged();
    void missingFileIsAnError();
    void savingIntoMissingFolderIsAnError();
    void brokenFileLoadsButDoesNotParse();
};

static QByteArray fixture(const QString &name)
{
    QByteArray xml;
    loadScanFile(QDir(QString::fromUtf8(FIXTURES_DIR)).filePath(name), xml);
    return xml;
}

void TestScanFile::savedScanLoadsBackUnchanged()
{
    const QByteArray original = fixture(QStringLiteral("two_hosts.xml"));
    QVERIFY(!original.isEmpty());
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("saved.xml"));

    const QString saveError = saveScanFile(path, original);
    QByteArray loaded;
    const QString loadError = loadScanFile(path, loaded);

    QVERIFY2(saveError.isEmpty(), qPrintable(saveError));
    QVERIFY2(loadError.isEmpty(), qPrintable(loadError));
    QCOMPARE(loaded, original);
    QCOMPARE(parseNmapXml(loaded).hosts.size(), 2);
}

void TestScanFile::missingFileIsAnError()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QByteArray xml;

    const QString error = loadScanFile(dir.filePath(QStringLiteral("nope.xml")), xml);

    QVERIFY(!error.isEmpty());
    QVERIFY(xml.isEmpty());
}

void TestScanFile::savingIntoMissingFolderIsAnError()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString error =
        saveScanFile(dir.filePath(QStringLiteral("no-such-folder/scan.xml")), "<nmaprun/>");

    QVERIFY(!error.isEmpty());
}

// Reading only fails if the file can't be read; whether it is a usable scan
// is the parser's call, and it has to say no.
void TestScanFile::brokenFileLoadsButDoesNotParse()
{
    const QByteArray original = fixture(QStringLiteral("two_hosts.xml"));
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("broken.xml"));
    QVERIFY(saveScanFile(path, original.left(original.size() / 2)).isEmpty());

    QByteArray loaded;
    const QString loadError = loadScanFile(path, loaded);
    const ParseResult result = parseNmapXml(loaded);

    QVERIFY2(loadError.isEmpty(), qPrintable(loadError));
    QVERIFY(!result.error.isEmpty());
    QVERIFY(result.hosts.isEmpty());
}

QTEST_GUILESS_MAIN(TestScanFile)
#include "test_scanfile.moc"
