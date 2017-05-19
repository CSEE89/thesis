#include"SpectrumManager.h"
#include"moddijkstra.h"
#include"Kshort_mod.h"
#include "TrafficMatrix.h"

//#define REQUESTES_HINT 10000

#ifndef RSA
#define RSA
/**
*  Props: Grooming, Protection, TrafficManager, SpectrumManager --> Composition,inheritance, polimorhism(reference,pointer), template
*/
template<typename GR>
class Algorithm {

template<class GR> friend class SimulationMethod;
protected:
	typedef Path<GR> Path;
	//	Grooming &grooming;
	SpectrumManager		*m_pSpectrum_manager = nullptr;
	TrafficManager		*m_pTraffic_manager	 = nullptr;
	long long		m_nBlockNum = 0;
	Path			m_allocated;
	GR&				m_graph;

public:
	Algorithm(GR &graph,int reqnum) :m_graph(graph)
	{
		m_pTraffic_manager = new TrafficManager(reqnum, lemon::countNodes(graph));
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
		SpectrumState mGroomSpectrum1, mGroomSpectrum2, end2EndSepctrum;
		MatrixLink* origMGroomingLink1 = nullptr; 
		MatrixLink* origMGroomingLink2 = nullptr;
		MatrixLink* origEnd2Link		 = nullptr;

		Link algorithmLink;
		int groomingLength	= 0;
		int algorithmLength = 0;
		bool end2endGrooming = false;
		bool bGromming = false;

		end2endGrooming = m_pTraffic_manager->End2endGrooming(nS, nT, width, &origEnd2Link,&end2EndSepctrum);
		if(end2endGrooming)
		{
			m_pSpectrum_manager->ForceAlloc(origEnd2Link->m_path, end2EndSepctrum);
		
			SpectrumStateEX se(origEnd2Link->m_spectrum);
			if (se.TestIfNeighbour(end2EndSepctrum) == false)
				_ASSERT(0);
		
			origEnd2Link->m_spectrum. or (end2EndSepctrum);
		
			m_pTraffic_manager->ExtendRequest(nS, nT, width, duration, origEnd2Link->linkID);
			return;
		}
		bGromming = m_pTraffic_manager->MatrixGroomingShortest(nS, nT, width, &origMGroomingLink1, &origMGroomingLink2,
			&mGroomSpectrum1, &mGroomSpectrum2);
		if (bGromming)
		{
			groomingLength = origMGroomingLink1->m_path.length() + origMGroomingLink2->m_path.length();
		}
		
		algorithmLink.m_path = calcPath(s, t, width);
		algorithmLength = algorithmLink.m_path.length();

		//decide wich methdos paths to alloc*******************
		if ((algorithmLength > 0) || (groomingLength > 0))
		{
			if ( ((groomingLength <= algorithmLength) && bGromming && (algorithmLength > 0)) || (algorithmLength == 0) )
			{
				m_pSpectrum_manager->ForceAlloc(origMGroomingLink1->m_path, mGroomSpectrum1);
				m_pSpectrum_manager->ForceAlloc(origMGroomingLink2->m_path, mGroomSpectrum2);
				
				//debug
				SpectrumStateEX se(origMGroomingLink1->m_spectrum);
				if (se.TestIfNeighbour(mGroomSpectrum1) == false)
					_ASSERT(0);
				SpectrumStateEX se1(origMGroomingLink2->m_spectrum);
				if (se1.TestIfNeighbour(mGroomSpectrum2) == false)
					_ASSERT(0);

				origMGroomingLink1->m_spectrum. or (mGroomSpectrum1); //traffic matrix meglovo elemtet szelesitjuk
				origMGroomingLink2->m_spectrum. or (mGroomSpectrum2);
								
				m_pTraffic_manager->ExtendRequest(origMGroomingLink1->s, origMGroomingLink1->t, width, duration, origMGroomingLink1->linkID);
				m_pTraffic_manager->ExtendRequest(origMGroomingLink2->s, origMGroomingLink2->t, width, duration, origMGroomingLink2->linkID);
			}
			else if(algorithmLength > 0)
			{				
				if ( m_pSpectrum_manager->Alloc(width, algorithmLink.m_path, algorithmLink.m_spectrum) )
				{
					m_pTraffic_manager->AddRequest(nS, nT, width, duration, algorithmLink); // TODO check if matrix stores correctly
				}
				else
				{
					m_nBlockNum += (width * duration);
				}
			}
		}
		else
		{
			m_nBlockNum += (width * duration);
		}
		m_pTraffic_manager->IncTime();
	}

