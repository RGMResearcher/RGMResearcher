#pragma once
#ifndef _GRAPH_H_
#define _GRAPH_H_

#include <map>
#include <unordered_map>
#include <ostream>
#include <functional>
#include <memory>
#include <queue>
#include <iterator>
#include <type_traits>

template<typename ptr_type>
class hash_ptr: public std::hash<typename std::remove_pointer<ptr_type>::type>
{
	typedef std::hash<typename std::remove_pointer<ptr_type>::type> parrent;
public:
	typedef typename std::hash<typename std::remove_pointer<
	ptr_type>::type>::result_type result_type;
	typedef ptr_type argument_type;

	result_type operator()(const argument_type& ptr) const
	{
		return parrent::operator()(*ptr);
	}
};

template<class type, class edge_value_type,
		 template<class> class hash_type = hash_ptr>
class graph;

template<class type, class edge_value_type = unsigned>
class vertex
{
	friend graph<type, edge_value_type>;
private:
	// Имя вершины
	type* _name;
public:
	// Входящая степень
	edge_value_type in_d,
	// Исходящая степень
	out_d,
	// Количество петель
	loop;
	// Входящие рёбра
	std::map<unsigned, edge_value_type> input,
	// Исходящие рёбра
	output;

	vertex(type* _Name) : in_d(0), out_d(0), loop(0), _name(_Name){}
	vertex(const vertex& lnk) :input(lnk.input),
		output(lnk.output), in_d(lnk.in_d), out_d(lnk.out_d),
		loop(lnk.loop), _name(0){}
	vertex(vertex&& lnk) :input(std::move(lnk.input)),
		in_d(std::move(lnk.in_d)), out_d(std::move(lnk.out_d)),
		output(std::move(lnk.output)), loop(std::move(lnk.loop)),
		_name(std::move(lnk._name)){}

	vertex& operator=(vertex&& v)
	{
		_name = v._name;
		in_d = v.in_d;
		out_d = v.out_d;
		loop = v.loop;
		input = std::move(v.input);
		output = std::move(v.output);
		return *this;
	}
	vertex& operator=(const vertex& v)
	{
		_name = v._name;
		in_d = v.in_d;
		out_d = v.out_d;
		loop = v.loop;
		input = v.input;
		output = v.output;
		return *this;
	}
	const type& name() const
	{
		return *_name;
	}

	void clear()
	{
		input.clear();
		output.clear();
		if (_name != nullptr)
		{
			delete _name;
			_name = nullptr;
		}
		in_d = out_d = loop = 0;
	}

	~vertex()
	{
		if (_name != nullptr)
			delete _name; // не должен вызываться
	}
};

