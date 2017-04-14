#include"Utils.h"

#ifndef SPECTRUM_MANAGER
#define SPECTRUM_MANAGER
/**
*  HINT try smart pointer insted of references
*/

class SpectrumManager {
	ListGraph *graph;
	AllocMethod *alloc_method;
	SpectrumMap *spectrum_map;
public:
	SpectrumManager() {
		graph = NULL;
		alloc_method = NULL;
		spectrum_map = NULL;
	}
	SpectrumManager(ListGraph *graph, SpectrumMap *spectrum, AllocMethod &method) :graph(graph), spectrum_map(spectrum), alloc_method(&method) {

	}
	SpectrumManager(const SpectrumManager &manager) :spectrum_map(manager.spectrum_map), alloc_method(manager.alloc_method), graph(manager.graph) {
		cout << "spctrum copy NOT TEsted";
	}
	SpectrumState getPathSpectrum(const Path<ListGraph> &path)
	{
		SpectrumState spectrum;
		Path<ListGraph>::ArcIt arc_it(path);
		for (arc_it; arc_it != INVALID; ++arc_it)
		{
			Node t = graph->target(arc_it);
			Node s = graph->source(arc_it);
			Edge e = lemon::findEdge(*graph, t, s);
			spectrum. or (spectrum_map->operator[](e));
		}
		return spectrum;
	}

	SpectrumManager& operator=(SpectrumManager &p_manager) {
		this->graph = p_manager.graph;
		this->spectrum_map = p_manager.spectrum_map;
		this->alloc_method = p_manager.alloc_method;
		return *this;
	}
	bool checkSpectrum(const int width, SpectrumState& path_spectrum) {
		if (alloc_method->makeLinkSpectrum(width, path_spectrum) != -1)
		{
			return true;
		}
		return false;
	}
	//SpectrumManager(algorithm)
	int alloc(const int width, Path<ListGraph> &path, SpectrumState &outSpect) {
		int pos = 0;
		SpectrumState pathspectrum = getPathSpectrum(path);
		SpectrumState linkSpectrum;
		if (!makeLinkSpectrum(pathspectrum, linkSpectrum, width, pos)) {
			return -1;
		}
		else {
			outSpect = linkSpectrum;
		}

		//int n1, n2;
		//n1 = graph->id(graph->source(path.front()));
		//n2 = graph->id(graph->target(path.back()));  //itt mért nem target?		
		Path<ListGraph>::ArcIt arc_it(path);
		for (arc_it; arc_it != INVALID; ++arc_it)
		{
			Node t = graph->target(arc_it);
			Node s = graph->source(arc_it);
			Edge e = lemon::findEdge(*graph, t, s);
			spectrum_map->operator[](e). or (linkSpectrum);
		}
		return pos;
	}

	bool ForceAlloc(SpectrumState spec, Path<ListGraph> &path)
	{
		Path<ListGraph>::ArcIt arc_it(path);
		for (arc_it; arc_it != INVALID; ++arc_it)
		{
			Node t = graph->target(arc_it);
			Node s = graph->source(arc_it);
			Edge e = lemon::findEdge(*graph, t, s);
			spectrum_map->operator[](e). or (spec);
		}
		return true;
	}

	static bool SetPermittingSpectrum(const int &width, SpectrumState &spectrum)
	{
		int gapwidth(0);
		for (int i = 0; i < CH::channel_num; i++)
		{
			if (!spectrum[i]) { ++gapwidth; }
			else
			{
				gapwidth = 0;
			}

			if (gapwidth == (width + 1))
			{

				return true;
			}
		}
		return false;
	}

	SpectrumMap* getMap() { return spectrum_map; }
	void dealloc();
	int getNodeLoad();
	int getNodeCapacity();
private:
	bool makeLinkSpectrum(SpectrumState path_spectrum, SpectrumState &retLinkSpectrum,const int width,int& pos) {
		pos = 0;
		if ((pos=alloc_method->makeLinkSpectrum(width, path_spectrum)) != -1) {
			for (int i = 0; i<width; i++) { retLinkSpectrum[pos + i] = 1; }
		}
		else {
			return false;
		}
		return true;
	}
};






