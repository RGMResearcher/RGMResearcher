#include "clusterisator.h"

const std::function<void()> default_distribution([](){});

// переиндексирование кластеров(удаление пустых мест)
void clusterisator::reindex_deleted()
{
    int i, k;
    for (i = 0, k = clusters.size() - 1;; ++i, --k)
    {
        for (; i < k && !clusters[i].g.empty(); ++i);
        for (; i < k && clusters[k].g.empty(); --k);
        if (k <= i)
            break;
        clusters[i] = std::move(clusters[k]);
        temp_result[i] = std::move(temp_result[k]);
        for (auto& el : v_to_cl)
        {
            if (el == k)
                el = i;
        }
    }
    clusters.resize(k + 1);
    temp_result.resize(k + 1);
    deleted = std::move(std::queue<unsigned>());
}

// Удаление вершины с индексом gr_index из кластера с индексом cl_index
void clusterisator::erase_from_cl(unsigned cl_index, unsigned gr_index)
{
    clusters[cl_index].g.erase(gr_index);
    v_to_cl[gr_index] = -1;
    if (clusters[cl_index].g.size() == 1)
    {
        v_to_cl[*clusters[cl_index].g.begin()] = -1;
        clusters[cl_index].aij_sum = 0;
        clusters[cl_index].di_sum = 0;
        return;
    }
    // пересчёт параметров
    unsigned aij_temp = 0;
    unsigned cl_size = clusters[cl_index].g.size();
    unsigned gr_size = temp_graph[gr_index].input.size() +
        temp_graph[gr_index].output.size();
    if (cl_size < gr_size)
    {
        for (auto& i : clusters[cl_index].g)
        {
            auto it1 = temp_graph[gr_index].input.find(i);
            if (it1 != temp_graph[gr_index].input.end())
                aij_temp += it1->second;
            it1 = temp_graph[gr_index].output.find(i);
            if (it1 != temp_graph[gr_index].output.end())
                aij_temp += it1->second;
        }
    }
    else
    {
        for (auto& i : temp_graph[gr_index].input)
        {
            auto it1 = clusters[cl_index].g.find(i.first);
            if (it1 != clusters[cl_index].g.end())
                aij_temp += i.second;
        }
        for (auto& i : temp_graph[gr_index].output)
        {
            auto it1 = clusters[cl_index].g.find(i.first);
            if (it1 != clusters[cl_index].g.end())
                aij_temp += i.second;
        }
    }
    clusters[cl_index].aij_sum -= aij_temp + temp_graph[gr_index].loop;
    clusters[cl_index].di_sum -= temp_graph[gr_index].in_d +
        temp_graph[gr_index].out_d;
}

