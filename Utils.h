#include"general_traits.h"
#ifndef UTILS
#define UTILS



	void deparallel(ListGraph &graph)
	{
		Node u, v;

		for (ListGraph::EdgeIt eit(graph); eit != INVALID; ++eit)
		{
			u = graph.u(eit);
			v = graph.v(eit);

			for (ConEdgeIt<ListGraph> it(graph, u, v); it != INVALID; ++it)
			{
				if (it != eit)graph.erase(eit);
			}
		}
	}


/**
* GaPFill kitöltési stratégiához
*/
struct GAP{
	int pos;
	int width;
	GAP(int pos, int width) :pos(pos), width(width){}
	GAP operator()(int pos, int width)
	{
		GAP g(pos, width);
		return g;
	}
};
class GAPcmp{
public:
	bool operator()(const GAP &g1, const GAP &g2)
	{
		return g1.width>g2.width;
	}
};


class AllocMethod {
public:
	virtual int makeLinkSpectrum(const int width, SpectrumState &spectrum) = 0;
	virtual ~AllocMethod(){
	
	}
};
class TwoSideSpectrumCheck : public AllocMethod {
public:
	int makeLinkSpectrum(const int width, SpectrumState &spectrum) {
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
};

class BaseSpectrumCheck : public AllocMethod{
public:
	int makeLinkSpectrum(const int width, SpectrumState &spectrum) {
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
};
#endif // !UTILS
