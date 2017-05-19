#include"general_traits.h"
#include <unordered_map>
/**/
#ifndef TRAFFIC_MATRIX
#define TRAFFIC_MATRIX



class MatrixLink : public Link
{
public:
	int s;
	int t;
	int linkID;
};

struct MatrixNode {
	map<int, MatrixLink> m_Links; //egy adott hosszhoz tobb ut is tartozhat

	MatrixLink GetShortest() {return  m_Links.begin()->second; }

	//void Insert();
};
#if 0

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
	
	int Add(int r, int c, Link link) //sor oszlop
	{
		int n = CalcIndex(r, c);
		MatrixNode* node = m_data[n];
		int key = link.m_path.length();
		map<int, MatrixLink>::iterator it = node->m_Links.find(key);
		
		while(it != node->m_Links.end())
		{
			key++;
			it = node->m_Links.find(key);
		}
		//TEST if exist
		//node->m_Links.find(key);
		MatrixLink* refLink		= &(node->m_Links[key]);
		refLink->m_path			= link.m_path;
		refLink->m_spectrum		= link.m_spectrum;
		refLink->s				= r;
		refLink->t				= c;
		refLink->linkID			= key;
		return key;
		//node->m_Links.insert(std::pair<int,Link> (key,link));
	}

	//void Extend(int r, int c, Link data)
	//{
	//	//MatrixNode * n = Get(r, c);
	//	//n
	//}

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
	Link		link;
	long int	m_dur;
	int			m_width;
	int			m_s;
	int			m_t;
	int			m_iLinkID;
	Request(long int dur, int width, int s, int t) :m_dur(dur), m_width(width), m_s(s), m_t(t) {}
};

enum eMatrixGroomingMethod {
	eSplittedSpectrum,
	eContSpectrum
};

class TrafficManager 
{
	std::vector<Request>	m_Requests;
	TrafficMatrix			m_trafficMatrix;
	SpectrumManager*		m_pSpectrummanager;
	eMatrixGroomingMethod	m_eMethod;

public:
	TrafficManager(int requests, int nodeNum) :m_trafficMatrix(nodeNum)
	{
		m_Requests.reserve(requests);
	}

	void SetSpectrumManager(SpectrumManager *p) {
		m_pSpectrummanager = p;
	}

	void SetSpectrumGrooming(eMatrixGroomingMethod e)
	{
		m_eMethod = e;
	}

	void AddRequest(int s, int t, int width, long int duration, Link link)
	{
		Request req(duration,width,s,t);
		req.link = link;
		int linkId = m_trafficMatrix.Add(s,t,link);
		req.m_iLinkID = linkId;
		m_Requests.push_back(req);
	}
	
	void ExtendRequest(int s, int t, int width, long int duration,int linkID)
	{
		Request req(duration, width, s, t);
		req.m_iLinkID = linkID;
		m_Requests.push_back(req);
	}

	//Alloc es matrix management egyutt
public:
	void AddNewLink();
	void AddNewGroomLink();

	///* egy request lejartakor vagy toroljuk a linket a matrixbol is, \
	vagy ha extendalva volt csak a megfelelo spektrumszelettel xor
	void IncTime()
	{
		int size = m_Requests.size();
		for (int i = 0; i < m_Requests.size(); i++)
		{
			m_Requests[i].m_dur--;
			if (m_Requests[i].m_dur == 0)
			{
				DecreaseLink(m_Requests[i]);
				//SimpleDecreaseLink(m_Requests[i]);
			}
		}
	}

	int GetActive()
	{
		int active = 0;
	
		for (int i = 0; i < m_Requests.size(); i++)
		{
			
			if (m_Requests[i].m_dur > 0)
				active++;
		}
		return active;
	}

	int EqualWithReq()
	{
		SpectrumMap sm(*(m_pSpectrummanager->GetGraph()));
		SpectrumManager  manager = *m_pSpectrummanager;
		manager.SetMap(&sm);
		ListGraph* gr = m_pSpectrummanager->GetGraph();
		//for every req alloc in sm
		for (int i = 0;i < m_Requests.size();i++)
		{
			if (m_Requests[i].m_dur > 0)
			{
				manager.ForceAlloc(m_Requests[i].link.m_path, m_Requests[i].link.m_spectrum);
			}
		}
		SpectrumMap* managerMap = m_pSpectrummanager->GetMap();
		ListGraph::EdgeIt eit(*gr);
		//cout << endl;
		for (eit; eit != INVALID; ++eit)
		{
			SpectrumState orig = (*managerMap)[eit];
			SpectrumState sreq = sm[eit];
			//managerMap->operator[](eit) == sm->
			//if (!(orig == sreq))
				//_ASSERT(0);
		}
		return 0;
	}

