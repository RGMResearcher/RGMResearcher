#include "clusterisator.h"

// Удаление вершины current из кластера it
void clusterisator::erase_from_cl(std::map<unsigned, cluster>::
	iterator& it, unsigned current, unsigned aij_temp)
{
	if (--it->second.g == 0)
	{
		clusters.erase(it);
		return;
	}
	it->second.aij_sum -= aij_temp + temp_graph[current].loop;
	it->second.di_sum -= (temp_graph[current].in_d + temp_graph[current].out_d);
}

// иннициализация cl(смежные кластеры)
void clusterisator::_Init(std::map<unsigned, unsigned>& cl, const std::map<
	unsigned, graph_type::edge_type>& graph_v)
{
	for (auto& i : graph_v)
	{
		auto it2 = cl.find(v_to_cl[i.first]);
		if (it2 != cl.end())
			it2->second += i.second;
		else
			cl.insert({ v_to_cl[i.first], i.second });
	}
}

void clusterisator::_Init()
{
	edge_count = temp_graph.edge_count();
	loop_count = 0;
	temp_result.clear();
	clusters.clear();

	v_to_cl.resize(temp_graph.size());
	for (auto& i : temp_graph)
	{
		temp_result.insert({ i.first, { i.first } });
		loop_count += i.second.loop;
		make_cluster(i.first);
	}
}

// Перемещает вершину it туда, куда нужно
// Возвращает true, если перемещение произошло
bool clusterisator::move_anywhere(unsigned gr_index, const double& e_factor)
{
	/*
	пройтись по смежным вершинам
	получить множество смежных кластеров и смежных вершин
	определить предпочтительный для присоединения кластер
	определить предпочтительную для присоединения вершину
	выполнить либо cluster_add либо make_cluster(модулярность не должна ухудшиться),
	перед этим выполнив cluster_erase если нужно
	*/
	std::map<unsigned, unsigned> cl; // смежный кластер на aij_temp(кол. рёбер между it и кластером)
	_Init(cl, temp_graph[gr_index].input);
	_Init(cl, temp_graph[gr_index].output);
	unsigned current_aij = 0;
	auto it = cl.find(v_to_cl[gr_index]);
	if (it != cl.end())
	{
		current_aij = it->second;
		cl.erase(v_to_cl[gr_index]);
	}
	std::pair<int, unsigned> max;
	std::map<unsigned, unsigned>::iterator it1;
	bool flag = !cl.empty();
	// max - наилучшее внекластерное
	std::map<unsigned, cluster>::iterator cl_it = clusters.end();
	if (cl.empty())
		return false;
	it1 = cl.begin();
	max = { d_modular(gr_index, (cl_it = clusters.find(it1->first)),
		it1->second), it1->first };
	for (++it1; it1 != cl.end(); ++it1)
	{
		std::map<unsigned, cluster>::iterator it_tmp;
		std::pair<int, unsigned> tmp = { d_modular(gr_index,
			(it_tmp = clusters.find(it1->first)), it1->second), it1->first };
		if (tmp.first > max.first)
		{
			max = tmp;
			cl_it = std::move(it_tmp);
		}
	}
	if (max.first <= 0)
		return false;
	auto yhg = clusters.find(v_to_cl[gr_index]);
	//int p = d_modular(yhg->second, max.second, gr_index);
	//if (p >= max.first)
	//	return false;
	this->erase_from_cl(yhg, gr_index, current_aij);
	this->add_to_cl(cl_it, gr_index, cl[max.second]);
	return true;
}

// Создаёт метаграф
void clusterisator::meta_graph()
{
	graph_type meta_gr;
	std::map<unsigned, std::list<unsigned>> tmp_result;

	for (auto& i : clusters)
	{
		unsigned index = meta_gr.new_vertex(i.first)->first;
		tmp_result.insert({ i.first, {} });
		i.second.g = 1;
	}
	for (auto& i : temp_graph)
	{
		unsigned out = v_to_cl[i.first];
		auto it = tmp_result.find(v_to_cl[i.first]);
		it->second.splice(it->second.end(), temp_result[i.second.name()]);
		meta_gr.connect_by_name(out, out, i.second.loop);
		for (auto& j : i.second.output)
		{
			unsigned fr = v_to_cl[j.first];
			meta_gr.connect_by_name(out, fr, j.second);
		}
	}
	temp_result = std::move(tmp_result);
	temp_graph = std::move(meta_gr);
	v_to_cl.resize(temp_graph.size());
	for (auto& i : temp_graph)
		v_to_cl[i.first] = i.second.name();
}

bool clusterisator::next_iteration(unsigned clusters_count,
	const double& e_factor)
{
	unsigned contin, contin2 = temp_graph.size(); bool contin_ch = false;
	for (;;)
	{
		contin = 0;
		for (auto& i : temp_graph)
		{
			if (move_anywhere(i.first, e_factor))
				++contin;
		}
		contin_ch = contin_ch || (contin > 0);
		if (contin == 0)// || contin > contin2)
			break;
		else
			contin2 = contin;
	}
	for (;;)
	{
		contin = 0;
		for (int h = temp_graph.size() - 1; h >= 0; --h)
		{
			if (move_anywhere(h, e_factor))
				++contin;
		}
		contin_ch = contin_ch || (contin > 0);
		if (contin == 0)// || contin > contin2)
			break;
		else
			contin2 = contin;
	}
	bool ret = (clusters.size() > clusters_count && contin_ch);
	if (ret)
		meta_graph();
	return ret;
}

double modular(const result_clusterisation& res)
{
	double ret1 = 0, ret2 = 0;
	for (auto& i : res.connections)
	{
		ret1 += i.second.loop;
		ret2 += pow(i.second.in_d +	i.second.out_d, 2);
	}
	double t = 1.0 / (2.0 * res.connections.edge_count());
	return t*(2.0*ret1 - res.loop_count - t*ret2);
}