	SpectrumManager* getManager() { return m_pSpectrum_manager; }
	virtual Path calcPath(Node s, Node t, const int width) abstract;
	virtual Path getAllocatedPath(){
		return m_allocated;
	}
	//virtual Path CreatePath(int width) abstract;
	TrafficManager* GetTrafficManager()
	{
		return m_pTraffic_manager;
	}
	long long GetBlockNum() { return m_nBlockNum; }
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
	ModDijkstra(GR &_graph,int reqhint ):Algorithm<GR>(_graph,reqhint){
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
		this->init();
		m_allocated.clear();
		bool switcher = false; // tudunk e allokálni valamelyik út mentén
		int pos(0);
		SpectrumMap* map = m_pSpectrum_manager->GetMap();
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
		return CreatePath(s,t,width);
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

	void init() 
	{
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
			SpectrumMap* spectrum_map = m_pSpectrum_manager->GetMap();
			permittingmap->operator[](it) = SpectrumManager::SetPermittingSpectrum(width, spectrum_map->operator[](it));
		}
	}

	/**Végigmegyünk az utvonalhalmazon amit megtalált az algoritmus, kiválasztjuk azt az útvonalat amin az spektrumallokálási módszer talál szabad sávot
	* függvényparaméterként átvett tmpPath-ba teszi az utat
	*/
	Path CreatePath(Node s, Node t, int width) //override
	{
		Path ret;
		for (std::multiset<pathpair, comp>::iterator it = _set.begin(); it != _set.end(); it++)
		{
			SpectrumState spectrum(it->second); //KOPI KONSTRUKTOR

			if (m_pSpectrum_manager->checkSpectrum(width, spectrum))
			{
				Path p = it->first;
				PathNodes pn(p, m_graph);
				if (((pn.beginNode() == s) || (pn.beginNode() == t)) && ((pn.endNode() == s) || (pn.endNode() == t)))
				{
					ret = (Path)it->first;
					break;
				}
				else { _ASSERT(0); }
			}
		}
		return ret;
	}

};



/**Class Kshort
*/
template<typename GR>class Kshort : public KShortestPath<GR>, public Algorithm<GR>{	
	int K = 1;
public:

	Kshort(GR &graph,int reqhint) :KShortestPath(graph), Algorithm<GR>(graph,reqhint)
	{

	}
	void setK(int k =1){
		K = k;
	}
	Path calcPath(Node s, Node t, const int width){
		A.clear();
		YenKshort(s, t, K);	
		return CreatePath(width);
	}

private:

	Path CreatePath(int width) //override
	{
		Path ret;
		//for (std::vector<std::vector<int> >::const_iterator it = A.begin(); it != A.end(); it++)
		for (int i = 0; i < A.size(); i++)
		{
			ret = vector2Path(A[i]);

			SpectrumState pathSpectrum = m_pSpectrum_manager->getPathSpectrum(ret); //útvonal sepktruma

			//if (GlobalSpectrumState::getInstance().checkSelector(width, spectrum) && !path.empty())  //alloc_pos beállítás ha van elég spektrum
			if (m_pSpectrum_manager->checkSpectrum(width,pathSpectrum) && !ret.empty())  //alloc_pos beállítás ha van elég spektrum
			{
				return ret;
			}

		}
		return ret;
	}

private:
	Path vector2Path(std::vector<int> &vec){
		Path tmpPath;
		for (int i = 1; i<vec.size(); i++)
		{
			int j = i - 1;
			Node s = graph.nodeFromId(vec[i]);
			Node t = graph.nodeFromId(vec[j]);
			ListGraph::Arc arc = lemon::findArc(graph, s, t);
			tmpPath.addBack(arc);
		}
		return tmpPath;
	}

};