	inline int IsThereRoom(SpectrumState spectrum, SpectrumState pathSpectrum, int width, SpectrumState *toall) 
	{
		// az adott link spectruma mellett az ut spectrumban van e hely??
		// megnezzuk hol kezdodik es hol van vege
		bool ok = true;
		int retpos;
		SpectrumStateEX toAlloc(*toall);
		SpectrumStateEX StEx(spectrum);
		int begin, end;
		StEx.BeginEnd(begin, end);
		
		begin -= width;
		if (begin < 0)
			begin = 0;
		retpos = begin;
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
				*toall = (SpectrumState)toAlloc;
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
				*toall = (SpectrumState)toAlloc;
				return retpos;
			}
		}
		return -1;
	}
protected:
	///* megy ut a-bol c be és c- bol b -be
	// megnezzuk hogy az egyik ut mellett van - e hely, és másik mellett?\
	// Link spektrumat atirjuk alloc spektrumra
	bool CheckSpectrumMatrixGrooming(Link l1, Link l2, int width, SpectrumState* toAlloc1, SpectrumState* toAlloc2)
	{
		SpectrumState pathSpectrum1 = m_pSpectrummanager->getPathSpectrum(l1.m_path);
		SpectrumState pathSpectrum2 = m_pSpectrummanager->getPathSpectrum(l2.m_path);
		SpectrumState tpA1;
		SpectrumState tpA2;
		int allocpos1 = IsThereRoom(l1.m_spectrum,pathSpectrum1,width, &tpA1);
		if (allocpos1 == -1)
			return false;
		int allocpos2 = IsThereRoom(l2.m_spectrum,pathSpectrum1,width, &tpA2);
		
		if ((allocpos1 != -1) && (allocpos2 != -1))
		{
			*toAlloc1 = tpA1;
			*toAlloc2 = tpA2;
			return true;
		}
		return false;
	}

	//1 a hosszabb szakaszon mellett a masikban ugyanabban savban van e hely?
	bool CheckSpectrumMatrixGroomingCont(Link l1, Link l2, int width, SpectrumState* toAlloc1, SpectrumState* toAlloc2)
	{
		SpectrumState pathSpectrum1 = m_pSpectrummanager->getPathSpectrum(l1.m_path);
		SpectrumState pathSpectrum2 = m_pSpectrummanager->getPathSpectrum(l2.m_path);
		SpectrumState sumPathSpectrum = pathSpectrum1;
		SpectrumState toAll;
		sumPathSpectrum. or (pathSpectrum2);
		
		Link* longerLink = (l1.m_path.length() > l2.m_path.length()) ? &l1 : &l2;		
		int allocpos = IsThereRoom(longerLink->m_spectrum, sumPathSpectrum, width, &toAll);
		if (allocpos != -1)
		{
			*toAlloc1 = toAll;
			*toAlloc2 = toAll;
			return true;
		}
		return false;
	}

public:
	bool End2endGrooming(int s, int t, int width,MatrixLink** orig, SpectrumState* toAlloc)
	{
		MatrixNode* n = m_trafficMatrix.Get(s, t);
		if (! n->m_Links.empty())
		{
			for (auto it = n->m_Links.begin();it != n->m_Links.end(); it++)
			{
				//Link link = n->m_Links.begin()->second;
				SpectrumState pathSpectrum = m_pSpectrummanager->getPathSpectrum(it->second.m_path);
				int allocpos = IsThereRoom(it->second.m_spectrum, pathSpectrum, width, toAlloc);
				if (allocpos != -1)
				{
					*orig = &(it->second);
					return true;
				}
			}
		}
		return false;
	}

	bool MultiLevelMatrixGrooming(int s, int t, int width, MatrixLink **orig1, MatrixLink** orig2, SpectrumState *retLink1, SpectrumState *retLink2)
	{

	}

	// s-bol t-be w szelesseg 	
	//bool Grooming(int s, int t, int width,Link *orig1,Link* orig2, Link *retLink1,Link *retLink2) 
	bool MatrixGroomingShortest(int s, int t, int width, MatrixLink **orig1, MatrixLink** orig2, SpectrumState *retLink1, SpectrumState *retLink2)
	{
		int nodeNum = m_trafficMatrix.GetNodeNum();
		typedef int LENGTH;		
		std::multimap<int, int> interNodes;
		
		for (int i = 0; i < nodeNum; i++) // vegignezzuk a matrix minden elemet hogy megy e forrasbol oda ut
		{
			if ((i != s) && (i != t))
			{
				MatrixNode *n1 = m_trafficMatrix.Get(s, i);
				MatrixNode *n2 = m_trafficMatrix.Get(i, t);
				if ((!n1->m_Links.empty()) && (!n2->m_Links.empty()))
				{
					int length = n1->GetShortest().m_path.length() + n2->GetShortest().m_path.length();
					interNodes.insert(std::pair<int,int>(length,i));
					//interNodes[length] = i;					
				}
			}
		}
		
		for (auto it = interNodes.begin(); it != interNodes.end(); it++)
		{
			int iLookUp = it->second;
			MatrixLink* l1 = &(m_trafficMatrix.Get(s, iLookUp)->m_Links.begin()->second);
			MatrixLink* l2 = &(m_trafficMatrix.Get(iLookUp, t)->m_Links.begin()->second);
			//linkID1 = (m_trafficMatrix.Get(s, iLookUp)->m_Links.begin()->first);
			//linkID2 = (m_trafficMatrix.Get(iLookUp, t)->m_Links.begin()->first);
			bool bSpectrum;
			switch (m_eMethod)
			{
				case eMatrixGroomingMethod::eSplittedSpectrum:
				{
					bSpectrum = CheckSpectrumMatrixGrooming(*l1, *l2, width, retLink1, retLink2);
					break;
				}
				case eMatrixGroomingMethod::eContSpectrum:
				{
					bSpectrum = CheckSpectrumMatrixGroomingCont(*l1, *l2, width, retLink1, retLink2);
					break;
				}
			}
			if (bSpectrum)
			{
				*orig1 = l1;
				*orig2 = l2;				
				return true;
			}
		}
		return false;
	}

