#include"SpectrumManager.h"
#include"moddijkstra.h"
#include"Kshort_mod.h"
#include "TrafficMatrix.h"

#define REQUESTES_HINT 10000

#ifndef RSA
#define RSA
/**
*  Props: Grooming, Protection, TrafficManager, SpectrumManager --> Composition,inheritance, polimorhism(reference,pointer), template
*/
template<typename GR>
class Algorithm {
protected:
	typedef Path<GR> Path;
	//	Grooming &grooming;
	SpectrumManager		*m_pSpectrum_manager = nullptr;
	TrafficManager		*m_pTraffic_manager	 = nullptr;
	//RequestMatrix	m_trafficMatrix;

	Path			m_allocated;
	GR&				m_graph;

public:
	Algorithm(GR &graph) :m_graph(graph)
	{
		m_pTraffic_manager = new TrafficManager(REQUESTES_HINT, lemon::countNodes(graph));
		//cout << "Algorithm default";
	}
	~Algorithm()
	{
		if (m_pTraffic_manager != nullptr)
		{
			delete m_pTraffic_manager;
			m_pTraffic_manager = nullptr;
		}
	}
	void setSpectrumManager(SpectrumManager *p){
		m_pSpectrum_manager = p;
		m_pTraffic_manager->SetSpectrumManager(p);
	}
	virtual void run(Node s, Node t, const int width, const long int duration) 
	{
		int nS = m_graph.id(s);
		int nT = m_graph.id(t);
		//Link groomLink;
//todo		bool bGromming = m_pTraffic_manager->Gromming(nS, nT, width, &groomLink);
//		if (bGromming)
		{
		//	m_pSpectrum_manager->forceAlloc(groomLink.m_path,groomLink.m_spectrum);
		//	m_pTraffic_manager->AddRequest(nS, nT, width, duration, linkSpectrum);
		}
		if (1) {}
		else 		{
			Path path = calcPath(s, t, width);
			SpectrumState linkSpectrum;
			if (!path.empty())
			{
				int pos = m_pSpectrum_manager->alloc(width, path, linkSpectrum); //spectrum, graf, traffic_manager?
				if (pos == -1)
					_ASSERT(0);
				//blokk
				m_pTraffic_manager->AddRequest(nS, nT, width, duration, linkSpectrum);
				m_allocated = path;
				//traffic_manager.addLink(link); //
			}
			else
			{
				//blokkolas
			}
		}
	};
	SpectrumManager* getManager() { return m_pSpectrum_manager; }
	virtual Path calcPath(Node s, Node t, const int width) abstract;
	virtual Path getAllocatedPath(){
		return m_allocated;
	}
	virtual Path CreatePath(int width) abstract;
	TrafficMatrix* GetMatrix() 
	{
		return m_trafficMatrix;
	}

};

