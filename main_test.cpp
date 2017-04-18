#include "stdafx.h"
//#include "RSAAlgoritms.h"
#include<lemon/random.h>
#include<lemon/lp.h>
#include"Factory.h"
#define REQUESTS 1000

using namespace std;
using namespace lemon;


int CH::channel_num(50);
void TestMatrix();

int main() {
	
	std::vector<vector<int> > eu_servers;
	vector<int> v = { 12 };
	//eu_servers.push_back(v);
	v = { 4 };
	//eu_servers.push_back(v);
	v = { 16 };
	//eu_servers.push_back(v);
	//2:
	v = { 4,12 };
	//eu_servers.push_back(v);
	v = { 24,12 };
	//eu_servers.push_back(v);
	v = { 14,16};
	//eu_servers.push_back(v);
	//3:
	v = { 4,14,16 };
	eu_servers.push_back(v);
	v = { 8,24,16 };
	eu_servers.push_back(v);
	v = { 8,24,12 };
	eu_servers.push_back(v);
	//4:
	v = { 24,4,8,16 };
	eu_servers.push_back(v);
	v = { 22,16,12,4 };
	eu_servers.push_back(v);
	v = { 4,14,16,12 };
	eu_servers.push_back(v);
	

	int ch_num_tomb[] = { 40, 50, 60 };
	
	/*
	TODOS
	* forgalmi matrix
	* anycast
	* megoszott vedelem
	* anycast megosztottal
	* kötögelés
	* genetikus
	*/
	typedef ListGraph::Node Node;
	typedef ListGraph::Edge Edge;
	typedef PathNodeIt<Path<ListGraph> > PN;
	ListGraph graph;
	lemon::graphReader(graph, "28_eu.lgf").run();

	deparallel(graph);
	cout << "edge: " << lemon::countEdges(graph) << endl;
	cout << "node num: " << lemon::countNodes(graph) << endl;

	ListGraph::EdgeMap<int> lengthmap(graph);
	ListGraph::EdgeMap<bool> permittingmap(graph);
	
	RSABuilder rb;
	rb.setGraph(graph);
	rb.createSpectrumMap();			
	rb.setAllocMethod(new BaseSpectrumCheck);
	ModDijkstra<ListGraph> *mod= rb.createModDijkstra();
	mod->run(graph.nodeFromId(1), graph.nodeFromId(9),10,10);
	//mod->printAllocatedNodes();
	SpectrumMap* map = mod->getManager()->getMap();
	//printSpectrum(map, graph);
	//cout << endl;
	//printSpectrum(rb.getSpectrumMap(),graph);
	

	//TestMatrix();
			
}

//make a test graph, test cases
class TestGraph1
{
	ListGraph graph;
	Node A,B,C,D,E,F;
	Edge ab, ac, bc, bd, be, ce, cd, de, df, ef;
	
public:

	TestGraph1()
	{
		A = graph.addNode();
		B = graph.addNode();
		C = graph.addNode();
		D = graph.addNode();
		E = graph.addNode();
		F = graph.addNode();

		// arcs
		ab = graph.addEdge(A, B);
		ac = graph.addEdge(A, C);
		bc = graph.addEdge(B, C);
		bd = graph.addEdge(B, D);
		be = graph.addEdge(B, E);
		ce = graph.addEdge(C, E);
		cd = graph.addEdge(C, D);
		de = graph.addEdge(D, E);
		df = graph.addEdge(D, F);
		ef = graph.addEdge(E, F);
	}

	void CreateEps()
	{
		typedef dim2::Point<int> Point;
		
		ListGraph::NodeMap<Point> coords(graph);
		ListGraph::NodeMap<double> sizes(graph);
		ListGraph::NodeMap<int> colors(graph);
		ListGraph::NodeMap<int> shapes(graph);
		ListGraph::EdgeMap<int> acolors(graph);
		ListGraph::EdgeMap<int> widths(graph);

		coords[A] = Point(50, 50);  sizes[n1] = 1; colors[n1] = 1; shapes[n1] = 0;
		coords[B] = Point(50, 70);  sizes[n2] = 2; colors[n2] = 2; shapes[n2] = 2;
		coords[C] = Point(70, 70);  sizes[n3] = 1; colors[n3] = 3; shapes[n3] = 0;
		coords[D] = Point(70, 50);  sizes[n4] = 2; colors[n4] = 4; shapes[n4] = 1;
		coords[E] = Point(85, 60);  sizes[n5] = 3; colors[n5] = 5; shapes[n5] = 2;
		coords[F] = Point(85, 60);  sizes[n5] = 3; colors[n5] = 5; shapes[n5] = 2;

		Arc a;

		a = g.addArc(n1, n2); acolors[a] = 0; widths[a] = 1;
		a = g.addArc(n2, n3); acolors[a] = 0; widths[a] = 1;
		a = g.addArc(n3, n5); acolors[a] = 0; widths[a] = 3;
		a = g.addArc(n5, n4); acolors[a] = 0; widths[a] = 1;
		a = g.addArc(n4, n1); acolors[a] = 0; widths[a] = 1;
		a = g.addArc(n2, n4); acolors[a] = 1; widths[a] = 2;
		a = g.addArc(n3, n4); acolors[a] = 2; widths[a] = 1;

		IdMap<ListDigraph, Node> id(g);

		// Create .eps files showing the digraph with different options
		cout << "Create 'graph_to_eps_demo_out_1_pure.eps'" << endl;
		graphToEps(g, "graph_to_eps_demo_out_1_pure.eps").
			coords(coords).
			title("Sample .eps figure").
			copyright("(C) 2003-2009 LEMON Project").
			run();
	}

};
void Simulation()
{
	Random random;
}
int Cantor(int k1, int k2)
{
	int c = 0.5*(k1 + k2)*(k1 + k2 + 1) + k2;
	return c;
}

#if 0
void TestMatrix()
{
	struct mano
	{
		int i1, i2;
		int val;
	};
	TrafficMatrix matrix(28);
	matrix.Print();
	Random random;
	lemon::Random random1(random);
	unordered_map<int,mano> idxs;
	
	idxs.reserve(REQUESTS);

	for (int i = 0; i < REQUESTS; i++)
	{
		int n1 = random1.integer(0, 27);
		int n2 = random1.integer(0, 27);
		
		long dur = (long int)random1.exponential(0.03); // 0.03
		int width1 = random1.integer(1, 5);
		int val = random1.integer(1, 100);

		if (n1 != n2) {
			if (n2 > n1)
			{
				int t = n2;
				n2 = n1;
				n1 = t;
			}
			if ((n1 == 1 || n2 == 1) && (n1 == 0 || n2 == 0))
			{
				cout << "m";
			}
			mano m;
			m.i1 = n1; m.i2 = n2; m.val = val;
			int c = Cantor(n1,n2);
			//idxs.insert(c,m);
			idxs[c] = m;
			matrix.A(n1,n2, val);
		}
	}
	mano m;
	for (auto it = idxs.begin(); it != idxs.end(); it++)
	{
		m = it->second;
		int c = Cantor(m.i1, m.i2);
		int ref2 = idxs[c].val;
		int ref = matrix.Get(m.i1, m.i2);
		if (ref != m.val)
		{
			_ASSERT(0);
		}
		cout << ref;
	}
	// bejaras

	matrix.Print();
}
#endif