template<class type, class edge_value_type, template<class> class hash_type>
class graph_iterator : public std::iterator < std::bidirectional_iterator_tag,
		typename std::pair<unsigned const&, vertex<type, edge_value_type>& >>
{
	friend graph<type, edge_value_type, hash_type>;
public:
	typedef typename std::pair<unsigned const&,
	vertex<type, edge_value_type> const& > value_type;
	typedef typename std::pair<unsigned const&,
	vertex<type, edge_value_type> const& >* pointer;
	typedef typename std::pair<unsigned const&,
	vertex<type, edge_value_type> const& >& reference;
private:
	const graph<type, edge_value_type, hash_type>* gr;
	typename std::unordered_map<type*, unsigned, hash_type<type*>>::iterator it;
	pointer p;

	graph_iterator(const graph<type, edge_value_type, hash_type>* _gr,
				   const typename std::unordered_map < type*, unsigned,
				   hash_type<type*>> ::iterator& _it) :gr(_gr), it(_it), p(nullptr)
	{}
	graph_iterator(const graph<type, edge_value_type, hash_type>* _gr,
				   const typename std::unordered_map < type*, unsigned, hash_type <
				   type* >> ::const_iterator& _it) :gr(_gr), it(_it), p(nullptr)
	{}
public:
	graph_iterator() :gr(nullptr), p(nullptr){}
	graph_iterator(const graph_iterator& iter) :gr(iter.gr), it(iter.it),
		p((iter.p != nullptr) ? (new value_type(*iter.p)) : nullptr){}
	graph_iterator(graph_iterator&& iter) :gr(iter.gr), it(std::move(iter.it)),
		p(iter.p)
	{
		iter.p = nullptr;
		iter.gr = nullptr;
	}

	graph_iterator& operator=(const graph_iterator& iter)
	{
		gr = iter.gr;
		it = iter.it;
		p = (iter.p != nullptr) ? (new value_type(*iter.p)) : nullptr;
	}

	graph_iterator& operator=(graph_iterator&& iter)
	{
		gr = iter.gr;
		it = std::move(iter.it);
		p = (iter.p != nullptr) ? (new value_type(*iter.p)) : nullptr;
		iter.gr = nullptr;
		iter.p = nullptr;
	}

	graph_iterator& operator++()
	{
		++it;
		if (p != nullptr)
		{
			delete p;
			p = nullptr;
		}
		return *this;
	}

	graph_iterator operator++(int)
	{
		auto temp = *this;
		this->operator++();
		return temp;
	}

	graph_iterator& operator--()
	{
		--it;
		if (p != nullptr)
		{
			delete p;
			p = nullptr;
		}
		return *this;
	}

	graph_iterator operator--(int)
	{
		auto temp = *this;
		this->operator--();
		return temp;
	}

	bool operator==(const graph_iterator& it) const
	{
		return (it.it == this->it);
	}

	bool operator!=(const graph_iterator& it) const
	{
		return !(*this == it);
	}

	reference operator*()
	{
		if (p == nullptr)
			p = new value_type(it->second, gr->operator[](it->second));
		return *p;
	}

	const reference operator*() const
	{
		if (p == nullptr)
			p = new value_type(it->second, gr->operator[](it->second));
		return *p;
	}

	pointer operator->()
	{
		if (p == nullptr)
		{
			p = new value_type(it->second, gr->operator[](it->second));
		}
		return p;
	}

	const pointer operator->() const
	{
		if (p == nullptr)
		{
			p = new value_type(it->second, gr->operator[](it->second));
		}
		return p;
	}

	~graph_iterator()
	{
		if (p != nullptr)
		{
			delete p;
			p = nullptr;
		}
	}
};

template<class type, class edge_value_type,
		 template<class> class hash_type>
class graph
{
public:
	typedef graph_iterator<type, edge_value_type, hash_type> iterator;
	typedef edge_value_type edge_type;
private:
	std::queue<unsigned> deleted;
	unsigned m; // Число рёбер
	mutable std::vector<vertex<type, edge_value_type>> v;
	mutable std::unordered_map<type*, unsigned, hash_type<type*>> reindex;
	mutable iterator it_end;

	void erase(type*, unsigned);
public:

	graph() :m(0)
	{
		it_end.it = reindex.end();
	}
	graph(const graph& gr) :v(gr.v), m(gr.m), reindex(gr.reindex),
		deleted(gr.deleted)
	{
		it_end.it = reindex.end();
	}
	graph(graph&& gr) :v(std::move(gr.v)), m(std::move(gr.m)),
		reindex(std::move(gr.reindex)), deleted(std::move(gr.deleted))
	{
		gr.m = 0;
		it_end.it = reindex.end();
	}
	graph(unsigned);

	void operator=(graph&& gr)
	{
		m = gr.m;
		v = std::move(gr.v);
		reindex = std::move(gr.reindex);
		it_end.it = reindex.end();
		deleted = std::move(gr.deleted);
		gr.m = 0;
	}

	graph& operator=(const graph& gr)
	{
		m = gr.m;
		v = gr.v;
		reindex = gr.reindex;
		it_end.it = reindex.end();
		deleted = gr.deleted;
		return *this;
	}

