#pragma once

#include "ScanOptions.h"

#include <QByteArray>
#include <QMainWindow>

class NmapRunner;
class OptionsPanel;
class QAction;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;
class QSortFilterProxyModel;
class QTableView;
class ScanTableModel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    ScanOptions currentOptions() const;
    void updateCommand();
    void onCommandEdited();
    void startScan();
    void onScanFinished(const QByteArray &xml);
    void onScanFailed(const QString &message);
    void setScanRunning(bool running);
    void openScan();
    void saveScan();
    void compareWith();

    QLineEdit *m_targetEdit;
    QPushButton *m_scanButton;
    OptionsPanel *m_options;
    QLineEdit *m_commandEdit;
    QLabel *m_editedHint;
    QLineEdit *m_filterEdit;
    QTableView *m_table;
    QPlainTextEdit *m_console;
    NmapRunner *m_runner;
    ScanTableModel *m_model;
    QSortFilterProxyModel *m_proxy;
    QAction *m_openAction;
    QAction *m_saveAction;
    QAction *m_compareAction;
    // The XML behind the table, from a scan or a file. Saving writes exactly
    // these bytes. Empty while there is nothing to save.
    QByteArray m_currentXml;
    // True once the user typed into the command. From then on their text is
    // what runs, until an option change replaces it.
    bool m_commandEdited = false;
};
