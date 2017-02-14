#include "graph.h"

// Установка n связей first --> second
void graph::connect(unsigned first, unsigned second, unsigned n)
{
	auto it_f = v.find(first);
	auto it_s = v.find(second);
	if (it_f != v.end() && it_s != v.end())
		connect(it_f, it_s, n);
}

// Удаление n рёбер first --> second
// Если рёбер меньше, чем n - они удалятся все
// Возвращает количество удалённых рёбер
unsigned graph::disconnect(unsigned first, unsigned second,
    unsigned n = 1, bool ndel)
{
	auto it_f = v.find(first);
	auto it_s = v.find(second);
	if (it_f != v.end() && it_s != v.end())
		return disconnect(it_f, it_s, n, ndel);
	else
		return 0;
}

// Установка n рёбер it_f --> it_s
void graph::connect(const graph::iterator& it_f,
    const graph::iterator& it_s, unsigned n)
{
	if (it_f->first == it_s->first)
	{
		it_f->second.loop += n;
	}
	else if (it_f->second.output.find(it_s->first) ==
		it_f->second.output.end())
	{
		it_f->second.output.insert({ it_s->first, n });
		it_s->second.input.insert({ it_f->first, n });
	}
	else
	{
		it_f->second.output[it_s->first] += n;
		it_s->second.input[it_f->first] += n;
	}
	it_f->second.out_d += n;
	it_s->second.in_d += n;
	m += n;
}

// Удаление n рёбер it_f --> it_s
// Если рёбер меньше, чем n - они удалятся все
// Возвращает количество удалённых рёбер
unsigned graph::disconnect(const graph::iterator& it_f,
    const graph::iterator& it_s, unsigned n = 1, bool ndel)
{
	unsigned ret = n;
	if (it_f->first == it_s->first)
	{
		if (it_f->second.loop > n)
			it_f->second.loop -= n;
		else
		{
			ret = it_f->second.loop;
			it_f->second.loop = 0;
		}
	}
	else
	{
		auto it_fs = it_f->second.output.find(it_s->first);
		if (it_fs == it_f->second.output.end())
			return 0;
		auto it_sf = it_s->second.input.find(it_f->first);
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
				it_s->second.input.erase(it_sf);
				it_f->second.output.erase(it_fs);
			}
			else
			{
				it_fs->second = 0;
				it_sf->second = 0;
			}
		}
	}
	it_f->second.out_d -= ret;
	it_s->second.in_d -= ret;
	m -= ret;
	return ret;
}

// Объединение двух вершин(second перехоит в first)
void graph::merge(const graph::iterator& it_f,
	const graph::iterator& it_s, bool _Ndel = true)
{
	unsigned count = this->count(it_f, it_s);
	if (count > 0)
	{
		disconnect(it_f, it_s, count);
		connect(it_f, it_f, count);
	}
	count = this->count(it_s, it_f);
	if (count > 0)
	{
		disconnect(it_s, it_f, count);
		connect(it_f, it_f, count);
	}
	count = this->count(it_s, it_s);
	if (count > 0)
	{
		disconnect(it_s, it_s, count);
		connect(it_f, it_f, count);
	}
	// пересадка рёбер
	for (auto it = it_s->second.input.begin(); !it_s->second.input.empty();
		it = it_s->second.input.begin())
	{
		auto it1 = v.find(it->first);
		count = this->count(it1, it_s);
		disconnect(it1, it_s, count);
		connect(it1, it_f, count);
	}
	for (auto it = it_s->second.output.begin(); !it_s->second.output.empty();
		it = it_s->second.output.begin())
	{
		auto it1 = v.find(it->first);
		count = this->count(it_s, it1);
		disconnect(it_s, it1, count);
		connect(it_f, it1, count);
	}
	if (_Ndel)
		v.erase(it_s);
}

// Объединение двух вершин(second перехоит в first)
void graph::merge(unsigned first, unsigned second, bool _Ndel)
{
	auto it_f = v.find(first);
	auto it_s = v.find(second);
	if (it_f != v.end() && it_s != v.end())
		merge(it_f, it_s, _Ndel);
}

// Вывод графа в виде списка смежности
std::ostream& operator<<(std::ostream& os, const graph& gr)
{
	if (gr.size() == 0)
		return os;
	auto it = gr.begin();
	os << it->first;
	for (++it; it != gr.end(); ++it)
		os << std::ends << it->first;
	for (auto it = gr.begin(); it != gr.end(); ++it)
	{
		for (auto i : it->second.output)
			os << std::endl << it->first << std::ends <<
			i.first << std::ends << i.second;
	}
	return os;
}

// Удаляет петли
void erase_loops(graph& gr)
{
    for (auto it = gr.begin(); it != gr.end(); ++it)
    {
        unsigned count = gr.count(it, it);
        gr.disconnect(it, it, count);
    }
}