/** TO DOS:
		TO DO 1 ALG SPEKTRUMOT NEM TAROL CSAK UTAKAT ADOTT CSP_HEZ
		TO DO 2 KÜL FUTTATÁSOK KÖZTI UTAKAT MEGTART
		TO DO 3 SIMA DIJKSTRAVAL LEGRÖVIDEBB UT, AMI HOSSZABB X_SZER MINT LEGRÖVIDEBB ELVET
*
ModDijkstra osztály, az algoritmus a LEMON módosított Dijktra osztályra épül
*/	
// NEW-t tartalmazó osztályokba másoló konstrzktor és op= KELL
 template<typename GR> class ModDijkstra : public Algorithm<GR>
{
	typedef typename GR::template EdgeMap<bool> pMap; //permitting map
	typedef typename GR::template NodeMap<pathpair_vector> path_Map;  //path map
	typedef typename GR::template EdgeMap<double> cost_Map;

	pMap* permittingmap; //
	path_Map *pathmap; //tárolja a nodeokban a dijkstra futás során tárolt értékeketet, útvonalak, és ahhoz tartozó spektrum
	//GR &graph;
	//ListGraph::EdgeMap<SpectrumState> &spectrum_map;  //globális spektrum
	std::multiset<pathpair, comp> _set;  //a két csomópont között megtalált útvonalak hossz szerint, (2 szer fut a dijkstra a 2 irányra ezeket pakolja bele)
	cost_Map* lengthmap;

public:
	ModDijkstra(GR &_graph ):Algorithm<GR>(_graph){
		MapFactory<GR> mapf;
		permittingmap = mapf.createPermittingmap(m_graph);
		pathmap = mapf.createPathMap(m_graph);
		lengthmap = mapf.createEdgeCostMap(m_graph);
		for (GR::EdgeIt it(m_graph); it != INVALID; ++it){
			lengthmap->set(it, 1);
		}
	}
	ModDijkstra(const ModDijkstra &mod){
		cout << "TO DO COPY CONST MODDIJK";
	}

	~ModDijkstra()
	{
		delete permittingmap;
		delete pathmap;
		delete lengthmap;
	}
	//bool run(Node s, Node t, const int width, cont long int duration){	
	//}
	
	

	Path calcPath(Node s, Node t, const int width)
	{
		m_allocated.clear();
		bool switcher = false; // tudunk e allokálni valamelyik út mentén
		int pos(0);
		SpectrumMap* map = m_pSpectrum_manager->getMap();
		lemon::csabi::Dijkstra<GR, cost_Map> dijkstra(m_graph, *lengthmap, *map);
		setperm(width);
		dijkstra.init();
		dijkstra.modaddSource(s, *pathmap);
		dijkstra.modstart(*permittingmap, *pathmap);

		pathpair_vector temp(pathmap->operator[](t));

		this->init();
		setperm(width);
		dijkstra.init();
		dijkstra.modaddSource(t, *pathmap);
		dijkstra.modstart(*permittingmap, *pathmap);
		setfill(pathmap->operator[](s), temp);

		//this->init()
		return CreatePath(width);
	}

	void printAllocated(){
		for (Path::ArcIt it(allocated); it != INVALID; ++it) cout << m_graph.id(it) / 2 << " ";
		cout << endl;
	}
	void printAllocatedNodes(){
		for (PathNodeIt<Path > it(m_graph, m_allocated); it != INVALID; ++it) cout << m_graph.id(it) << " ";
		cout << endl;
	}


private:

	void setlengthMap(ListGraph::EdgeMap<double>& map){
		for (GR::EdgeIt it(m_graph); it != INVALID; ++it)
		{
			lengthmap->set(it, map[it]);
		}
	}

	void init(){
		GR::NodeIt it(m_graph);
		for (it; it != INVALID; ++it)
		{
			pathmap->operator[](it).clear();
		}
		delete pathmap;
		MapFactory<GR> mapf;
		this->pathmap = mapf.createPathMap(m_graph);
		GR::EdgeIt eit(m_graph);
		_set.clear();

	}
	/**
	Két útvonalhalmazt ad össze és rendez hossz szerint, majd bemásolja a _set halmaz tárolóba
	*/
	void setfill(pathpair_vector &t1, pathpair_vector &t2)
	{
		int t1size = t1.size();
		pathpair *ptr = t1.data();
		for (int i = 0; i<t1size; i++) {
			_set.insert(ptr[i]);
		}
		int t2size = t2.size();
		ptr = t2.data();
		for (int i = 0; i<t2size; i++) {
			_set.insert(ptr[i]);
		}
#if 0
		for (int i = 0; i<t1.size(); i++){
			_set.insert(t1[i]);
		}
		for (int i = 0; i<t2.size(); i++){
			_set.insert(t2[i]);
		}
#endif
	}
	/**
	* beállítja a tiltó map-et, lituljtuk azokat az éleket amin elve nincs elég szbad spektrum
	*/
	void setperm(const int &width){
		for (GR::EdgeIt it(m_graph); it != INVALID; ++it) //végigjárjuk az összes élet és megnézzük van e elég hely
		{
			SpectrumMap* spectrum_map = m_pSpectrum_manager->getMap();
			permittingmap->operator[](it) = SpectrumManager::SetPermittingSpectrum(width, spectrum_map->operator[](it));
		}
	}

	/**Végigmegyünk az utvonalhalmazon amit megtalált az algoritmus, kiválasztjuk azt az útvonalat amin az spektrumallokálási módszer talál szabad sávot
	* függvényparaméterként átvett tmpPath-ba teszi az utat
	*/
	Path CreatePath(int width) override
	{
		for (std::multiset<pathpair, comp>::iterator it = _set.begin(); it != _set.end(); it++)
		{

			SpectrumState spectrum(it->second); //KOPI KONSTRUKTOR

			if (m_pSpectrum_manager->checkSpectrum(width, spectrum))
			{
				return (Path)it->first;
			}

		}
		Path b;
		return b;
	}

};



/**Class Kshort
*/
template<typename GR>class Kshort : public KShortestPath<GR>, public Algorithm<GR>{	
	int K;
public:

	Kshort(GR &graph) :KShortestPath(graph)
	{

	}
	void setK(int &k){
		K = k;
	}
	Path calcPath(Node s, Node t, const int width){
		A.clear();
		YenKshort(s, t, K);	
		return createPath(width);
	}

private:


	Path CreatePath(int width) override
	{
		for (std::vector<std::vector<int> >::const_iterator it = A.begin(); it != A.end(); it++)
		{
			Path path = vector2Path(it);

			SpectrumState pathSpectrum = m_pSpectrum_manager->getPathSpectrum(path); //útvonal sepktruma

			//if (GlobalSpectrumState::getInstance().checkSelector(width, spectrum) && !path.empty())  //alloc_pos beállítás ha van elég spektrum
			if (m_pSpectrum_manager->checkSpectrum(width,pathSpectrum) && !path.empty())  //alloc_pos beállítás ha van elég spektrum
			{
				return path;
			}

		}
		return Path path;
	}

private:
	Path vector2Path(std::vector<int> &vec){
		Path<ListGraph> tmpPath;
		for (int i = 1; i<vec.size(); i++)
		{
			int j = i - 1;
			Node s = graph.nodeFromId(it->at(i));
			Node t = graph.nodeFromId(it->at(j));
			ListGraph::Arc arc = lemon::findArc(graph, s, t);
			tmpPath.addBack(arc);
		}
		return tmpPath;
	}

};
#endif RSA