template <typename GR> class SimulationMethod {

protected:
	Algorithm<GR>		*m_algorithm;
	GR					*m_graph;
	SpectrumManager		*m_pSpectrumManager = nullptr;
	TrafficManager		*m_pTraffic_manager = nullptr;
	long long			*m_nBlockNum = 0;
	__int64				m_sumSp = 0;
	int					m_mGroomcnt = 0;
	int					m_eGroomcnt = 0;
		
public:
	SimulationMethod(Algorithm<GR> * algorithm)
	{
		m_algorithm			= algorithm;
		m_graph				= &(m_algorithm->m_graph);
		m_pSpectrumManager	= m_algorithm->m_pSpectrum_manager;
		m_pTraffic_manager	= m_algorithm->m_pTraffic_manager;
		m_nBlockNum			= &(m_algorithm->m_nBlockNum);
	}

	__int64 GetSumSP() { return m_sumSp; }

	int GetActiveNum() { return m_pTraffic_manager->GetActive(); }

	int GetmGroomCnt() { return m_mGroomcnt; }

	int GeteGroomCnt() { return m_eGroomcnt; }

	virtual void run(Node s, Node t, const int width, const long int duration)
	{
		int nS = m_graph->id(s);
		int nT = m_graph->id(t);

		Link algorithmLink;
		int algorithmLength = 0;
		
		algorithmLink.m_path = m_algorithm->calcPath(s, t, width);
		algorithmLength = algorithmLink.m_path.length();

		if (algorithmLength > 0)
		{
			m_sumSp += ShortestPathLength(s, t);
			//if (len> algorithmLength)
			//	_ASSERT(0);
			if (m_pSpectrumManager->Alloc(width, algorithmLink.m_path, algorithmLink.m_spectrum))
			{
				m_pTraffic_manager->AddRequest(nS, nT, width, duration, algorithmLink); // TODO check if matrix stores correctly
			}
			else
			{
				*m_nBlockNum += 1;///(width * duration);
			}
		}
		else
		{
			*m_nBlockNum += 1;//(width * duration);
		}		
		m_pTraffic_manager->IncTime();
	}

	int ShortestPathLength(Node s, Node t)
	{
		//calc shortest path
		GR::EdgeMap<int> lengthmap1(*m_graph);

		for (GR::EdgeIt it(*m_graph); it != INVALID; ++it)
		{
			lengthmap1.set(it, 1);
		}

		Path<GR> sp;

		//Dijkstra<GR, GR::EdgeMap<int>> dijkstra(*m_graph, lengthmap1);
		//dijkstra.init();
		//dijkstra.addSource(s);
		//dijkstra.start();
		//sp = dijkstra.path(t);
		//PathNodes pn(sp, *m_graph);
		//KShortestPath<GR>::Length_Map lengthmap(*m_graph);
		KShortestPath<GR>::Distance_Map dist(*m_graph);
		dijkstra(*m_graph, lengthmap1).distMap(dist).path(sp).run(s, t);
		int spLength = sp.length();
		return sp.length();
	}
};



