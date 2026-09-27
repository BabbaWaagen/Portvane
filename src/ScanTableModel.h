#pragma once

#include "ScanData.h"

#include <QAbstractTableModel>
#include <QList>

// Shows the hosts of one scan as a flat table with one row per port.
class ScanTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Column {
        AddressColumn,
        HostnameColumn,
        HostStateColumn,
        PortColumn,
        ProtocolColumn,
        PortStateColumn,
        ServiceColumn,
        VersionColumn,
        ColumnCount
    };

    explicit ScanTableModel(QObject *parent = nullptr);

    void setHosts(const QList<Host> &hosts);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

private:
    // A host with no listed ports gets one row with portIndex -1, so hosts
    // that are down or had nothing but closed ports still show up.
    struct Row
    {
        qsizetype hostIndex;
        qsizetype portIndex;
    };

    QList<Host> m_hosts;
    QList<Row> m_rows;
};
