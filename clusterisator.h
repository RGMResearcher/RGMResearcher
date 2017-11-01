#pragma once
#ifndef _CLUSTERISATOR_H_
#define _CLUSTERISATOR_H_

#include <set>
#include <map>
#include <vector>
#include <list>
#include <memory>
#include <math.h>
#include "graph.h"

struct cluster
{
public:
	long long di_sum;
	int aij_sum, loops;
	unsigned g;

	cluster() :di_sum(0), aij_sum(0), loops(0), g(0){}

	void clear()
	{
		g = di_sum = aij_sum = 0;
	}
};

typedef graph<unsigned, unsigned, hash_ptr> graph_type;

struct result_clusterisation
{
	// Сруктура связей метаграфа
	graph_type const& connections;
	// Структура объединения(для дендограммы)
	std::map<unsigned, cluster> const& clusters;
	// Состав вешин в каждом кластере
	std::map<unsigned, std::list<unsigned>> const& vertexes;
	// число петель в исходном графе
	unsigned loop_count;

	result_clusterisation(graph_type const& gr, std::map<unsigned,
		cluster> const& cl, std::map<unsigned,
		std::list<unsigned >> const& temp_result, unsigned loops) :clusters(cl),
		connections(gr), vertexes(temp_result), loop_count(loops){}
};

class clusterisator
{
private:
	unsigned edge_count, loop_count;
	std::vector<int> v_to_cl;
	graph_type temp_graph;
	// номер кластера на кластер
	std::map<unsigned, cluster> clusters;
	// номер кластера на список верщин в нём
	std::map<unsigned, std::list<unsigned>> temp_result;

	// Удаление вершины из кластера
	void erase_from_cl(std::map<unsigned, cluster>::iterator&,
		unsigned, unsigned);

	// Добавляет вершину it в кластер с итератором cl
	// aij_sum_delt - кол. рёбер между вставляемой вершиной и
	// элементами кластера cl
	void add_to_cl(const std::map<unsigned, cluster>::iterator& cl,
		unsigned gr_index, unsigned aij_sum_delt)
	{
		v_to_cl[gr_index] = cl->first;
		++cl->second.g;
		cl->second.aij_sum += aij_sum_delt + temp_graph[gr_index].loop;
		cl->second.di_sum += (temp_graph[gr_index].in_d + temp_graph[gr_index].out_d);
	}

	// Для тех случаев, когда у вершины нет смежных рёбер
	// Возвращает номер созданного кластера.
	unsigned make_cluster(unsigned gr_index)
	{
		unsigned ret = (clusters.empty()) ? 0 : (clusters.rbegin()->first + 1);
		auto it = clusters.insert({ ret, cluster() }).first;
		add_to_cl(it, gr_index, 0);
		return ret;
	}

	// Объединение двух вершин в новый кластер.
	// Возвращает номер созданного кластера.
	unsigned make_cluster(unsigned gr_first, unsigned gr_second)
	{
		unsigned ret = (clusters.empty()) ? 0 : (clusters.rbegin()->first + 1);
		auto it = clusters.insert({ ret, cluster() }).first;
		add_to_cl(it, gr_first, temp_graph.count_by_index(gr_second, gr_first));
		add_to_cl(it, gr_second, temp_graph.count_by_index(gr_first, gr_second));
		return ret;
	}

	void meta_graph();
	void _Init(std::map<unsigned, unsigned>&, const std::map<unsigned,
		graph_type::edge_type>&);
	void _Init();

	// Возврат. знач. не является значением модулярность
	// Только для операций сравнения < > =
	int d_modular(const cluster& cl, unsigned current_aij, unsigned gr_index)
	{
		int cur = temp_graph[gr_index].in_d + temp_graph[gr_index].out_d;
		return 2 * (int)edge_count*((int)current_aij - (int)temp_graph[
			gr_index].loop) - cur*((int)cl.di_sum - (int)cur - (int)current_aij);
	}

	// Возврат. знач. не является значением модулярность
	// Только для операций сравнения < > =
	int d_modular(unsigned gr_first, unsigned gr_second)
	{
		return (int)(2 * edge_count*(temp_graph.count_by_index(gr_first, gr_second) +
			temp_graph.count_by_index(gr_second, gr_first))) -
			(int)(temp_graph[gr_first].in_d + temp_graph[gr_first].out_d)*(int)(
			temp_graph[gr_second].in_d + temp_graph[gr_second].out_d);
	}

	// Возврат. знач. не является значением модулярность
	// Только для операций сравнения < > =
	int d_modular(unsigned gr_index, const std::map<unsigned, cluster>::iterator& it_cl,
		unsigned aij_temp)
	{
		return (int)(2 * edge_count*aij_temp) - (int)((temp_graph[gr_index].in_d +
			temp_graph[gr_index].out_d)*it_cl->second.di_sum);
	}


	int modular(const cluster& cl)
	{
		return 4 * loop_count * cl.aij_sum - cl.di_sum*cl.di_sum;
	}

	int modular(unsigned gr_index, const cluster& cl, int aij_temp)
	{
		return 4 * loop_count*(cl.aij_sum + temp_graph[gr_index].loop + aij_temp) -
			pow(cl.di_sum + temp_graph[gr_index].in_d +
			temp_graph[gr_index].out_d, 2);
	}

	int modular(unsigned gr_index)
	{
		return 4 * loop_count*temp_graph[gr_index].loop - pow(temp_graph[gr_index].in_d
			+ temp_graph[gr_index].out_d, 2);
	}

	int modular(unsigned gr_1, unsigned gr_2, int aij_temp)
	{
		return 4 * loop_count*(temp_graph[gr_1].loop + temp_graph[gr_2].loop + aij_temp)
			- pow(temp_graph[gr_1].in_d + temp_graph[gr_1].out_d +
			temp_graph[gr_2].in_d + temp_graph[gr_2].out_d, 2);
	}

	bool move_anywhere(unsigned, const double &e_factor = 4.0);

public:
	typedef std::function<void()> distributor;
	static const distributor default_distribution;

	clusterisator(){}
	void init(graph_type&& gr)
	{
		temp_graph = std::move(gr);
		_Init();
	}

	void init(const graph_type& gr)
	{
		temp_graph = gr;
		_Init();
	}
	// первичное разбиение на кластеры
	void first_partition() {}

	bool next_iteration(unsigned = 1, const double &e_factor = 4.0);

	result_clusterisation result() const
	{
		return result_clusterisation(temp_graph, clusters, temp_result, loop_count);
	}

	virtual ~clusterisator(){}
};

double modular(const result_clusterisation&);

#endif // _CLUSTERISATOR_H_
