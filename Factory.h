#include"RSAAlgoritms.h"

template<typename T> class BaseBuilder{	
protected:
	T* m_result;
	ListGraph* graph = NULL;
	SpectrumMap* map = NULL;
public:
	BaseBuilder()
	{
	}
	BaseBuilder(const BaseBuilder &b)
	{
		cout << "Builder copy CALLED";
		graph = b.graph;
		//map = b.map; //wrong
		map = new SpectrumMap(*graph);
		for (ListGraph::EdgeIt it(*graph); it != INVALID; ++it){
			map->set(it, b.map->operator[](it));
		}
	}
	SpectrumMap* getSpectrumMap()
	{
		return map;
	}
	virtual void setGraph(ListGraph &p_graph)
	{ 		
		this->graph = &p_graph; 
	}
	virtual void setSpectrumMap(SpectrumMap &s_map) // TODO make local copy
	{ 
		map = new SpectrumMap(*graph);
		ListGraph::EdgeIt it(*graph);
		for ( it ; it != INVALID; ++it)
		{
			map->set(it,s_map[it]);
		}
		//map = &s_map; 
	} 
	virtual SpectrumMap* createSpectrumMap()
	{		
		if (graph != NULL)
		{
			map = new SpectrumMap(*graph);
		}
		else{
			cout << "ERROR BASE BUILDER SET GRAPH FIRST" << endl;
		}
		return map;
	}
	virtual T* create(int hint) abstract;
	virtual ~BaseBuilder()
	{
		cout << "memory leak";
	}
};

class RSABuilder : public BaseBuilder < Algorithm<ListGraph> > 
{

	SpectrumManager *m_spectrumManager;
	AllocMethod* method;
public:
	ModDijkstra<ListGraph>* createModDijkstra(int reqhint){
		m_result = new ModDijkstra<ListGraph>(*graph, reqhint);
		m_spectrumManager = new SpectrumManager(graph, map, *method);
		m_result->setSpectrumManager(m_spectrumManager);
		return dynamic_cast<ModDijkstra<ListGraph>* >(m_result);
	}
	ModDijkstra<ListGraph>* create(int reqnum){
		return createModDijkstra(reqnum);
	}
	void setAllocMethod(AllocMethod *p){
		this->method = p;
	}

	Kshort<ListGraph>* createKshort(int k,int reqhint) 
	{
		m_result = new Kshort<ListGraph>(*graph,reqhint);
		dynamic_cast<Kshort<ListGraph>*>(m_result)->setK(k);
		m_spectrumManager = new SpectrumManager(graph, map, *method);
		m_result->setSpectrumManager(m_spectrumManager);
		return dynamic_cast<Kshort<ListGraph>* >(m_result);
	}
	~RSABuilder()
	{
		if (m_result != nullptr)
		{
			delete m_result;
			m_result = nullptr;
		}
		if (m_spectrumManager != nullptr)
		{
			delete m_spectrumManager;
			m_spectrumManager = nullptr;
		}
	}
};