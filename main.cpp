#include "mainwindow.h"
#include <QApplication>

#include "graphutility.h"
unsigned clustered()
{
    graph gr = clustered_graph(20, 4, 6).first;
    clusterisator ob;
    unsigned t1 = clock();
    /*
    string str;
    pair<unsigned, unsigned> p;
    t1 = clock();
    ifstream fin("UU2fh.txt");
    while (getline(fin, str))
    {
    unsigned pos = str.find(" -- ");
    string tmp_str = str.substr(0, pos);
    p.first = atoi(tmp_str.c_str());
    tmp_str = str.substr(pos + 4, str.size() - pos - 4);
    p.second = atoi(tmp_str.c_str());
    gr.new_vertex(p.first);
    gr.new_vertex(p.second);
    gr.connect(p.first, p.second);
    }
    fin.close();
    t1 = clock() - t1;
    cout << gr.size() << endl;
    cout << t1 << "\tGraph loaded.\n";
    cout << "Clusterisation started...\n";
    t1 = clock();
    */
    ob(gr);
    t1 = clock() - t1;
    return t1;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
