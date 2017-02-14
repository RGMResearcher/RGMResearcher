#include "graphutility.h"

// Модель Боллобаша-Риордана
graph preferred_attachment(unsigned n, unsigned m)
{
	/* n вершин, n*m рёбер
	граф строится до достижения n*m рёбер и n*m вершин
	затем вершины объединяются друг с другом (каждые m штук в одну), пока их не станет ровно n
	*/
	n *= m;
	std::vector<unsigned> vec; // хранит номера вершин
	// при вставке ребра между first и second вершинами
	// в vec помещаются и first и second
	graph gr; unsigned i;
	std::mt19937 gen(std::time(0));

	for (i = 0; i < n; ++i)
	{
		gr.new_vertex(i);
		vec.push_back(i);
		// Кидать не 1 исходящую вершину, а несколько
		std::uniform_int_distribution<unsigned> dist(0, vec.size() - 1);
		unsigned temp = vec[dist(gen)];
		gr.connect(i, temp);
		vec.push_back(temp);
	}
	if (m < 2)
		return gr;
	// объединение вершин
	for (i = 0; i < n; i += m)
	{
		for (unsigned k = i + m - 1; k > i;)
		{
			for (unsigned h = i; h < k; ++h, --k)
			{
				gr.merge(h, k);
			}
		}
	}
	return gr;
}

void top_sort::dfs(unsigned v)
{
	used.insert(v);
	auto it1 = gr->find(v);
	for (auto& it : it1->second.output)
	{
		if (it.first != v && used.find(it.first) == used.end())
			dfs(it.first);
	}
	ans.push_back(v);
}

// не работает
std::map<unsigned, double> page_rank(const graph& gr, const double& c,
    const double& delta)
{
	std::map<unsigned, double> ret;
	for (auto& i : gr)
		ret.insert({ i.first, 1.0 });
	top_sort srt;
	const std::vector<unsigned>& v_s = srt.sort(gr);
	double dlt;
	unsigned count = 0;
	do
	{
		dlt = 0;
		for (auto& i : v_s)
		{
			double temp = 0, total = 0;
			auto it = gr.find(i);
			for (auto& v : it->second.input)
			{
				// петли не влияют на ранг
				if (v.first == i)
					continue;
				auto it1 = gr.find(v.first);
				temp += ret[v.first] / (it1->second.out_d -
					it1->second.output.count(v.first)); // ОШИБКА
				throw std::string("Неверно считается степень верины и количество петель");
			}
			total += temp*c;
			temp = 0;
			// место для суммы с никуда не указывающими вершинами
			total += (1 - c) / gr.size();

			temp = ret[i];
			if (abs(temp - total) > dlt)
				dlt = std::abs(temp - total);
			ret[i] = total;
		}
	} while (/*++count <= 2 * gr.size() && */dlt > delta);
	return ret;
}

// Генерирует кластеризованый граф
// v_count - количество вершин, min_cl_size - минимальный размер кластера,
// max_cl_size- максимальный размер кластера, q_cl - вероятность наличия ребра в кластере
std::pair<graph, unsigned> clustered_graph(unsigned v_count, unsigned min_cl_size,
    unsigned max_cl_size, double q_cl)
{
	if (min_cl_size > max_cl_size || max_cl_size > v_count ||
		min_cl_size <= 1)
	{
		throw(std::string("Wrong cl_size params"));
	}
	if (q_cl <= 0.5 || q_cl > 1)
	{
		throw(std::string("Wrong cl_size params"));
	}

	std::uniform_int_distribution<int> dist(min_cl_size, max_cl_size);
	std::uniform_int_distribution<int> in_out(0, 1);
	std::uniform_real_distribution<double> q_def(0, 1);

	std::mt19937 gen(time(0));
	std::vector<unsigned> size;
	unsigned count;
	for (count = 0;;)
	{
		unsigned temp = dist(gen);
		count += temp;
		size.push_back(temp);

		if (count >= v_count - max_cl_size && count <= v_count -
			min_cl_size)
		{
			break;
		}
		if (count > v_count - min_cl_size)
			break;
	}
	size.push_back(v_count - count);
	while (size.back() < min_cl_size)
	{
		bool err = true;
		for (unsigned i = 0; i < size.size() - 1 && size.back() <
			min_cl_size; ++i)
		{
			if (size[i] > min_cl_size)
			{
				++size.back();
				--size[i];
				err = false;
			}
		}
		if (size.back() < min_cl_size && err)
			throw(std::string("Wrong cl_size params"));
	}
	// генерим кластеры размерностью size[i] каждый.
	std::pair<graph, unsigned> ret;
	for (unsigned i = 0; i < v_count; ++i)
		ret.first.new_vertex(i);
	std::vector<std::pair<unsigned, unsigned>> clusters;
	for (unsigned b_begin = 0, cl = 0; cl < size.size(); ++cl)
	{
		unsigned b_end = b_begin + size[cl];
		clusters.push_back({ b_begin, b_end });
		for (unsigned i = b_begin; i < b_end; ++i)
			for (unsigned j = i + 1; j < b_end; ++j)
			{
				if (q_def(gen) <= q_cl)
				{
					if (in_out(gen) == 0)
						ret.first.connect(j, i, 1);
					else
						ret.first.connect(i, j, 1);
				}
			}
		auto end = ret.first.find(b_end);
		for (auto it = ret.first.find(b_begin); it != end; ++it)
		{
			if (it->second.in_d + it->second.out_d > 0)
				continue;
			std::uniform_int_distribution<int> rand_v(b_begin, b_end - 1);
			unsigned r_v;
			do{
				r_v = rand_v(gen);
			} while (r_v == it->first);

			auto it1 = ret.first.find(r_v);
			if (in_out(gen) == 0)
				ret.first.connect(it1, it, 1);
			else
				ret.first.connect(it, it1, 1);
		}
		b_begin = b_end;
	}
	ret.second = size.size();
	size.clear();
	// Зашумление
	for (unsigned i = 0; i < clusters.size(); ++i)
	{
		for (unsigned k = i + 1; k < clusters.size(); ++k)
		{
			auto end_f = ret.first.find(clusters[i].second);
			for (auto it_f = ret.first.find(clusters[i].first);
				it_f != end_f; ++it_f)
			{
				auto end_s = ret.first.find(clusters[k].second);
				for (auto it_s = ret.first.find(clusters[k].first);
					it_s != end_s; ++it_s)
				{
					if (q_def(gen) <= q_cl)
						continue;
					if (in_out(gen) == 0)
						ret.first.connect(it_s, it_f, 1);
					else
						ret.first.connect(it_f, it_s, 1);
				}
			}
		}
	}
	return ret;
}
