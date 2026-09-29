#include "MainWindow.h"

#include "CommandLine.h"
#include "DiffWindow.h"
#include "NmapLocator.h"
#include "NmapRunner.h"
#include "NmapXmlParser.h"
#include "OptionsPanel.h"
#include "Privileges.h"
#include "ScanDiff.h"
#include "ScanFile.h"
#include "ScanTableModel.h"

#include <QAction>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontDatabase>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSettings>
#include <QSortFilterProxyModel>
#include <QSplitter>
#include <QStatusBar>
#include <QTableView>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_targetEdit(new QLineEdit)
    , m_scanButton(new QPushButton(tr("Scan")))
    , m_options(new OptionsPanel)
    , m_commandEdit(new QLineEdit)
    , m_editedHint(new QLabel(tr("Edited by hand: this command runs as typed. "
                                 "Changing an option replaces it.")))
    , m_filterEdit(new QLineEdit)
    , m_table(new QTableView)
    , m_console(new QPlainTextEdit)
    , m_runner(new NmapRunner(this))
    , m_model(new ScanTableModel(this))
    , m_proxy(new QSortFilterProxyModel(this))
    , m_openAction(new QAction(tr("&Open Scan..."), this))
    , m_saveAction(new QAction(tr("&Save Scan..."), this))
    , m_compareAction(new QAction(tr("&Compare With..."), this))
{
    setWindowTitle(tr("Portvane"));
    resize(1000, 700);

    m_openAction->setShortcut(QKeySequence::Open);
    m_saveAction->setShortcut(QKeySequence::Save);
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(m_openAction);
    fileMenu->addAction(m_saveAction);
    fileMenu->addSeparator();
    fileMenu->addAction(m_compareAction);

    m_targetEdit->setPlaceholderText(tr("Host, IP range or CIDR, e.g. scanme.nmap.org"));
    m_options->setPrivileges(detectPrivileges());
    m_commandEdit->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    m_editedHint->setEnabled(false); // drawn greyed out: a note, not a warning
    m_editedHint->hide();
    m_console->setReadOnly(true);
    m_console->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

    m_filterEdit->setPlaceholderText(tr("Filter results, e.g. open, 443 or ssh"));
    m_filterEdit->setClearButtonEnabled(true);

    // -1 makes the filter match against every column, not just the first.
    m_proxy->setSourceModel(m_model);
    m_proxy->setFilterKeyColumn(-1);
    m_proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);

    m_table->setModel(m_proxy);
    m_table->setSortingEnabled(true);
    // Otherwise the header's default (descending) would list hosts backwards.
    m_table->sortByColumn(ScanTableModel::AddressColumn, Qt::AscendingOrder);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->verticalHeader()->hide();
    m_table->horizontalHeader()->setStretchLastSection(true);

    auto *targetRow = new QHBoxLayout;
    targetRow->addWidget(new QLabel(tr("Target:")));
    targetRow->addWidget(m_targetEdit);
    targetRow->addWidget(m_scanButton);

    // The program is fixed (see Settings); only its arguments are editable.
    // The console shows the full command, with path and -oX, once it runs.
    auto *commandRow = new QHBoxLayout;
    commandRow->addWidget(new QLabel(tr("Command:")));
    commandRow->addWidget(new QLabel(QStringLiteral("nmap")));
    commandRow->addWidget(m_commandEdit);

    auto *filterRow = new QHBoxLayout;
    filterRow->addWidget(new QLabel(tr("Filter:")));
    filterRow->addWidget(m_filterEdit);

    auto *splitter = new QSplitter(Qt::Vertical);
    splitter->addWidget(m_table);
    splitter->addWidget(m_console);

    auto *central = new QWidget;
    auto *layout = new QVBoxLayout(central);
    layout->addLayout(targetRow);
    layout->addWidget(m_options);
    layout->addLayout(commandRow);
    layout->addWidget(m_editedHint);
    layout->addLayout(filterRow);
    layout->addWidget(splitter);
    setCentralWidget(central);

    connect(m_openAction, &QAction::triggered, this, &MainWindow::openScan);
    connect(m_saveAction, &QAction::triggered, this, &MainWindow::saveScan);
    connect(m_compareAction, &QAction::triggered, this, &MainWindow::compareWith);
    connect(m_scanButton, &QPushButton::clicked, this, &MainWindow::startScan);
    connect(m_targetEdit, &QLineEdit::returnPressed, this, &MainWindow::startScan);
    connect(m_commandEdit, &QLineEdit::returnPressed, this, &MainWindow::startScan);
    connect(m_targetEdit, &QLineEdit::textChanged, this, &MainWindow::updateCommand);
    connect(m_options, &OptionsPanel::optionsChanged, this, &MainWindow::updateCommand);
    // textEdited, unlike textChanged, only fires for typing, not for setText().
    connect(m_commandEdit, &QLineEdit::textEdited, this, &MainWindow::onCommandEdited);
    connect(m_filterEdit, &QLineEdit::textChanged, m_proxy,
            &QSortFilterProxyModel::setFilterFixedString);
    connect(m_runner, &NmapRunner::commandStarted, this, [this](const QString &commandLine) {
        m_console->appendPlainText(QStringLiteral("$ ") + commandLine);
    });
    connect(m_runner, &NmapRunner::outputLine, m_console, &QPlainTextEdit::appendPlainText);
    connect(m_runner, &NmapRunner::scanFinished, this, &MainWindow::onScanFinished);
    connect(m_runner, &NmapRunner::scanFailed, this, &MainWindow::onScanFailed);

    updateCommand();
    setScanRunning(false);
}

