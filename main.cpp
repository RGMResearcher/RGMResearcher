#include "mainwindow.h"
#include <QApplication>
#include <fstream>

void to_gv(const result_clusterisation& result, const std::string& file,
           double log_base = 1.5)
{
    std::ofstream fout(file);
    fout << "graph result {\n";
    fout << "\tnode [shape = circle];\n";
    for (auto& i : result.vertexes)
    {
        double size = (i.second.size() > 1) ?
                    (log(i.second.size()) / log(log_base)) : 1.0;
        fout << '\t' << i.first << " [ label = \"" << i.second.size() <<
                "\", height = " << size << ", width = " << size << " ];\n";
    }
    for (auto& i : result.connections)
    {
        for (auto& k : i.second.output)
        {
            for (unsigned n = 0; n < k.second; ++n)
                fout << '\t' << i.first << " -- " << k.first << ";\n";
        }
    }
    fout << '}';
    fout.close();
}

unsigned clustered()
{
    graph gr;
    clusterisator ob;
    std::string str;
    std::pair<unsigned, unsigned> p;
    std::ifstream fin("buddahs.gv");
    while (getline(fin, str))
    {
        unsigned pos = str.find(" -- ");
        std::string tmp_str = str.substr(0, pos);
        p.first = atoi(tmp_str.c_str());
        tmp_str = str.substr(pos + 4, str.size() - pos - 4);
        p.second = atoi(tmp_str.c_str());
        gr.new_vertex(p.first);
        gr.new_vertex(p.second);
        gr.connect(p.first, p.second);
    }
    fin.close();
    ob.init(gr);
    for (unsigned i = 0; ob.next_iteration(); ++i)
    {
        std::string name = "result" + std::to_string(i) + ".gv";
        to_gv(ob.temp_result(), name);
    }
    return 0;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
