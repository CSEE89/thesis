#include <iostream>
#include<lemon/list_graph.h>
#include<lemon/core.h>
#include<lemon/adaptors.h>
#include<algorithm>
#include<iterator>
#include<time.h>
#include<bitset>
#include<lemon/path.h>
#include <cstdlib>
#include<lemon/graph_to_eps.h>
#include<lemon/list_graph.h>
#include<lemon/core.h>
#include<iostream>
#include<set>
#include<lemon/lgf_reader.h>
#include<lemon/adaptors.h>
#include<lemon/connectivity.h>
#include<lemon/path.h>

/*

* Author: csabi
*A spektrumszeltek definiálása. Spektrumkitöltési stratégiák implementálása.
* Üzemi és védelmi utak tárolása, karbantartása, felszabadítása.
*/

#ifndef TRAITS_H
#define	TRAITS_H

using namespace lemon;
using namespace std;





Path<lemon::ListGraph> p;
typedef ListGraph::Node Node;
typedef ListGraph::Edge Edge;
//typedef typename ListGraph::EdgeMap<typename> GraphMap;
class SpectrumState;
typedef ListGraph::EdgeMap<SpectrumState> SpectrumMap;
typedef Path<ListGraph> listpath; //tipusdefiniciók a ModDijkstra tárolási egységeinek
typedef std::pair<listpath, SpectrumState> pathpair;
typedef std::vector<pathpair> pathpair_vector;
enum ALLOC {OneSideChannelFill,GapFill,TwoSideChannelFill};  //soektrum feltöltési stratégiák

#if 0		
class SpectrumMap : public ListGraph::EdgeMap<SpectrumState> 
{
	SpectrumMap(const SpectrumMap* specMap)
	{
		
		for (ListGraph::EdgeIt it(g); it != INVALID; ++it) {
			tmp->set(it, true);
		}
	}
};

#endif

/**
Csatornák száma, beaállítva a main.cpp ben
*/


#define BITSETCNT 64

class SpectrumState{
protected:
	std::bitset<BITSETCNT> carrier;

public:
	SpectrumState()
	{
		carrier = std::bitset<BITSETCNT>(0);
	}

	SpectrumState(const SpectrumState &other)	
	{
		carrier = other.carrier;
	}

	/// Két szál spektrumának összevagyolása, this-> carrier kapja az összevagyolt értékeket
	inline void or(SpectrumState &s)
	{
		carrier |= s.carrier;
	}
	inline void dealloc(SpectrumState &s)
	{
		//s.carrier != s.carrier;
		//carrier &= s.carrier;
		carrier ^= s.carrier;
		//for (int i = 0; i < BITSETCNT; i++)
		//{
		//	if (s.carrier[i])
		//		carrier[i] = 0;
		//}
	}

	bool Equal(SpectrumState &s)
	{
		return carrier == s.carrier;
	}
	bool operator==(SpectrumState &s)
	{
		return carrier == s.carrier;
	}
	/// spektrum kiirasa
	void print()
	{
		for (int i = 0; i<BITSETCNT; i++)
		{
			std::cout << carrier[i] << " ";
		}
	}

	/// Egy-egy spektrumszelet elérése, referenciával tér vissza ,hogy módosítható legyen az elem
	std::bitset<BITSETCNT>::reference operator[](int i){ return carrier[i]; }	
	std::bitset<BITSETCNT>::reference at(int i){ return carrier[i]; }

};
/*class SpectrumStateEX 
{
	const SpectrumState *m_spectrumS;
public:
	
	SpectrumStateEX(const SpectrumState* sp)
	{
		m_spectrumS = sp;
	}

	inline void BeginEnd(int &begin, int &end)
	{
		bool first = false;
		for (int i = 0; i < BITSETCNT; i++)
		{

			if (m_spectrumS->carrier[i] && !first)
			{
				begin = i;
				first = true;
			}
			else if (!carrier[i] && first)
			{
				end = i;
				break;
			}
		}
	}

	inline void Zero()
	{
		std::bitset<BITSETCNT> n;
		carrier = n;
	}
};*/

class SpectrumStateEX : public SpectrumState
{
public:
	SpectrumStateEX():SpectrumState() {}
	SpectrumStateEX(const SpectrumState& sp) : SpectrumState(sp) {}

	inline void BeginEnd(int &begin, int &end)
	{
		bool first = false;
		for (int i = 0; i < BITSETCNT; i++)
		{
			
			if (carrier[i] && !first) 
			{
				begin = i;
				first = true;
			}
			else if(!carrier[i] && first)
			{
				end = i;
				break;
			}			
		}
	}
	int valami(int a, int b)
	{
		return 1;
	}
	inline bool DeallocAndInvert(int width, SpectrumState * toDealloc)
	{
		bool begin = false;
		
		for (int i = 0; i < BITSETCNT; i++)
		{
			if (carrier[i])
			{
				begin = true;
				carrier[i] = 0;
				toDealloc->operator[](i) = 1;
				width--;
			}
			else if(begin) {
				_ASSERT(0);
			}
			if (width == 0)
			{
				if ((i < BITSETCNT) && carrier[i + 1])
				{
					return false;
				}
				else if((i < BITSETCNT) && !carrier[i + 1]) 
				{
					return true;
				}
				return false;
			}
		}
	}