ScanOptions MainWindow::currentOptions() const
{
    ScanOptions options = m_options->options();
    options.targets = m_targetEdit->text().split(QLatin1Char(' '), Qt::SkipEmptyParts);
    return options;
}

void MainWindow::updateCommand()
{
    m_commandEdit->setText(joinCommandLine(buildNmapArguments(currentOptions())));
    m_commandEdited = false;
    m_editedHint->hide();
}

void MainWindow::onCommandEdited()
{
    m_commandEdited = true;
    m_editedHint->show();
}

void MainWindow::startScan()
{
    if (m_runner->isRunning())
        return;

    // The option builder's arguments go to nmap as they are. Only a command
    // typed by hand has to be split up again.
    QStringList arguments;
    if (m_commandEdited) {
        const std::optional<QStringList> split = splitCommandLine(m_commandEdit->text());
        if (!split) {
            statusBar()->showMessage(tr("The command has a quote that is never closed."));
            return;
        }
        arguments = *split;
        if (arguments.isEmpty()) {
            statusBar()->showMessage(tr("The command is empty."));
            return;
        }
    } else {
        const ScanOptions options = currentOptions();
        if (options.targets.isEmpty()) {
            statusBar()->showMessage(tr("Enter a target to scan."));
            return;
        }
        arguments = buildNmapArguments(options);
    }

    const QString overridePath = QSettings().value(QStringLiteral("nmap/path")).toString();
    const std::optional<QString> nmapPath = findNmap(overridePath);
    if (!nmapPath) {
        const QString message = overridePath.isEmpty()
            ? tr("nmap could not be found. Install nmap, or set its path in Settings.")
            : tr("The nmap path set in Settings is not an executable file:\n%1").arg(overridePath);
        QMessageBox::warning(this, tr("nmap not found"), message);
        return;
    }

    m_console->clear();
    m_model->setHosts({});
    m_currentXml.clear();
    setScanRunning(true);
    statusBar()->showMessage(tr("Scanning..."));
    m_runner->start(*nmapPath, arguments);
}