class AllocMethods{
public:
	static int TwoSideSpectrumCheck(int width, SpectrumState &spectrum)
	{
		int gapwidth1(0), gapwidth2(0), alloc_pos(0);
		for (int i = 0, j = CH::channel_num - 1; i<CH::channel_num&&j>0; i++, j--)
		{
			if (!spectrum[i]) { ++gapwidth1; }
			else
			{
				gapwidth1 = 0;
			}

			if (gapwidth1 == (width + 2))
			{
				alloc_pos = (i - width);
				return alloc_pos;
			}
			if (!spectrum[j]) { ++gapwidth2; }
			else
			{
				gapwidth2 = 0;
			}

			if (gapwidth2 == (width + 2))
			{
				alloc_pos = (j + 1);// itt kell máshogy beálltíani
				return alloc_pos;
			}

		}
		return -1;
	}

	// hézag kitöltési spektrum allokálás
	static int GapFillSpectrumCheck(int width, SpectrumState &spectrum)
	{
		int gapwidth(0), alloc_pos(0);
		std::multiset<GAP, GAPcmp> gaps;
		for (int i = 0; i<CH::channel_num; i++)
		{
			if (!spectrum[i]) { ++gapwidth; }
			else
			{
				if (gapwidth>width + 2)
				{
					gaps.insert(GAP(i - gapwidth + 1, gapwidth));
				}
				gapwidth = 0;
			}
		}


		if (gapwidth>width + 2)
		{
			gaps.insert(GAP(CH::channel_num - gapwidth, gapwidth));
		}

		std::multiset<GAP, GAPcmp>::iterator it = gaps.begin();
		if (!gaps.empty()){
			int tpos = (int)it->pos;
			int tpos2 = (int)it->width;
			alloc_pos = (int)it->pos + ((int)it->width - width) / 2; //a legnagyobb hézag közepe

			return alloc_pos;
		}
		return -1;
	}

	// egy irányú spektrum allokálás
	static int BaseSpectrumCheck(int width, SpectrumState &spectrum)
	{
		int gapwidth(0), alloc_pos(0);
		for (int i = 0; i<CH::channel_num; i++)
		{
			if (!spectrum[i]) { ++gapwidth; }
			else
			{
				gapwidth = 0;
			}

			if (gapwidth == (width + 2))
			{
				alloc_pos = (i - width);
				return alloc_pos;
			}
		}
		return -1;
	}

	static int OtherBaseSpectrumCheck(int width, SpectrumState &spectrum)
	{
		int gapwidth(0), alloc_pos(0);
		for (int i = CH::channel_num - 1; i>0; i--)
		{
			if (!spectrum[i]) { ++gapwidth; }
			else
			{
				gapwidth = 0;
			}

			if (gapwidth == (width + 2))
			{
				alloc_pos = (i + 1);
				return alloc_pos;
			}
		}
		return -1;
	}
};
//bejárás, spekrum visszadás, módosítás
//él spektruma
/*
class MySpectrumMap{
	const ListGraph &graph;
	std::map<int, SpectrumState> map; // arc id, arc spectrum
};
class SpectrumManager2{
	const ListGraph &graph;
	MySpectrumMap spectrum_map;
public:
	SpectrumManager2(const ListGraph &graph, SpectrumMap spectrum) :graph(graph), spectrum_map(spectrum){}
	SpectrumManager2(const ListGraph &graph) :graph(graph), spectrum_map(graph){} //TO-DO TEST
	//SpectrumManager(algorithm)
	void alloc(){

	};
	void dealloc();
	int getNodeLoad();
	int getNodeCapacity();
	};*/

#endif SPECTRUM_MANAGER