	//van a 2 kozt hezag
	bool TestIfNeighbour(SpectrumState &st)
	{
		bool started = false;
		bool ended = false;
		for (int i = 0; i < BITSETCNT; i++)
		{
			if (carrier[i] || st[i])
			{
				started = 1;
				if (ended)
					return false;
			}
			if (started && (!carrier[i] && !st[i]))
				ended = true;
		}
		return true;
	}

	int countOnes() 
	{
		int ones = 0;
		for (int i = 0; i < BITSETCNT; i++)
		{
			if (carrier[i])
				ones++;
		}
		return ones;
	}

	inline void Zero() 
	{
		std::bitset<BITSETCNT> n;
		carrier = n;
	}
};

// komparátor osztály: útvonalakat(pathpair) hasonlít össze hossz szerint,\
modDijkstra használja
class comp{
public:
	bool operator()(const pathpair &a, const pathpair &b)
	{
		return a.first.length() < b.first.length();
	}
};
template <typename T>class MapFactory{
public:
	typedef T GR;
	typedef typename GR::template EdgeMap<bool> pMap; //permitting map
	typedef typename GR::template NodeMap<pathpair_vector> path_Map;  //path map
	typedef typename GR::template EdgeMap<SpectrumState> edge_spectrum_Map;
	typedef typename GR::template EdgeMap<double> cost_Map;
	typedef typename GR::template EdgeMap<int> int_Map;
	pMap *createPermittingmap(const GR &g)
	{
		GR::EdgeMap<bool>* tmp = new GR::EdgeMap<bool>(g);
		for (GR::EdgeIt it(g); it != INVALID; ++it){
			tmp->set(it, true);
		}
		return tmp;
	}
	path_Map* createPathMap(const GR &g)
	{
		path_Map *ret = new GR::NodeMap<pathpair_vector>(g);
		
		for(GR::NodeIt it(g); it!= INVALID; ++it)
		{
			(*ret)[it].reserve(1000);
		}
		return ret;
	}
	edge_spectrum_Map* createEdgeSpectrumMap(const GR &g)
	{

		return new GR::EdgeMap<SpectrumState>(g);
	}
	cost_Map* createEdgeCostMap(const GR &g)
	{

		return new GR::EdgeMap<double>(g);
	}
	int_Map* createIntMap(const GR &g)
	{
		return new GR::EdgeMap<int>(g);
	}

};

//\
Print függvények 
//Útvonal kiírása
void printPath(listpath &path, const ListGraph &graph)
{
	std::cout << "[";
	PathNodeIt<Path<ListGraph> > path_it(graph, path);
	for (path_it; path_it != INVALID; ++path_it)
	{
		std::cout << ", " << graph.id(path_it);
	}

	std::cout << "]";
	std::cout << std::endl;
}

void printSet(pathpair lp, const ListGraph &graph)
{
	lemon::Path<ListGraph> path;
	path = lp.first;
	std::cout << "[";
	PathNodeIt<Path<ListGraph> > path_it(graph, path);
	for (path_it; path_it != INVALID; ++path_it)
	{
		std::cout << ", " << graph.id(path_it);
	}

	std::cout << "]";
	std::cout << std::endl;
}

void printNode(const pathpair_vector &vec, const ListGraph &graph)
{
	for (pathpair_vector::const_iterator cvi = vec.begin(); cvi != vec.end(); cvi++)
	{
		lemon::Path<ListGraph> path;
		path = cvi->first;
		std::cout << "[";
		PathNodeIt<Path<ListGraph> > path_it(graph, path);
		for (path_it; path_it != INVALID; ++path_it)
		{
			std::cout << ", " << graph.id(path_it);
		}

		std::cout << "]" << std::endl;
	}


}

void printSpectrum(ListGraph::EdgeMap<SpectrumState> *spectrum_map, const ListGraph &g)
{
	ListGraph::EdgeIt eit(g);
	//cout << endl;
	for (eit; eit != INVALID; ++eit)
	{
		Node u = g.u(eit);
		Node v = g.v(eit);
		cout << g.id(u)<< " "<<g.id(v)<< " ";
		spectrum_map->operator[](eit).print();
		std::cout << std::endl;
	}

}

double spectrumUtilization(ListGraph::EdgeMap<SpectrumState> *spectrum_map, const ListGraph &g)
{
	ListGraph::EdgeIt eit(g);
	//cout << endl;
	int edgeNum = lemon::countEdges(g);
	int ones = 0;
	for (eit; eit != INVALID; ++eit)
	{
		SpectrumStateEX sex( spectrum_map->operator[](eit) );
		ones += sex.countOnes();
		//std::cout << std::endl;
	}

	return double((double)ones / (double)(64 * edgeNum));
}

class PathNodes{
public:
	std::vector<Node> p_nodes;
	
public:
	PathNodes(Path<ListGraph> path, const ListGraph &graph){
		for (PathNodeIt<Path<ListGraph> > pnit(graph, path); pnit != INVALID; ++pnit){
			p_nodes.push_back(pnit);			
		}
	}
	Node at(int i){
		return p_nodes[i];
	}
	Node endNode(){
		return p_nodes.back();
	}
	Node beginNode(){
		return p_nodes.front();
	}
private:
	PathNodes(){}
};

class Stopper{
	long time;
	long elapsed=0;
public:
	Stopper(){}
	void start(){ time = clock(); }
	void stop(){ elapsed += clock() - time; }
	long getTime(){ return clock() - time; }
	long getElapsed(){ return elapsed; }
	void reset(){ time = 0; elapsed = 0; }
};


#endif TRAITS_H