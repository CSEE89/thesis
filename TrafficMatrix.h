#include"general_traits.h"
#include <unordered_map>
/**/
#ifndef TRAFFIC_MATRIX
#define TRAFFIC_MATRIX


struct Link {
	SpectrumState		m_spectrum;
	Path<ListGraph>		m_path;
};

struct MatrixNode {
	map<int,Link> m_Links;

};
#if 0
struct Link {
	SpectrumState		m_spectrum;
	Path<ListGraph>		m_path;		
};

struct MatrixNode {
	vector<Link> m_Links;

	MatrixNode()
	{
		m_Links.reserve(5);
	}
};
#endif

class TrafficMatrix
{
	unordered_map<int, MatrixNode*>	m_data;
	int								m_nodeNum;
	int								m_elementNum;

public:
	TrafficMatrix(int n)
	{
		m_nodeNum = n;
		m_elementNum = (n * (n - 1)) / 2;
		m_data.reserve(m_elementNum + m_elementNum / 2);
		m_nodeNum = n;
		MatrixNode* val = nullptr;
		int key;
		int cnt = 0;
		for (int i = 0; i < m_nodeNum; i++)
		{
			for (int j = i + 1; j < m_nodeNum; j++)
			{
				key = CalcIndex(i, j);
				val = new MatrixNode;
				m_data.emplace(key, val);
				cnt++;
			}
		}
		cout << "tenyleges elemszam: " << cnt;
	}

	~TrafficMatrix()
	{
		int key;
		for (int i = 0; i < m_nodeNum; i++)
		{
			for (int j = i + 1; j < m_nodeNum; j++)
			{
				key = CalcIndex(i, j);				
				MatrixNode* val = m_data[key];
				delete val;
			}
		}
	}
	
	void Add(int r, int c, Link data) //sor oszlop
	{
		int n = CalcIndex(r, c);
		MatrixNode* node = m_data[n];
		node->m_Links[data.m_path.length()]=(data); //todo test
	}

	MatrixNode* Get(int r, int c)
	{
		int n = CalcIndex(r, c);
		return m_data[n];
	}

	int GetNodeNum()
	{
		return m_nodeNum;
	}

	void Print()
	{
		for (auto it = m_data.begin(); it != m_data.end(); it++)
		{
		//	cout <<it->first<< " : " <<it->second<<endl;
		}
	}

protected:
	inline int CalcIndex(int k1, int k2)
	{
		if (k1 > m_nodeNum || k2 > m_nodeNum || k1 == k2)
		{
			_ASSERT(0);
			cout << "wrong matrix data";
		}
		if (k1 > k2)
		{
			int t = k1;
			k1 = k2;
			k2 = t;
		}
		int c = 0.5*(k1 + k2)*(k1 + k2 + 1) + k2; // cantor
		return c;
	}

};


struct Request //int a[4];
{

	long int	m_dur;
	int			m_width;
	int			m_s;
	int			m_t;
	Request(long int dur, int width, int s, int t) :m_dur(dur), m_width(width), m_s(s), m_t(t) {}
};

class TrafficManager 
{
	std::vector<Request>	m_Requests;
	TrafficMatrix			m_trafficMatrix;
	SpectrumManager*		m_pSpectrummanager;
public:
	TrafficManager(int requests, int nodeNum) :m_trafficMatrix(nodeNum)
	{
		m_Requests.reserve(requests);
	}
	void SetSpectrumManager(SpectrumManager *p) {
		m_pSpectrummanager = p;

	}
	void AddRequest(int s, int t, int width, long int duration, SpectrumState linkspectrum)
	{
		Request req(duration,width,s,t);
		m_Requests.push_back(req);
		//m_trafficMatrix.Add(s,t,linkspectrum);
	}

	void IncTime()
	{
		int size = m_Requests.size();
		Request* ptr = m_Requests.data();
		for (int i = 0; i < size; i++ )
		{
			ptr->m_dur--;
			if (ptr->m_dur == 0)
			{
				DecreaseLink(ptr->m_s, ptr->m_t, ptr->m_width);
			}
		}
	}

