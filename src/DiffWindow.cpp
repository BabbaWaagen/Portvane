#include "DiffWindow.h"

#include "DiffTableModel.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QSortFilterProxyModel>
#include <QTableView>
#include <QVBoxLayout>

DiffWindow::DiffWindow(const QList<Change> &changes, const QString &beforeName,
                       const QString &afterName, QWidget *parent)
    : QWidget(parent, Qt::Window)
{
    setWindowTitle(tr("Compare Scans"));
    setAttribute(Qt::WA_DeleteOnClose);
    resize(800, 500);

    auto *model = new DiffTableModel(this);
    model->setChanges(changes);

    auto *proxy = new QSortFilterProxyModel(this);
    proxy->setSourceModel(model);
    proxy->setFilterKeyColumn(-1);
    proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);

    auto *table = new QTableView;
    table->setModel(proxy);
    table->setSortingEnabled(true);
    table->sortByColumn(DiffTableModel::AddressColumn, Qt::AscendingOrder);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->verticalHeader()->hide();
    table->horizontalHeader()->setStretchLastSection(true);

    auto *filterEdit = new QLineEdit;
    filterEdit->setPlaceholderText(tr("Filter changes, e.g. removed, 443 or 10.0.0.5"));
    filterEdit->setClearButtonEnabled(true);
    connect(filterEdit, &QLineEdit::textChanged, proxy,
            &QSortFilterProxyModel::setFilterFixedString);

    const QString summary = changes.isEmpty()
        ? tr("No differences.")
        : tr("%1 changes.").arg(changes.size());

    auto *filterRow = new QHBoxLayout;
    filterRow->addWidget(new QLabel(tr("Filter:")));
    filterRow->addWidget(filterEdit);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel(tr("Before: %1\nAfter: %2").arg(beforeName, afterName)));
    layout->addWidget(new QLabel(summary));
    layout->addLayout(filterRow);
    layout->addWidget(table);
}