	// добавление вершины с заданным именем
	iterator new_vertex(const type& name)
	{
		auto it = reindex.find((type*)&name);
		if (it == reindex.end())
		{
			auto* p = new type(name);
			if (this->deleted.empty())
			{
				it = reindex.insert({ p, v.size() }).first;
				v.push_back(vertex<type, edge_value_type>(p));
			}
			else
			{
				it = reindex.insert({ p, deleted.front() }).first;
				if (v[deleted.front()]._name != nullptr)
					delete v[deleted.front()]._name;
				v[deleted.front()]._name = p;
				deleted.pop();
			}
			// it_end.it = reindex.end();
		}
		return iterator(this, it);
	}

	// Поиск вешины по имени
	iterator find(const type& v_name) const
	{
		return iterator(this, reindex.find((type*)&v_name));
	}

	// Удаление вершины
	void erase(const iterator& it)
	{
		erase(*it.it);
	}

	// Удаление вершины
	void erase_by_name(const type& name)
	{
		auto it = this->find(name);
		if (it != this->end())
			erase(*it.it);
	}

	void erase_by_index(unsigned index)
	{
		erase(v[index]._name, index);
	}

	// Установка n связей first --> second
	void connect_by_index(unsigned, unsigned, edge_value_type n = 1);

	// Установка n связей first --> second
	void connect(const iterator& it_f,
				 const iterator& it_s, edge_value_type n = 1)
	{
		this->connect_by_index(it_f.it->second, it_s.it->second, n);
	}

	// Установка n связей first --> second
	void connect_by_name(const type& first,
						 const type& second, edge_value_type n= 1)
	{
		auto it_f = this->find(first);
		if (first == second)
			this->connect(it_f, it_f, n);
		else
			this->connect(it_f, this->find(second), n);
	}

	// Удаление n рёбер first --> second
	// Если рёбер меньше, чем n - они удалятся все
	// Возвращает количество удалённых рёбер
	edge_value_type disconnect_by_index(unsigned,
										unsigned, edge_value_type n = 1, bool ndel = true);

	// Удаление n рёбер first --> second
	// Если рёбер меньше, чем n - они удалятся все
	// Возвращает количество удалённых рёбер
	edge_value_type disconnect(const iterator& it_f,
							   const iterator& it_s, edge_value_type n = 1, bool ndel = true)
	{
		return this->disconnect_by_index(it_f.it->second,
										 it_s.it->second, n, ndel);
	}

	// Удаление n рёбер first --> second
	// Если рёбер меньше, чем n - они удалятся все
	// Возвращает количество удалённых рёбер
	edge_value_type disconnect_by_name(const type&,
									   const type&, edge_value_type n = 1, bool ndel = true);

	// Количество рёбер it_f --> it_s
	edge_value_type count_by_index(unsigned it_f,
								   unsigned it_s) const
	{
		if (it_f == it_s)
			return v[it_f].loop;
		auto it = v[it_f].output.find(it_s);
		return (it == v[it_f].output.end()) ? 0 : it->second;
	}

	// Количество рёбер it_f --> it_s
	edge_value_type count(const iterator& it_f,
						  const iterator& it_s) const
	{
		return this->count_by_index(it_f.it->second, it_s.it->second);
	}

	// Количество рёбер first --> second
	edge_value_type count_by_name(const type&, const type&) const;

	void merge_by_index(unsigned, unsigned);
	// Объединение двух вершин. it_f += it_s
	void merge(const iterator& it_f, const iterator& it_s)
	{
		merge_by_index(it_f->it->first, it_s->it->first);
	}

	// Объединение двух вершин. first += second
	void merge_by_name(const type& first, const type& second)
	{
		merge(this->find(first), this->find(second));
	}

	// Количество вершин в графе
	unsigned size() const
	{
		return reindex.size();
	}

	/*
	vertex<type, edge_value_type>& operator[](unsigned index)
	{
		return v[index];
	}
	*/

	const vertex<type, edge_value_type>& operator[](unsigned index) const
	{
		return v[index];
	}

	friend std::ostream& operator<<(std::ostream&, const graph&);

	void clear()
	{
		v.clear();
		deleted = std::move(std::queue<unsigned>());
		for (auto& i : reindex)
			delete i.first;
		reindex.clear();
		m = 0;
	}

