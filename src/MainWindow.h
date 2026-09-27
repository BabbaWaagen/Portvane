#pragma once

#include "ScanOptions.h"

#include <QMainWindow>

class NmapRunner;
class OptionsPanel;
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
    // True once the user typed into the command. From then on their text is
    // what runs, until an option change replaces it.
    bool m_commandEdited = false;
};
