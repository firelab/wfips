#include <QtGui>

#include "treemodel.h"

int main(int argc, char *argv[])
{
   QApplication app(argc, argv);

   /*QFile file(":/default.txt");
   file.open(QIODevice::ReadOnly);
   TreeModel model(file.readAll());
   file.close();*/

   TreeModel model;
   model.addAllLayer(model.getRootItem(), 120);

   QVector<int> agencyAdj;
   agencyAdj.push_back(80);
   agencyAdj.push_back(90);
   agencyAdj.push_back(100);

   QVector<int> DOIagencyAdj;
   DOIagencyAdj.push_back(90);
   DOIagencyAdj.push_back(120);
   DOIagencyAdj.push_back(100);
   DOIagencyAdj.push_back(80);

   TreeItem *parent = model.getRootItem()->child(0);

   model.addAgencyLayer(parent, agencyAdj, DOIagencyAdj);

   QVector<std::pair< string, int> > toBeAdded;

   toBeAdded.push_back(std::make_pair("NR", 110));
   toBeAdded.push_back(std::make_pair("GB", 95));
   toBeAdded.push_back(std::make_pair("CA", 88));
   toBeAdded.push_back(std::make_pair("NW", 105));
   toBeAdded.push_back(std::make_pair("SW", 95));

   TreeItem *FSparent = parent->child(0);

   model.addGeneralLayer(FSparent, toBeAdded, "region");

   toBeAdded.clear();
   toBeAdded.push_back(std::make_pair("MT", 90));
   toBeAdded.push_back(std::make_pair("CA", 105));
   toBeAdded.push_back(std::make_pair("WY", 50));
   toBeAdded.push_back(std::make_pair("WA", 110));

   TreeItem *SLparent = parent->child(2);

   model.addGeneralLayer(SLparent, toBeAdded, "state");

   toBeAdded.clear();
   toBeAdded.push_back(std::make_pair("CRW", 130));
   toBeAdded.push_back(std::make_pair("EN", 95));
   toBeAdded.push_back(std::make_pair("SEAT", 150));

   TreeItem *MTparent = FSparent->child(0);

   model.addGeneralLayer(MTparent, toBeAdded, "resource");

   QTreeView view;
   view.setModel(&model);
   view.setWindowTitle(QObject::tr("Simple Tree Model"));
   view.show();

   return app.exec();
}