	iterator begin()
	{
		return iterator(this, this->reindex.begin());
	}

	iterator end()
	{
		return this->it_end;
	}

	const iterator begin() const
	{
		return iterator(this, this->reindex.begin());
	}

	const iterator end() const
	{
		return this->it_end;
	}

	// Число рёбер
	unsigned edge_count() const
	{
		return m;
	}

	unsigned erase_loops();
};

template<class type, class edge_value_type, template<class> class hash_type>
// Установка n рёбер it_f --> it_s
void graph<type, edge_value_type, hash_type>::connect_by_index(
		unsigned it_f, unsigned it_s, edge_value_type n)
{
	if (it_f == it_s)
	{
		this->v[it_f].loop += n;
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

template<class type, class edge_value_type, template<class> class hash_type>
// Удаляет петли
unsigned graph<type, edge_value_type, hash_type>::erase_loops()
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

template<class type, class edge_value_type, template<class> class hash_type>
void graph<type, edge_value_type, hash_type>::erase(
		type* p, unsigned index)
{
	for (auto& i : v[index].input)
		this->disconnect_by_index(index, i.first, i.second);
	for (auto& i : v[index].output)
		this->disconnect_by_index(i.first, index, i.second);
	v[index].clear();
	m -= v[index].loop;
	deleted.push(index);
	reindex.erase(p);
	// it_end.it = reindex.end();
}

template<class type, class edge_value_type, template<class> class hash_type>
edge_value_type graph<type, edge_value_type, hash_type>::count_by_name(
		const type& first, const type& second) const
{
	auto it_f = this->reindex.find(&first);
	if (it_f == this->reindex.end())
		return 0;
	if (first == second)
		return this->count_by_index(it_f->second, it_f->second);
	auto it_s = this->reindex.find(&second);
	if (it_s != this->end())
		return this->count_by_index(it_f->second, it_s->second);
	return 0;
}

template<class type, class edge_value_type, template<class> class hash_type>
// Удаление n рёбер it_f --> it_s
// Если рёбер меньше, чем n - они удалятся все
// Возвращает количество удалённых рёбер
edge_value_type graph<type, edge_value_type, hash_type>::disconnect_by_index(
		unsigned it_f, unsigned it_s, edge_value_type n, bool ndel)
{
	edge_value_type ret = n;
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

template<class type, class edge_value_type, template<class> class hash_type>
// Удаление n рёбер first --> second
// Если рёбер меньше, чем n - они удалятся все
// Возвращает количество удалённых рёбер
edge_value_type graph<type, edge_value_type, hash_type>::disconnect_by_name(
		const type& first, const type& second, edge_value_type n, bool ndel)
{
	auto it_f = this->reindex.find(&first);
	if (it_f != this->reindex.end())
	{
		if (first == second)
		{
			return this->disconnect_by_index(it_f->second, it_f->second,
											 n, ndel);
		}
		else
		{
			auto it_s = this->reindex.find(&second);
			if (it_s != this->end())
			{
				return this->disconnect_by_index(it_f->second,
												 it_s->second, n, ndel);
			}
		}
	}
	return 0;
}

template<class type, class edge_value_type, template<class> class hash_type>
// Объединение двух вершин(second перехоит в first)
void graph<type, edge_value_type, hash_type>::merge_by_index(
		unsigned first, unsigned second)
{
	edge_value_type count = this->count_by_index(first, second);
	if (count > 0)
	{
		disconnect_by_index(first, second, count);
		connect_by_index(first, first, count);
	}
	count = this->count_by_index(second, first);
	if (count > 0)
	{
		disconnect_by_index(second, first, count);
		connect_by_index(first, first, count);
	}
	count = this->count_by_index(second, second);
	if (count > 0)
	{
		disconnect_by_index(second, second, count);
		connect_by_index(first, first, count);
	}
	// пересадка рёбер
	for (auto& i : v[second].input)
		connect_by_index(i.first, first, i.second);
	for (auto& i : v[second].output)
		connect_by_index(first, i.first, i.second);
	this->erase_by_index(second);
}

#endif // _GRAPH_H_
