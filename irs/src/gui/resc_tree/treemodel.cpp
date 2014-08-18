#include "treemodel.h"
#include <QStringList>
#include <QVector>
#include <string>
#include <stdio.h>
#include <string.h>
using std::string;


TreeModel::TreeModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    QVector<QVariant> rootData;
    rootData << "Layer" << "Local_Adjustment" <<"Original_Number" << "Previous_Adjustment" << "Adjusted_Number" << "Type";
    rootItem = new TreeItem(rootData);

    int rc = sqlite3_initialize();
    string fileName = "C:\\FIPS\\GUI\\Tree\\Data\\Test_Inter.db";
    char *File = new char [fileName.length() + 1];
    strcpy(File, fileName.c_str());

    rc = sqlite3_open_v2(File, &db, SQLITE_OPEN_READWRITE, NULL);

}

TreeModel::TreeModel(const QString &data, QObject *parent)
    : QAbstractItemModel(parent)
{
    //QString datatest = "All \t All Resources\n FS \t Forest Service Resources\n DOI \tDOI Resources\n   BLM \t BLM Resources\n   BIA \t BIA Resources";
    QVector<QVariant> rootData;
    rootData << "Layer" << "Local_Adjustment" <<"Original_Number" << "Previous_Adjustment" << "Adjusted_Number" << "Type";
    rootItem = new TreeItem(rootData);
    //setupModelData(datatest.split(QString("\n")), rootItem);

    int rc = sqlite3_initialize();

    string fileName = "C:\\FIPS\\GUI\\Tree\\Data\\Test_Inter.db";
    char *File = new char [fileName.length() + 1];
    strcpy(File, fileName.c_str());

    rc = sqlite3_open_v2(File, &db, SQLITE_OPEN_READWRITE, NULL);
}

TreeModel::~TreeModel()
{
    delete rootItem;
}

TreeItem *TreeModel::getRootItem()
{
    return rootItem;
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    TreeItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<TreeItem*>(parent.internalPointer());

    TreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex TreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    TreeItem *childItem = static_cast<TreeItem*>(index.internalPointer());
    TreeItem *parentItem = childItem->parent();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->childNumber(), 0, parentItem);
}

int TreeModel::rowCount(const QModelIndex &parent) const
{
    TreeItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<TreeItem*>(parent.internalPointer());

    return parentItem->childCount();
}

int TreeModel::columnCount(const QModelIndex &parent)const
{
    if (parent.isValid())
        return static_cast<TreeItem*>(parent.internalPointer())->columnCount();
    else
        return rootItem->columnCount();
}

QVariant TreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());

    return item->data(index.column());
}

Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section);

    return QVariant();
}

void TreeModel::setupModelData(const QStringList &lines, TreeItem *parent)
 {
     QList<TreeItem*> parents;
     QList<int> indentations;
     parents << parent;
     indentations << 0;

     int number = 0;

     while (number < lines.count()) {
         int position = 0;
         while (position < lines[number].length()) {
             if (lines[number].mid(position, 1) != " ")
                 break;
             position++;
         }

         QString lineData = lines[number].mid(position).trimmed();

         if (!lineData.isEmpty()) {
             // Read the column data from the rest of the line.
             QStringList columnStrings = lineData.split("\t", QString::SkipEmptyParts);
             QVector<QVariant> columnData;
             for (int column = 0; column < columnStrings.count(); ++column)
                 columnData << columnStrings[column];

             if (position > indentations.last()) {
                 // The last child of the current parent is now the new parent
                 // unless the current parent has no children.

                 if (parents.last()->childCount() > 0) {
                     parents << parents.last()->child(parents.last()->childCount()-1);
                     indentations << position;
                 }
             } else {
                 while (position < indentations.last() && parents.count() > 0) {
                     parents.pop_back();
                     indentations.pop_back();
                 }
             }

             // Append a new item to the current parent's list of children.
             TreeItem *parent = parents.last();
             parent->insertChildren(parent->childCount(), 1, rootItem->columnCount());
             for (int column = 0; column < columnData.size(); ++column)
                 parent->child(parent->childCount() - 1)->setData(column, columnData[column]);
         }

         number++;
     }
 }