#if 0
	//
	bool MultiPathMatrixGrooming(int s, int t, int width, Link **orig1, Link** orig2, SpectrumState *retLink1, SpectrumState *retLink2)
	{
		int nodeNum = m_trafficMatrix.GetNodeNum();
		typedef int LENGTH;
		struct Paths{
			Link l1;
			Link l2;
		};
		std::multimap<int, Paths> interNodes;



		for (int i = 0; i < nodeNum; i++) // vegignezzuk a matrix minden elemet hogy megy e forrasbol oda ut
		{
			if ((i != s) && (i != t))
			{
				MatrixNode *n1 = m_trafficMatrix.Get(s, i);
				MatrixNode *n2 = m_trafficMatrix.Get(i, t);
				if ((!n1->m_Links.empty()) && (!n2->m_Links.empty()))
				{
					//combine
				}
			}
		}

		for (auto it = interNodes.begin(); it != interNodes.end(); it++)
		{
			int iLookUp = it->second;
			Link* l1 = &(m_trafficMatrix.Get(s, iLookUp)->m_Links.begin()->second);
			Link* l2 = &(m_trafficMatrix.Get(iLookUp, t)->m_Links.begin()->second);

			if (CheckSpectrumMatrixGrooming(*l1, *l2, width, retLink1, retLink2))
			{
				*orig1 = l1;
				*orig2 = l2;
				return true;
			}
		}

		return false;
		// if there is link to a-c and c-b try to allocate next to them
		//m_trafficMatrix.
		//for(int i)
	}
#endif
	/*void DecreaseLink1(Request req)
	{
		// grooming 2 link
		// traffic_matrixban keresek leghosszabbtol vissza eleg kapacitast //feltelezem h 2 csomopont koyti linkek osszkapacitasa pont akkor mint ami a a requestvektroban van, osszes elem a 2 csp kozt 
		// kozben torlom traffic matrixbol ha vminek 0 a kapacitasa
		// spektrumbol deallok 
		MatrixNode* n = m_trafficMatrix.Get(req.m_s, req.m_t);
		int needWidth = req.m_width;
		for (auto it = n->m_Links.end(); it != n->m_Links.begin() && needWidth > 0; it--)
		{
			//it->second.m_spectrum
			int w = CalcWith()
				if (w > needWidth)
				{
				}
		}
		m_pSpectrummanager->Dealloc(link);
	}*/

	//pont azt torlom ami le volt foglalva, a grooming is tobb link

	void SimpleDecreaseLink(Request req)
	{
		m_pSpectrummanager->Dealloc(req.link);
	}

	void DecreaseLink(Request req)
	{
		MatrixNode* n = m_trafficMatrix.Get(req.m_s,req.m_t);
		if (n->m_Links.find(req.m_iLinkID) == n->m_Links.end()) //ennek az idnek szerepelni kel amit itt ki nem toroltuk
			_ASSERT(0);
		Link * link = &(n->m_Links[req.m_iLinkID]);
		SpectrumStateEX sex(link->m_spectrum);
		Link toDealloc;
		toDealloc.m_path = link->m_path;
		if (sex.DeallocAndInvert(req.m_width, &(toDealloc.m_spectrum)))
		//sex.valami(1, 2);
		{
			n->m_Links.erase(req.m_iLinkID);
		}
		else {
			link->m_spectrum = sex;
		}

		m_pSpectrummanager->Dealloc(toDealloc);
	}

	//void DecreaseLink(Request req)
	//{
	//	MatrixNode* n = m_trafficMatrix.Get(req.m_s, req.m_t);
	//	
	//	m_pSpectrummanager->Dealloc(toDealloc);
	//}

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