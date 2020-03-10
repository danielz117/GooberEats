#include "provided.h"
#include <list>
#include "ExpandableHashMap.h"
#include <queue>
using namespace std;

class PointToPointRouterImpl
{
public:
	PointToPointRouterImpl(const StreetMap* sm);
	~PointToPointRouterImpl();
	DeliveryResult generatePointToPointRoute(
		const GeoCoord& start,
		const GeoCoord& end,
		list<StreetSegment>& route,
		double& totalDistanceTravelled) const;
private:
	const StreetMap* streetmap;	
	struct Compare {
		bool operator()(pair<StreetSegment*, double> const& p1, pair<StreetSegment*, double> const& p2)
		{
			return p1.second > p2.second;
		}
	};
};

PointToPointRouterImpl::PointToPointRouterImpl(const StreetMap* sm)
{
	streetmap = sm;
}

PointToPointRouterImpl::~PointToPointRouterImpl()
{
}

DeliveryResult PointToPointRouterImpl::generatePointToPointRoute(
	const GeoCoord& start,
	const GeoCoord& end,
	list<StreetSegment>& route,
	double& totalDistanceTravelled) const
{
	ExpandableHashMap<GeoCoord, StreetSegment*> coords;
	ExpandableHashMap<StreetSegment*, bool> breadcrumbs;
	vector<StreetSegment> test;
	if (start == end) {
		route.clear();
		totalDistanceTravelled = 0;
		return DELIVERY_SUCCESS;
	}
	else if (streetmap->getSegmentsThatStartWith(start, test) == false &&
		streetmap->getSegmentsThatStartWith(end, test) == false) {
		return BAD_COORD;
	}
	else {
		bool routeFound = false;
		GeoCoord next;
		priority_queue<pair<StreetSegment*, double>, vector<pair<StreetSegment*, double>>, Compare> possible;

		vector<StreetSegment> first;
		streetmap->getSegmentsThatStartWith(start, first);
		double g;
		double h;
		double f;
		StreetSegment best;
		for (int i = 0; i < first.size(); i++) {
			g = distanceEarthMiles(start, first[i].end);
			h = distanceEarthMiles(end, first[i].end);
			f = g + h;
			breadcrumbs.associate(&first[i], true);
			pair<StreetSegment*, double> insert;
			insert.first = &first[i];
			insert.second = f;
			possible.push(insert);
		}
		best = *possible.top().first;
		coords.associate(start, &best);
		next = best.end;

		while (!(possible.empty())) {
			if (next == end) {
				routeFound = true;
				break;
			}
			vector<StreetSegment> seg;
			streetmap->getSegmentsThatStartWith(next, seg);
			double g;
			double h;
			double f;
			StreetSegment best;
			for (int i = 0; i < seg.size(); i++) {
				if (breadcrumbs.find(&seg[i]) == nullptr) {
					g = distanceEarthMiles(start, seg[i].end);
					h = distanceEarthMiles(end, seg[i].end);
					f = g + h;
					breadcrumbs.associate(&seg[i], true);
					pair<StreetSegment*, double> insert;
					insert.first = &seg[i];
					insert.second = f;
					possible.push(insert);
				}

			}
			best = *possible.top().first;
			coords.associate(next, &best);
			next = best.end;
		}
		if (!(routeFound)) {
			return NO_ROUTE;
		}
		else {
			while (next != start) {
				StreetSegment insert;
				insert = **(coords.find(next));
				route.push_back(insert);
			}
			return DELIVERY_SUCCESS;
		}
	}
}

//******************** PointToPointRouter functions ***************************

// These functions simply delegate to PointToPointRouterImpl's functions.
// You probably don't want to change any of this code.

PointToPointRouter::PointToPointRouter(const StreetMap* sm)
{
	m_impl = new PointToPointRouterImpl(sm);
}

PointToPointRouter::~PointToPointRouter()
{
	delete m_impl;
}

DeliveryResult PointToPointRouter::generatePointToPointRoute(
	const GeoCoord& start,
	const GeoCoord& end,
	list<StreetSegment>& route,
	double& totalDistanceTravelled) const
{
	return m_impl->generatePointToPointRoute(start, end, route, totalDistanceTravelled);
}
