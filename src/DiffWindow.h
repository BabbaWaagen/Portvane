#pragma once

#include "ScanDiff.h"

#include <QWidget>

// A separate window listing the changes between two scans, so the main
// window's results stay in view next to it.
class DiffWindow : public QWidget
{
    Q_OBJECT

public:
    DiffWindow(const QList<Change> &changes, const QString &beforeName,
               const QString &afterName, QWidget *parent = nullptr);
};