	inline int IsThereRoom(SpectrumState spectrum, SpectrumState pathSpectrum, int width, SpectrumState &toall) 
	{
		// az adott link spectruma mellett az ut spectrumban van e hely??
		// megnezzuk hol kezdodik es hol van vege
		bool ok = true;
		int retpos;
		SpectrumStateEX toAlloc(toall);
		SpectrumStateEX StEx(spectrum);
		int begin, end;
		StEx.BeginEnd(begin, end);
		retpos = begin;
		begin -= width;
		if (begin > 0)
		{
			for (int i = begin; i < begin + width; i++)
			{
				if (pathSpectrum[i])
				{
					ok = false;
					break;
				}
				toAlloc[i] = 1;
			}
			if (ok)
			{
				return retpos;
			}
		}
		toAlloc.Zero();
		if (end + width < BITSETCNT)
		{
			ok = true;
			retpos = end;
			for (int i = end; i < end + width; i++)
			{
				if (pathSpectrum[i])
				{
					ok = false;
					break;
				}
				toAlloc[i] = 1;
			}
			if (ok)
			{
				return retpos;
			}
		}
		return -1;
	}
protected:
	bool CheckSpectrumGrooming(Link l1, Link l2, int width)
	{
		// megy ut a-bol c be és c- bol b -be
		// megnezzuk hogy az egyik ut mellett van - e hely, és másik mellett?
		SpectrumState toAlloc1, toAlloc2;
		SpectrumState pathSpectrum1 = m_pSpectrummanager->getPathSpectrum(l1.m_path);
		SpectrumState pathSpectrum2 = m_pSpectrummanager->getPathSpectrum(l2.m_path);
		int allocpos1 = IsThereRoom(l1.m_spectrum,pathSpectrum1,width, toAlloc1);
		int allocpos2 = IsThereRoom(l1.m_spectrum,pathSpectrum1,width, toAlloc2);
		

		if (allocpos1 != -1 && allocpos2 != -1)
		{
			//alloc
			m_pSpectrummanager->ForceAlloc(toAlloc1, l1.m_path);
			m_pSpectrummanager->ForceAlloc(toAlloc2, l2.m_path);
			return true;
		}
		return false;
	}
	bool EndToEndGromming() 
	{

	}

public:
	//
	bool Grooming(int s, int t, int width) 
	{
		int nodeNum = m_trafficMatrix.GetNodeNum();
		vector<int> l1; l1.reserve(nodeNum);
		
		map<int,int> groomingPaths;
		for (int i = 0; i < nodeNum; i++) // forrasbol hova megy
		{
			if ( (!m_trafficMatrix.Get(s, i)->m_Links.empty() )
				&& (!m_trafficMatrix.Get(i, t)->m_Links.empty()) ) 
			{
				l1.push_back(i);     // TODO 2 utvanl hosszat összad , e szerint berak set-be 
			}
			Path<ListGraph> p;

		}
		
		for (int i = 0; i < l1.size(); i++)
		{
			MatrixNode* n1 = m_trafficMatrix.Get(s,i);
			MatrixNode* n2 = m_trafficMatrix.Get(i, t);
			if (CheckSpectrumGrooming(n1->m_Links.begin()->second, n2->m_Links.begin()->second,width) )
			{
				
				return true;
			}
		}

		//
		/*for (int i = 0; i < m_nodeNum; i++)
		{
			for (int j = i + 1; j < m_nodeNum; j++)
			{
				key = CalcIndex(i, j);
				val = new MatrixNode;
				m_data.emplace(key, val);
				cnt++;
			}
		}*/
		return false;
		// if there is link to a-c and c-b try to allocate next to them
		//m_trafficMatrix.
		//for(int i)
	}
	struct slice 
	{
		int pos;
		int width;
	};
	void DecreaseLink(int s, int t, int width)
	{
		//SpectrumState linksSpectrum = m_trafficMatrix.Get(s,t);
		int gapwidth = 0;
		// cel a töredezettseg csokkentese, elöszr ha van töredeket törlunk
		std::set<slice> slices;
		// delete the smallest or from biggest -> full of smallest or part?
	}

};

struct TrafficLink
{
	SpectrumState spectrum;
};




class RequestMatrix 
{
	int* m_data;
	int m_nodeNum;

public:
	RequestMatrix(int n)
	{
		m_nodeNum = n;
		m_data = new int[n*n];
		for (int i = 0; i < n*n; i++)
		{
			m_data[i] = 0;
		}
	
	}

	~RequestMatrix()
	{
		delete[] m_data;
	}

	void Set(int r, int c, int data) //sor oszlop
	{
		int n = CalcIndex(r, c);
		m_data[n] = data;
	}

	void Add(int r, int c, int data) //sor oszlop
	{
		int n = CalcIndex(r, c);
		m_data[n] += data;
	}

	int Get(int r, int c)
	{
		int n = CalcIndex(r, c);
		return m_data[n];
	}

	void Print() 
	{
		for (int i = 0; i < m_nodeNum; i++)
		{
			for (int j = 0; j < m_nodeNum; j++)
			{
				cout << m_data[i*m_nodeNum + j] << " ";
			}
			cout << endl;
		}
	}

protected:
	int CalcIndex(int r, int c) 
	{
		if (r > m_nodeNum || c > m_nodeNum || r == c)
		{
			_ASSERT(0);
			cout << "wrong matrix data";
		}
		if (r > c)
		{
			int t = r;
			r = c;
			c = t;
		}
		int n = (r )*m_nodeNum + (c );
		return n;
	}

};

#if 0
class TrafficMatrix{
	//std::multimap<int, PathMatrix> path_matrix;
	//std::multimap<int, Link> traffic_matrix;
	std::vector<Link> links;
	//Link links link_array[n];
public:
	void insert(Path<ListGraph> path, Node s, Node t, int pos, int width, long int timestamp)
	{
		Link link(path, s, t, pos, width, timestamp);
		insert(link);
	}
	void insert(Link link){
		links.push_back(link);
		/*
		global_key++;
		int key(0), s(0), t(0);
		ListGraph::Node start = graph->source(path.front());
		ListGraph::Node end = graph->target(path.back());
		s = graph->id(start);
		t = graph->id(end);
		if (s>t){ int tmp = s; s = t; t = tmp; }
		key = s*(n - 1) + t;
		PathMatrix tempMatrix(global_key, path, pos, width, timestamp);
		path_matrix.insert(std::pair<int, PathMatrix>(key, tempMatrix));
		*/
	}
	void deleteFrom(){}
	void getElement(int i){}
	
};
#endif
#endif TRAFFIC_MATRIX