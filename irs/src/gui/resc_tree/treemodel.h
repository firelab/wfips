#ifndef TREEMODEL_H
#define TREEMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <treeitem.h>
#include <QString>
#include <QVector>
#include <sqlite3.h>
#include <string>
#include <utility>
using std::string;

class TreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    TreeModel(QObject *parent = 0);
    TreeModel(const QString &data, QObject *parent = 0);
    ~TreeModel();

    QVariant data(const QModelIndex &index, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    TreeItem *getRootItem();


    // Add layers to the tree
    int addAllLayer(TreeItem *parent, int adjustment = 0);
    int addAgencyLayer(TreeItem *parent, QVector<int>adjustments, QVector<int>DOIadjustments);
    int addGeneralLayer(TreeItem *parent, QVector<std::pair< string, int> > toBeAdded, string type);

private:
    void setupModelData(const QStringList &lines, TreeItem *parent);

    // Queries to add layers
    int getSqlCount(TreeItem *parent, string type, string value);
    int cumulativeAdjustment(TreeItem *parent);


    TreeItem *rootItem;
    sqlite3 *db;
};

#endif // TREEMODEL_H