void MainWindow::onScanFinished(const QByteArray &xml)
{
    const ParseResult result = parseNmapXml(xml);
    if (!result.error.isEmpty()) {
        m_console->appendPlainText(tr("Error: could not read the scan results: %1").arg(result.error));
        statusBar()->showMessage(tr("Scan failed."));
        setScanRunning(false);
        return;
    }

    m_currentXml = xml;
    m_model->setHosts(result.hosts);
    statusBar()->showMessage(tr("Scan finished: %1 hosts.").arg(result.hosts.size()));
    setScanRunning(false);
}

void MainWindow::onScanFailed(const QString &message)
{
    m_console->appendPlainText(tr("Error: %1").arg(message));
    statusBar()->showMessage(tr("Scan failed."));
    setScanRunning(false);
}

// Opening a file mid-scan would be overwritten by the scan's results, so
// Open is locked together with the Scan button.
void MainWindow::setScanRunning(bool running)
{
    m_scanButton->setEnabled(!running);
    m_openAction->setEnabled(!running);
    m_saveAction->setEnabled(!running && !m_currentXml.isEmpty());
    m_compareAction->setEnabled(!running && !m_currentXml.isEmpty());
}

// Reads and parses a saved scan. The error, if any, is from whichever of the
// two steps failed.
static ParseResult readScan(const QString &path, QByteArray &xml)
{
    const QString loadError = loadScanFile(path, xml);
    if (!loadError.isEmpty()) {
        ParseResult result;
        result.error = loadError;
        return result;
    }
    return parseNmapXml(xml);
}

void MainWindow::openScan()
{
    const QString path = QFileDialog::getOpenFileName(this, tr("Open Scan"), QString(),
                                                      tr("nmap XML (*.xml);;All files (*)"));
    if (path.isEmpty())
        return; // cancelled

    QByteArray xml;
    const ParseResult result = readScan(path, xml);
    // On failure the current results stay, so a wrong click costs nothing.
    if (!result.error.isEmpty()) {
        QMessageBox::warning(this, tr("Could not open scan"),
                             tr("%1\n\n%2").arg(QDir::toNativeSeparators(path), result.error));
        return;
    }

    m_currentXml = xml;
    m_model->setHosts(result.hosts);
    setScanRunning(false);
    statusBar()->showMessage(
        tr("Opened %1: %2 hosts.").arg(QFileInfo(path).fileName()).arg(result.hosts.size()));
}

void MainWindow::saveScan()
{
    const QString path = QFileDialog::getSaveFileName(this, tr("Save Scan"), QString(),
                                                      tr("nmap XML (*.xml)"));
    if (path.isEmpty())
        return; // cancelled

    const QString error = saveScanFile(path, m_currentXml);
    if (!error.isEmpty()) {
        QMessageBox::warning(this, tr("Could not save scan"),
                             tr("%1\n\n%2").arg(QDir::toNativeSeparators(path), error));
        return;
    }
    statusBar()->showMessage(tr("Saved %1.").arg(QDir::toNativeSeparators(path)));
}

// The shown results are the newer side ("after"); the picked file is the
// older one. To compare two files, open one and compare it with the other.
void MainWindow::compareWith()
{
    const QString path = QFileDialog::getOpenFileName(this, tr("Compare With Earlier Scan"),
                                                      QString(),
                                                      tr("nmap XML (*.xml);;All files (*)"));
    if (path.isEmpty())
        return; // cancelled

    QByteArray xml;
    const ParseResult before = readScan(path, xml);
    if (!before.error.isEmpty()) {
        QMessageBox::warning(this, tr("Could not open scan"),
                             tr("%1\n\n%2").arg(QDir::toNativeSeparators(path), before.error));
        return;
    }

    // m_currentXml was parsed successfully when it was set, so this can't fail.
    const QList<Host> after = parseNmapXml(m_currentXml).hosts;
    auto *window = new DiffWindow(diffScans(before.hosts, after), QFileInfo(path).fileName(),
                                  tr("current results"), this);
    window->show();
}