int TreeModel::addAllLayer(TreeItem *parent, int adjustment)
{
    // parent is the rootItem
    parent->insertChildren(parent->childCount(), 1, rootItem->columnCount());

    // add the column information
    parent->child(parent->childCount() - 1)->setData(0, "All");
    // get the new adjustment
    parent->child(parent->childCount() - 1)->setData(1, adjustment);
    // get the count of the number of resources
    int count = getSqlCount(parent, "All", "all");
    parent->child(parent->childCount() - 1)->setData(2, count);
    // the previous steps (there are none) leave no adjustment
    int prev_adjustment = 100;
    parent->child(parent->childCount() - 1)->setData(3, prev_adjustment);
    // just mulitply
    double num = static_cast<double>(count * adjustment * prev_adjustment);
    int newCount = num / 10000.0 + 0.5;
    parent->child(parent->childCount() - 1)->setData(4, newCount);
    // the layer type is all
    parent->child(parent->childCount() - 1)->setData(5, "all");

    return 1;
}

int TreeModel::addAgencyLayer(TreeItem *parent, QVector<int> adjustments, QVector<int>DOIadjustments)
{
    if (adjustments.size() != 3)    {
        adjustments.clear();
        for (int i = 0; i < 3; i++)
            adjustments.push_back(100);
    }

    if (DOIadjustments.size() != 4) {
        DOIadjustments.clear();
        for (int i = 0; i < 4; i++)
            DOIadjustments.push_back(100);
    }

    QVector<string> Agencies;
    Agencies << "FS" << "DOI" << "STATE/LOCAL";

    QVector<string> DOIAgencies;
    DOIAgencies << "BIA" << "BLM" << "FWS" << "NPS";

    for (int i = 0; i < Agencies.size(); i++)   {

        // parent is a selected node
        parent->insertChildren(parent->childCount(), 1, rootItem->columnCount());

        parent->child(parent->childCount() - 1)->setData(0, QVariant((Agencies[i]).c_str()));
        parent->child(parent->childCount() - 1)->setData(1, adjustments[i]);
        int prev_adjustment = cumulativeAdjustment(parent);
        parent->child(parent->childCount() - 1)->setData(3, prev_adjustment);
        parent->child(parent->childCount() - 1)->setData(5, "agency");

        int agencyCount = 0;
        if (Agencies[i] == "DOI")   {
            // Add the DOI Agencies
            TreeItem *DOIparent = parent->child(parent->childCount() -1);

            for (int j = 0; j < DOIAgencies.size(); j++ )  {
                DOIparent->insertChildren(DOIparent->childCount(), 1, rootItem->columnCount());
                DOIparent->child(DOIparent->childCount() - 1)->setData(0, QVariant((DOIAgencies[j]).c_str()));
                DOIparent->child(DOIparent->childCount() - 1)->setData(1, DOIadjustments[j]);
                int count = getSqlCount(DOIparent, "agency", DOIAgencies[j]);
                agencyCount = agencyCount + count;
                DOIparent->child(DOIparent->childCount() - 1)->setData(2, count);
                int DOIprev_adjustment = cumulativeAdjustment(DOIparent);
                DOIparent->child(DOIparent->childCount() - 1)->setData(3, DOIprev_adjustment);
                double num = static_cast<double>(count * DOIadjustments[j] * DOIprev_adjustment);
                int newCount =  num / 10000.0  + 0.5;
                DOIparent->child(DOIparent->childCount() - 1)->setData(4, newCount);
                DOIparent->child(DOIparent->childCount() - 1)->setData(5, "agency");
            }
        }

        else
            agencyCount = getSqlCount(parent, "agency", Agencies[i]);

        parent->child(parent->childCount() - 1)->setData(2, agencyCount);
        double  num = static_cast<double>(agencyCount * adjustments[i] * prev_adjustment);
        int newCount = num / 10000.0 + 0.5;
        parent->child(parent->childCount() - 1)->setData(4, newCount);

    }

    return 1;

}