// иннициализация cl(смежные кластеры) и no_cl(смежные вершины,
// не лежащие в кластере)
void clusterisator::_Init(std::map<unsigned, graph_vertex_count_type>& cl,
    std::map<unsigned, graph_vertex_count_type>& no_cl, const std::map<
    unsigned, graph_vertex_count_type>& graph_v)
{
    for (auto& i : graph_v)
    {
        if (v_to_cl[i.first] != -1)
        {
            auto it2 = cl.find(v_to_cl[i.first]);
            if (it2 != cl.end())
                it2->second += i.second;
            else
                cl.insert({ v_to_cl[i.first], i.second });
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

// иннициализация параметров
void clusterisator::_Init()
{
    edge_count = temp_graph.edge_count();
    v_to_cl = std::move(std::vector<int>(temp_graph.size(), -1));
    for (auto& i : temp_graph)
        temp_result.push_back({ i.second, { i.second } });
}

// Перемещает вершину it туда, куда нужно
// Возвращает true, если перемещение произошло
bool clusterisator::move_anywhere(unsigned index, const double& e_factor)
{
    /*
    пройтись по смежным вершинам
    получить множество смежных кластеров и смежных вершин
    определить предпочтительный для присоединения кластер
    определить предпочтительную для присоединения вершину
    выполнить либо cluster_add либо make_cluster(модулярность не должна ухудшиться),
    перед этим выполнив cluster_erase если нужно
    */
    std::map<unsigned, graph_vertex_count_type> cl; // смежный кластер на aij_temp(кол. рёбер между it и кластером)
    std::map<unsigned, graph_vertex_count_type> no_cl; // безкластерная вершина на кол. рёбер между ней и it
    _Init(cl, no_cl, temp_graph[index].input);
    _Init(cl, no_cl, temp_graph[index].output);

    std::pair<double, unsigned> max;
    std::map<unsigned, graph_vertex_count_type>::iterator it1;
    char flag = !no_cl.empty();
    if (flag == 1)
    {
        it1 = no_cl.begin();
        max = { d_modular(index, it1->first, e_factor), it1->first };
        // d_mod на номер вершины

        for (++it1; it1 != no_cl.end(); ++it1)
        {
            std::pair<double, unsigned> ifmax = {
                d_modular(index, it1->first, e_factor), it1->first
            };
            if (ifmax > max)
                max = ifmax;
        }
    }
    // max - наилучшее внекластерное
    if (!cl.empty())
    {
        it1 = cl.begin();
        if (flag == 0)
        {
            max = { d_modular(index, it1->first,
                it1->second, e_factor), it1->first };
            ++it1;
            flag = 2;
        }
        for (; it1 != cl.end(); ++it1)
        {
            std::pair<double, unsigned> tmp = { d_modular(index,
                it1->first, it1->second, e_factor), it1->first };
            if (tmp > max)
            {
                flag = 2;
                max = tmp;
            }
        }
    }
    if (flag == 0)
        return false;
    // clcl если != 1 номер текущего кластера; -1 - если вне кластера
    if (v_to_cl[index] != -1)
    {
        if (d_modular(v_to_cl[index], e_factor) >= max.first)
            return false;
        this->erase_from_cl(v_to_cl[index], index);
    }
    if (flag == 1)
        this->make_cluster(index, max.second);
    else if (flag == 2)
        this->add_to_cl(max.second, index, cl[max.second]);

    return true;
}

// Создаёт метаграф
void clusterisator::meta_graph()
{
    graph meta_gr;
    std::vector<std::list<unsigned>> tmp_result(clusters.size());
    for (unsigned i = 0; i < clusters.size(); ++i)
    {
        auto it = meta_gr.new_vertex(i);
        meta_gr.connect(it, it, clusters[i].aij_sum);
    }
    for (auto& i : temp_graph)
    {
        if (clusters[v_to_cl[i.second]].g.empty())
            continue;
        auto& it = tmp_result[v_to_cl[i.second]];
        for (auto& j : temp_graph[i.second].output)
        {
            if (v_to_cl[i.second] == v_to_cl[j.first])
                it.splice(it.end(), temp_result[j.first]);
            else
            {
                meta_gr.connect_by_name(v_to_cl[j.first],
                    v_to_cl[i.second], j.second);
            }
        }
        for (auto& j : temp_graph[i.second].input)
        {
            unsigned fr = v_to_cl[j.first];
            if (v_to_cl[i.second] == fr)
                it.splice(it.end(), temp_result[j.first]);
        }
    }

    for (unsigned i = 0; i < temp_result.size(); ++i) // добиваем остатки result
    {
        if (!temp_result[i].empty())
        {
            auto& it = tmp_result[v_to_cl[temp_graph.find(i)->second]];
            it.splice(it.end(), temp_result[i]);
        }
    }
    temp_result = std::move(tmp_result);
    temp_graph = std::move(meta_gr);
}

// Переход на следующий уровень(следующая итерация)
bool clusterisator::next_iteration(const double& e_factor)
{
    clusters.clear();
    bool contin;
    do {
        contin = false;
        for (auto& i : temp_graph)
        {
            contin = (move_anywhere(i.second, e_factor) || contin);
        }
    } while (contin);

    bool ret = (clusters.size() > 1);
    if (ret)
    {
        this->reindex_deleted();
        meta_graph();
    }
    v_to_cl.clear();
    return ret;
}