template<typename GR> class MatrixGrooming : public SimulationMethod<GR> {
	
public:
	MatrixGrooming(Algorithm<GR> * algorithm) :SimulationMethod<GR>(algorithm)
	{
		m_pTraffic_manager->SetSpectrumGrooming(eMatrixGroomingMethod::eSplittedSpectrum);
	}

	virtual void run(Node s, Node t, const int width, const long int duration) override
	{
		int nS = m_graph->id(s);
		int nT = m_graph->id(t);
		SpectrumState mGroomSpectrum1, mGroomSpectrum2, end2EndSepctrum;
		MatrixLink* origMGroomingLink1 = nullptr;
		MatrixLink* origMGroomingLink2 = nullptr;
		MatrixLink* origEnd2Link = nullptr;

		Link algorithmLink;
		int groomingLength = 0;
		int algorithmLength = 0;
		bool end2endGrooming = false;
		bool bGromming = false;

		//end2end grooming******
		end2endGrooming = m_pTraffic_manager->End2endGrooming(nS, nT, width, &origEnd2Link, &end2EndSepctrum);
		if (end2endGrooming)
		{
			m_pSpectrumManager->ForceAlloc(origEnd2Link->m_path, end2EndSepctrum);

			SpectrumStateEX se(origEnd2Link->m_spectrum);
			if (se.TestIfNeighbour(end2EndSepctrum) == false)
				_ASSERT(0);

			origEnd2Link->m_spectrum. or (end2EndSepctrum);

			m_pTraffic_manager->ExtendRequest(nS, nT, width, duration, origEnd2Link->linkID);
			m_eGroomcnt++;
			m_pTraffic_manager->IncTime();
			return;
		}
		
		bGromming = m_pTraffic_manager->MatrixGroomingShortest(nS, nT, width, &origMGroomingLink1, &origMGroomingLink2,
			&mGroomSpectrum1, &mGroomSpectrum2);
		if (bGromming)
		{
			groomingLength = origMGroomingLink1->m_path.length() + origMGroomingLink2->m_path.length();
		}

		algorithmLink.m_path = m_algorithm->calcPath(s, t, width);
		algorithmLength = algorithmLink.m_path.length();

		//decide wich methdos paths to alloc*******************
		if ((algorithmLength > 0) || (groomingLength > 0))
		{
			if (((groomingLength < algorithmLength) && bGromming && (algorithmLength > 0)) ||
				(algorithmLength == 0) && ((double)groomingLength < (double)ShortestPathLength(s, t) * (double)1.5))
			{
				m_pSpectrumManager->ForceAlloc(origMGroomingLink1->m_path, mGroomSpectrum1);
				m_pSpectrumManager->ForceAlloc(origMGroomingLink2->m_path, mGroomSpectrum2);

				//debug
				//SpectrumStateEX se(origMGroomingLink1->m_spectrum);
				//if (se.TestIfNeighbour(mGroomSpectrum1) == false)
				//	_ASSERT(0);
				//SpectrumStateEX se1(origMGroomingLink2->m_spectrum);
				//if (se1.TestIfNeighbour(mGroomSpectrum2) == false)
				//	_ASSERT(0);

				origMGroomingLink1->m_spectrum. or (mGroomSpectrum1); //traffic matrix meglovo elemtet szelesitjuk
				origMGroomingLink2->m_spectrum. or (mGroomSpectrum2);
				
				m_pTraffic_manager->ExtendRequest(origMGroomingLink1->s, origMGroomingLink1->t, width, duration, origMGroomingLink1->linkID);
				m_pTraffic_manager->ExtendRequest(origMGroomingLink2->s, origMGroomingLink2->t, width, duration, origMGroomingLink2->linkID);
				m_mGroomcnt++;
			}
			else if (algorithmLength > 0)
			{
				if (m_pSpectrumManager->Alloc(width, algorithmLink.m_path, algorithmLink.m_spectrum))
				{
					m_pTraffic_manager->AddRequest(nS, nT, width, duration, algorithmLink); // TODO check if matrix stores correctly
				}
				else
				{
					*m_nBlockNum += 1;// (width * duration);
				}
			}
		}
		else
		{
			*m_nBlockNum += 1;// (width * duration);
		}
		m_pTraffic_manager->IncTime();
	}
};



