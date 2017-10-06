#include "mainwindow.h"
#include <QApplication>

#include <string>
#include "graphutility.h"
using namespace std;

void example()
{
	fstream gr_file("graph.gv");
	graph gr; string str;
	unsigned in, out;
	while (getline(gr_file, str))
	{
		int pos = str.find(" -- ");
		if (pos == -1)
			continue;
		in = atoi(str.substr(0, pos).c_str());
		out = atoi(str.substr(pos + 4, str.size() - pos - 4).c_str());
		// добавление ребра в граф(с созданием вершин in и out)
		gr.connect(gr.new_vertex(in), gr.new_vertex(out));
	}
	clusterisator cl;
	cl.init(gr); // передаём граф
	while(cl.next_iteration()) // пока улучшается продолжаем
		;
	double mod1 = modular(cl.result()); // Вычисляем модулярность на
	// основе результата кластеризации

	gr.clear();
	auto t = clustered_graph(20, 4, 5, 0.8);// число рёбер, мин. размер кластера,
	// макс. размер кластера, вероятность появления ребра внутри кластера

	double mod2 = modular(t); // модулярность сгенерированного графа
	cl.init(t.gr);
	while(cl.next_iteration()) // пока улучшается продолжаем
		;
	double mod3 = modular(cl.result()); // модулярность того же графа, но переразбитого на кластеры

	// далее можно сравнить mod2 и mod3
}

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	MainWindow w;
	w.show();

	return a.exec();
}
