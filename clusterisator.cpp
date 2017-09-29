#include "clusterisator.h"

const std::function<void()> default_distribution([](){});

// Удаление вершины current из кластера it
void clusterisator::erase_from_cl(std::map<unsigned, cluster>::
	iterator& it, unsigned current, unsigned aij_temp)
{
	it->second.g.erase(current);
	v_to_cl[current] = -1;
	if (it->second.g.size() == 0)
	{
		clusters.erase(it);
		return;
	}
	// пересчёт параметров
	/*unsigned aij_temp = 0;
	if (it->second.g.size() < temp_graph[current].in_d +
	temp_graph[current].out_d)
	{
	for (auto& i : it->second.g)
	{
	auto it1 = temp_graph[current].input.find(i);
	if (it1 != temp_graph[current].input.end())
	aij_temp += it1->second;
	it1 = temp_graph[current].output.find(i);
	if (it1 != temp_graph[current].output.end())
	aij_temp += it1->second;
	}
	}
	else
	{
	for (auto& i : temp_graph[current].input)
	{
	auto it1 = it->second.g.find(i.first);
	if (it1 != it->second.g.end())
	aij_temp += i.second;
	}
	for (auto& i : temp_graph[current].output)
	{
	auto it1 = it->second.g.find(i.first);
	if (it1 != it->second.g.end())
	aij_temp += i.second;
	}
	}*/
	if (it->second.aij_sum < aij_temp)
		throw "error";
	it->second.aij_sum -= aij_temp + temp_graph[current].loop;
	it->second.di_sum -= temp_graph[current].in_d + temp_graph[current].out_d;
}

// иннициализация cl(смежные кластеры) и no_cl(смежные вершины,
// не лежащие в кластере)
void clusterisator::_Init(std::map<unsigned, unsigned>& cl,
	std::map<unsigned, unsigned>& no_cl, const std::map<
	unsigned, graph_vertex_count_type>& graph_v)
{
	for (auto& i : graph_v)
	{
		int it1 = v_to_cl[i.first];
		if (it1 != -1)
		{
			auto it2 = cl.find(it1);
			if (it2 != cl.end())
				it2->second += i.second;
			else
				cl.insert({ it1, i.second });
		}
		else
		{
			auto it2 = no_cl.find(i.first);
			if (it2 != no_cl.end())
				it2->second += i.second;
			else
				no_cl.insert({ i.first, i.second });
		}
	}
}

void clusterisator::_Init()
{
	edge_count = temp_graph.edge_count();
	v_to_cl.resize(temp_graph.size());
	for (auto& i : v_to_cl)
		i = -1;
	loop_count = 0;
	for (auto& i : temp_graph)
	{
		temp_result.insert({ i.first, { i.first } });
		loop_count += temp_graph.count_by_index(i.second, i.second);
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
	std::map<unsigned, unsigned> no_cl; // безкластерная вершина на кол. рёбер между ней и it
	_Init(cl, no_cl, temp_graph[gr_index].input);
	_Init(cl, no_cl, temp_graph[gr_index].output);
	int clcl = v_to_cl[gr_index];
	unsigned current_aij;
	if (clcl != -1)
	{
		auto it = cl.find(clcl);
		if (it != cl.end())
		{
			current_aij = it->second;
			cl.erase(clcl);
		}
		else
			current_aij = 0;
	}
	else
		current_aij = 0;
	std::pair<int, unsigned> max;
	std::map<unsigned, unsigned>::iterator it1;
	char flag = !no_cl.empty();
	if (flag == 1)
	{
		it1 = no_cl.begin();
		max = { d_modular(gr_index, it1->first, e_factor), it1->first };
		// d_mod на номер вершины

		for (++it1; it1 != no_cl.end(); ++it1)
		{
			std::pair<int, unsigned> ifmax = {
				d_modular(gr_index, it1->first, e_factor), it1->first
			};
			if (ifmax > max)
				max = ifmax;
		}
	}
	// max - наилучшее внекластерное
	std::map<unsigned, cluster>::iterator cl_it = clusters.end();
	if (!cl.empty())
	{
		it1 = cl.begin();
		if (flag == 0)
		{
			max = { d_modular(gr_index, cl_it = clusters.find(it1->first),
				it1->second, e_factor), it1->first };
			++it1;
			flag = 2;
		}
		for (; it1 != cl.end(); ++it1)
		{
			std::map<unsigned, cluster>::iterator it_tmp;
			std::pair<int, unsigned> tmp = { d_modular(gr_index,
				it_tmp = clusters.find(it1->first), it1->second, e_factor), it1->first };
			if (tmp > max)
			{
				flag = 2;
				max = tmp;
				cl_it = std::move(it_tmp);
			}
		}
	}
	// clcl если != -1 номер текущего кластера; -1 - если вне кластера
	if (clcl != -1)
	{
		if (flag == 0 || max.first <= 0)
			return false;
		auto yhg = clusters.find(clcl);
		auto p = d_modular(yhg->second, current_aij, gr_index, e_factor);
		if (p >= max.first)
			return false;
		this->erase_from_cl(yhg, gr_index, current_aij);
	}
	if (flag == 1)
		this->make_cluster(gr_index, max.second);
	else if (flag == 2)
		this->add_to_cl(cl_it, gr_index, cl[max.second]);
	else
		make_cluster(gr_index);

	return true;
}

// Создаёт метаграф
void clusterisator::meta_graph()
{
	graph meta_gr;
	std::map<unsigned, std::list<unsigned>> tmp_result;
	for (auto& i : clusters)
	{
		meta_gr.new_vertex(i.first);
		tmp_result.insert({ i.first, {} });
	}
	for (auto& i : temp_graph)
	{
		unsigned out = v_to_cl[i.second];
		auto it = tmp_result.find(v_to_cl[i.second]);
		it->second.splice(it->second.end(), temp_result[i.first]);
		meta_gr.connect_by_name(out, out, temp_graph[i.second].loop);
		for (auto& j : temp_graph[i.second].output)
		{
			unsigned fr = v_to_cl[j.first];
			meta_gr.connect_by_name(out, fr, j.second);
		}
	}
	temp_result = std::move(tmp_result);
	temp_graph = std::move(meta_gr);
	v_to_cl.resize(temp_graph.size());
	for (auto& i : v_to_cl)
		i = -1;
}

bool clusterisator::next_iteration(unsigned clusters_count,
	const double& e_factor)
{
	clusters.clear();
	unsigned contin, contin_2 = temp_graph.size() * 2;
	for (;;)
	{
		contin = 0;
		for (unsigned i = 0; i < temp_graph.size(); ++i)
		{
			if (move_anywhere(i, e_factor))
				++contin;
		}
		if (contin <= contin_2 && contin > 0)
			contin_2 = contin;
		else
			break;
	}

	bool ret = (clusters.size() > clusters_count &&
		clusters.size() < temp_graph.size());
	if (ret)
		meta_graph();
	return ret;
}

double modular(const result_clusterisation& res)
{
	double ret1 = 0, ret2 = 0;
	for (auto& i : res.connections)
	{
		ret1 += res.connections[i.second].loop;
		ret2 += pow(res.connections[i.second].in_d +
			res.connections[i.second].out_d, 2);
	}
	double t = 1.0 / (2.0 * res.connections.edge_count());
	return t*(2.0*ret1 - res.loop_count - t*ret2);
}