template<typename GR> class MatrixGroomingContSpectrum : public SimulationMethod<GR> {

public:
	MatrixGroomingContSpectrum(Algorithm<GR> * algorithm):SimulationMethod<GR>( algorithm)
	{
		m_pTraffic_manager->SetSpectrumGrooming(eMatrixGroomingMethod::eContSpectrum);
	}

	//MatrixGroomingLimit(Algorithm<GR> * algorithm)
	//{
	//	m_algorithm = algorithm;
	//	m_graph = &(m_algorithm->m_graph);
	//	m_pSpectrumManager = m_algorithm->m_pSpectrum_manager;
	//	m_pTraffic_manager = m_algorithm->m_pTraffic_manager;
	//	m_nBlockNum = &(m_algorithm->m_nBlockNum);
	//}

	virtual void run(Node s, Node t, const int width, const long int duration) override
	{
		int nS = m_graph->id(s);
		int nT = m_graph->id(t);
		SpectrumState mGroomSpectrum1, mGroomSpectrum2, end2EndSepctrum;
		MatrixLink* origMGroomingLink1 = nullptr;
		MatrixLink* origMGroomingLink2 = nullptr;
		MatrixLink* origEnd2Link = nullptr;
		
		Link algorithmLink;
		int groomingLength = 0;
		int algorithmLength = 0;
		bool end2endGrooming = false;
		bool bGromming = false;

		//end2end grooming******
		end2endGrooming = m_pTraffic_manager->End2endGrooming(nS, nT, width, &origEnd2Link, &end2EndSepctrum);
		if (end2endGrooming)
		{
			m_pSpectrumManager->ForceAlloc(origEnd2Link->m_path, end2EndSepctrum);

			SpectrumStateEX se(origEnd2Link->m_spectrum);
			if (se.TestIfNeighbour(end2EndSepctrum) == false)
				_ASSERT(0);

			origEnd2Link->m_spectrum. or (end2EndSepctrum);

			m_pTraffic_manager->ExtendRequest(nS, nT, width, duration, origEnd2Link->linkID);
			m_eGroomcnt++;
			m_pTraffic_manager->IncTime();
			return;
		}

		bGromming = m_pTraffic_manager->MatrixGroomingShortest(nS, nT, width, &origMGroomingLink1, &origMGroomingLink2,
			&mGroomSpectrum1, &mGroomSpectrum2);
		if (bGromming)
		{
			groomingLength = origMGroomingLink1->m_path.length() + origMGroomingLink2->m_path.length();
		}

		algorithmLink.m_path = m_algorithm->calcPath(s, t, width);
		algorithmLength = algorithmLink.m_path.length();

		//decide wich methdos paths to alloc*******************
		if ((algorithmLength > 0) || (groomingLength > 0))
		{
			if (((groomingLength < algorithmLength) && bGromming && (algorithmLength > 0)) || 
				 (algorithmLength == 0) && ((double)groomingLength < (double)ShortestPathLength(s,t) * (double)1.5) )
			{
				m_pSpectrumManager->ForceAlloc(origMGroomingLink1->m_path, mGroomSpectrum1);
				m_pSpectrumManager->ForceAlloc(origMGroomingLink2->m_path, mGroomSpectrum2);
			
				//debug
				//SpectrumStateEX se(origMGroomingLink1->m_spectrum);
				//if (se.TestIfNeighbour(mGroomSpectrum1) == false)
				//	_ASSERT(0);
				//SpectrumStateEX se1(origMGroomingLink2->m_spectrum);
				//if (se1.TestIfNeighbour(mGroomSpectrum2) == false)
				//	_ASSERT(0);
				
				SpectrumStateEX se(origMGroomingLink1->m_spectrum);
				if (se.TestIfNeighbour(mGroomSpectrum1) == false)
				{
					Link newLink = *origMGroomingLink1;
					newLink.m_spectrum = mGroomSpectrum1;
					m_pTraffic_manager->AddRequest(origMGroomingLink1->s, origMGroomingLink1->t, width, duration, newLink);
				}
				else
				{
					origMGroomingLink1->m_spectrum. or (mGroomSpectrum1); //traffic matrix meglovo elemtet szelesitjuk
					m_pTraffic_manager->ExtendRequest(origMGroomingLink1->s, origMGroomingLink1->t, width, duration, origMGroomingLink1->linkID);
				}

				SpectrumStateEX se2(origMGroomingLink2->m_spectrum);
				if (se2.TestIfNeighbour(mGroomSpectrum2) == false)
				{
					Link newLink = *origMGroomingLink2;
					newLink.m_spectrum = mGroomSpectrum2;
					m_pTraffic_manager->AddRequest(origMGroomingLink2->s, origMGroomingLink2->t, width, duration, newLink);
				}
				else
				{
					origMGroomingLink2->m_spectrum. or (mGroomSpectrum2); //traffic matrix meglovo elemtet szelesitjuk
					m_pTraffic_manager->ExtendRequest(origMGroomingLink2->s, origMGroomingLink2->t, width, duration, origMGroomingLink2->linkID);
				}

				m_mGroomcnt++;
			}
			else if (algorithmLength > 0)
			{
				if (m_pSpectrumManager->Alloc(width, algorithmLink.m_path, algorithmLink.m_spectrum))
				{
					m_pTraffic_manager->AddRequest(nS, nT, width, duration, algorithmLink); // TODO check if matrix stores correctly
				}
				else
				{
					*m_nBlockNum += 1;// (width * duration);
				}
			}
		}
		else
		{
			*m_nBlockNum += 1;// (width * duration);
		}
		m_pTraffic_manager->IncTime();
	}
};
#endif RSA