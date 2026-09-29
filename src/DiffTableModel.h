#pragma once

#include "ScanDiff.h"

#include <QAbstractTableModel>
#include <QList>

// Shows the changes between two scans, one row per change.
class DiffTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Column {
        ChangeColumn,
        AddressColumn,
        PortColumn,
        ProtocolColumn,
        BeforeColumn,
        AfterColumn,
        ColumnCount
    };

    explicit DiffTableModel(QObject *parent = nullptr);

    void setChanges(const QList<Change> &changes);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

private:
    QList<Change> m_changes;
};
