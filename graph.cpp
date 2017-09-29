#include "graph.h"

// Удаляет петли
unsigned graph::erase_loops()
{
	unsigned ret = 0;
	for (auto& i : this->v)
	{
		m -= i.loop;
		ret += i.loop;
		i.loop = 0;
	}
	return ret;
}

void graph::erase(const std::pair<unsigned, unsigned>& p)
{
    for (auto& i : v[p.second].input)
        this->disconnect_by_index(p.second, i.first, i.second);
    for (auto& i : v[p.second].output)
        this->disconnect_by_index(i.first, p.second, i.second);
    v[p.second] = std::move(vertex());
    m -= v[p.second].loop;
    deleted.push(p.second);
    reindex.erase(p.first);
}

// Установка n рёбер it_f --> it_s
void graph::connect_by_index(unsigned it_f,
    unsigned it_s, graph_vertex_count_type n)
{
    if (it_f == it_s)
    {
        v[it_f].loop += n;
    }
    else
    {
        auto it = v[it_f].output.find(it_s);
        if (it == v[it_f].output.end())
        {
            v[it_f].output.insert({ it_s, n });
            v[it_s].input.insert({ it_f, n });
        }
        else
        {
            it->second += n;
            v[it_s].input[it_f] += n;
        }
    }
    v[it_f].out_d += n;
    v[it_s].in_d += n;
    m += n;
}

graph_vertex_count_type graph::count_by_name(unsigned first, unsigned second) const
{
    auto it_f = this->find(first);
    if (it_f == this->end())
        return 0;
    if (first == second)
        return this->count(it_f, it_f);
    auto it_s = this->find(second);
    if (it_s != this->end())
        return this->count(it_f, it_s);
    return 0;
}

// Удаление n рёбер it_f --> it_s
// Если рёбер меньше, чем n - они удалятся все
// Возвращает количество удалённых рёбер
graph_vertex_count_type graph::disconnect_by_index(unsigned it_f,
    unsigned it_s, graph_vertex_count_type n = 1, bool ndel)
{
    graph_vertex_count_type ret = n;
    if (it_f == it_s)
    {
        if (v[it_f].loop > n)
            v[it_f].loop -= n;
        else
        {
            ret = v[it_f].loop;
            v[it_f].loop = 0;
        }
    }
    else
    {
        auto it_fs = v[it_f].output.find(it_s);
        if (it_fs == v[it_f].output.end())
            return 0;
        auto it_sf = v[it_s].input.find(it_f);
        if (it_fs->second > n)
        {
            it_fs->second -= n;
            it_sf->second -= n;
        }
        else
        {
            ret = it_fs->second;
            if (ndel)
            {
                v[it_s].input.erase(it_sf);
                v[it_f].output.erase(it_fs);
            }
            else
            {
                it_fs->second = 0;
                it_sf->second = 0;
            }
        }
    }
    v[it_f].out_d -= ret;
    v[it_s].in_d -= ret;
    m -= ret;
    return ret;
}

// Удаление n рёбер first --> second
// Если рёбер меньше, чем n - они удалятся все
// Возвращает количество удалённых рёбер
graph_vertex_count_type graph::disconnect_by_name(unsigned first,
    unsigned second, graph_vertex_count_type n, bool ndel)
{
    auto it_f = this->find(first);
    if (it_f != this->end())
    {
        if (first == second)
            return this->disconnect(it_f, it_f, n, ndel);
        else
        {
            auto it_s = this->find(second);
            if (it_s != this->end())
                return this->disconnect(it_f, it_s, n, ndel);
        }
    }
    return 0;
}

// Объединение двух вершин(second перехоит в first)
void graph::merge(const std::pair<unsigned, unsigned>& it_f,
    const std::pair<unsigned, unsigned>& it_s)
{
    graph_vertex_count_type count = this->count_by_index(it_f.second, it_s.second);
    if (count > 0)
    {
        disconnect_by_index(it_f.second, it_s.second, count);
        connect_by_index(it_f.second, it_f.second, count);
    }
    count = this->count_by_index(it_s.second, it_f.second);
    if (count > 0)
    {
        disconnect_by_index(it_s.second, it_f.second, count);
        connect_by_index(it_f.second, it_f.second, count);
    }
    count = this->count_by_index(it_s.second, it_s.second);
    if (count > 0)
    {
        disconnect_by_index(it_s.second, it_s.second, count);
        connect_by_index(it_f.second, it_f.second, count);
    }
    // пересадка рёбер
    for (auto& i : v[it_s.second].input)
        connect_by_index(i.first, it_f.second, i.second);
    for (auto& i : v[it_s.second].output)
        connect_by_index(it_f.second, i.first, i.second);
    this->erase_by_index(it_s.second);
}


// Вывод графа в виде списка смежности
std::ostream& operator<<(std::ostream& os, const graph& gr)
{
    if (gr.size() == 0)
        return os;
    throw(std::string("not worked"));
    return os;
}

// Удаляет петли
void erase_loops(graph& gr)
{
    for (auto it = gr.begin(); it != gr.end(); ++it)
    {
        graph_vertex_count_type count = gr.count(it, it);
        if (count > 0)
            gr.disconnect(it, it, count);
    }
}