int TreeModel::addGeneralLayer(TreeItem *parent, QVector<std::pair< string, int> > toBeAdded, string type)
{
    for (int i = 0; i < toBeAdded.size(); i++)   {

        // parent is a selected node
        parent->insertChildren(parent->childCount(), 1, rootItem->columnCount());

        parent->child(parent->childCount() - 1)->setData(0, QVariant((toBeAdded[i].first).c_str()));
        parent->child(parent->childCount() - 1)->setData(1, QVariant(toBeAdded[i].second));
        int count = getSqlCount(parent, type, toBeAdded[i].first);
        parent->child(parent->childCount() - 1)->setData(2, count);
        int prev_adjustment = cumulativeAdjustment(parent);
        parent->child(parent->childCount() - 1)->setData(3, prev_adjustment);
        double num = static_cast<double>(count * toBeAdded[i].second * prev_adjustment);
        int newCount = num / 10000.0 + 0.5;
        parent->child(parent->childCount() - 1)->setData(4, newCount);
        parent->child(parent->childCount() - 1)->setData(5, QVariant(type.c_str()));
    }

    return 1;
}

int TreeModel::getSqlCount(TreeItem *parent, string type, string value)
{
    int count = 0;
    char selectStmt[1000];
    int n = sprintf(selectStmt, "SELECT sum(count) FROM test");

    // while the parent of the TreeItem is not the rootItem
    TreeItem *theParent = parent;
    string theType = type;
    string theValue = value;
    int theNumber = 0;
    QVector<string> valueVector;

    if (theParent->parent())
        n = sprintf(selectStmt, "%s WHERE", selectStmt);

    while (theParent->parent())    {
        if (theType == "agency") {
            if (theValue != "DOI")  {
                n = sprintf(selectStmt, "%s agency = ?%d", selectStmt, theNumber + 1);
                valueVector.push_back(theValue.c_str());
                theNumber++;
            }
        }

        if (theType == "region")  {
            n = sprintf(selectStmt, "%s fpu LIKE ?%d", selectStmt, theNumber + 1);
            theValue.insert(theValue.end(), '%');
            valueVector.push_back(theValue);
            theNumber++;
        }

        if (theType == "state")   {
            n = sprintf(selectStmt, "%s fpu LIKE ?%d", selectStmt, theNumber + 1);
            theValue.insert(theValue.begin(), '%');
            theValue.insert(theValue.end(), '%');
            valueVector.push_back(theValue);
            theNumber++;
        }

        if (theType == "resource")    {
            n = sprintf(selectStmt, "%s resc_type LIKE ?%d", selectStmt, theNumber + 1);
            if (theValue == "SEAT")
                theValue.insert(theValue.begin(), '%');
            else
                theValue.insert(theValue.end(), '%');

            valueVector.push_back(theValue);
            theNumber++;
        }

        QVariant the = theParent->data(5);
        theType = the.toString().toStdString();
        the = theParent->data(0);
        theValue = the.toString().toStdString();
        theParent = theParent->parent();

        if (theParent->parent() && theValue != "DOI")
            n = sprintf(selectStmt, "%s AND", selectStmt);
    }

    // get the number of resources in the analysis (sql step)
    sqlite3_stmt *stmt= NULL;

    // construct the query for different types
    int rc = sqlite3_prepare_v2(db, selectStmt, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return -1;

    // bind the values
    for (int i = 0; i < valueVector.size(); i++)    {
        char *charValue = new char[valueVector[i].length() + 1];
        strcpy(charValue, valueVector[i].c_str());
        sqlite3_bind_text(stmt, i+1, charValue, -1, NULL);
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE && rc != SQLITE_ROW) return 0;

    if (sqlite3_column_type(stmt, 0) == SQLITE_INTEGER)
        count = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);

    return count;
}

int TreeModel::cumulativeAdjustment(TreeItem *parent)
{
    double adjustment = parent->data(1).toDouble();

    TreeItem *grandparent = parent->parent();

    while (grandparent->parent()) {
        adjustment = (adjustment * grandparent->data(1).toDouble()) / 100.0;

        grandparent = grandparent->parent();
    }

    return static_cast<int>(adjustment + 0.5);
}
