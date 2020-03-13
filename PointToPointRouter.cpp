#include "provided.h"
#include "ExpandableHashMap.h"
#include <queue>
#include <list>
#include <unordered_set>

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
		bool operator()(pair<GeoCoord, double> const& p1, pair<GeoCoord, double> const& p2)
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
	totalDistanceTravelled = 0;
	ExpandableHashMap<GeoCoord, GeoCoord> coords;
	ExpandableHashMap<GeoCoord, GeoCoord> breadcrumbs;
	vector<StreetSegment> test;
	if (start == end) {
		route.clear();
		totalDistanceTravelled = 0;
		return DELIVERY_SUCCESS;
	}
	else if (streetmap->getSegmentsThatStartWith(start, test) == false ||
		streetmap->getSegmentsThatStartWith(end, test) == false) {
		cerr << "BAD COORD";
		return BAD_COORD;
	}
	else {
		bool routeFound = false;
		GeoCoord next;
		priority_queue<pair<GeoCoord, double>, vector<pair<GeoCoord, double>>, Compare> possible;

		vector<StreetSegment> first;
		streetmap->getSegmentsThatStartWith(start, first);
		GeoCoord best;

		/*for (int i = 0; i < first.size(); i++) {
			g = distanceEarthMiles(start, first[i].end);
			h = distanceEarthMiles(end, first[i].end);
			f = g + h;
			StreetSegment* pfirst = new StreetSegment;
			pfirst = &first[i];
			breadcrumbs.insert({pfirst, true });
			pair<StreetSegment*, double> insert;
			insert.first = &first[i];
			insert.second = f;
			possible.push(insert);
		}
		best = *possible.top().first;
		next = best.end;
		coords.associate(next, best);
		*/

		pair<GeoCoord, double> initial;
		GeoCoord a;
		a.latitude = start.latitude;
		a.longitude = start.longitude;
		a.latitudeText = start.latitudeText;
		a.longitudeText = start.longitudeText;
		initial.first = a;
		initial.second = distanceEarthMiles(start, end);

		possible.push(initial);
		next = start;

		vector<StreetSegment> seg;
		while (!(possible.empty())) {
			seg.clear();
			if (next == end) {
				routeFound = true;
				break;
			}

			streetmap->getSegmentsThatStartWith(next, seg);
			double g = 0;
			double h = 0;
			double f = 0;
			for (int i = 0; i < seg.size(); i++) {
				if ((breadcrumbs.find(seg[i].end)) == nullptr) {
					g = distanceEarthMiles(next, seg[i].end);
					h = distanceEarthMiles(end, seg[i].end);
					f = g + h;
					pair<GeoCoord, double> insert;
					GeoCoord co;
					co.latitude = seg[i].end.latitude;
					co.longitude = seg[i].end.longitude;
					co.latitudeText = seg[i].end.latitudeText;
					co.longitudeText = seg[i].end.longitudeText;
					insert.first = co;
					insert.second = f;
					possible.push(insert);
				}
			}
			//cerr << next.latitudeText << "," << next.longitudeText << endl;
			if (possible.empty()) {
				cerr << "empty";
			}
			best = (possible.top().first);
			breadcrumbs.associate(possible.top().first, possible.top().first);
			vector<StreetSegment> prev;
			streetmap->getSegmentsThatStartWith(best, prev);
			for (int i = 0; i < prev.size(); i++) {
				if ((breadcrumbs.find(prev[i].end)) != nullptr) {
					next = prev[i].end;
				}
			}
			possible.pop();
			coords.associate(best, next);
			next = best;
		}
		if (!(routeFound)) {
			cerr << "NO ROUTE";
			return NO_ROUTE;
		}
		else {
			cerr << "FOUND ROUTE" << endl;
			while (next != start) {
				StreetSegment* insert;
				StreetSegment seg;
				insert = &seg;
				//insert = (coords.find(next));
				vector<StreetSegment> pot;
				streetmap->getSegmentsThatStartWith(*(coords.find(next)), pot);
				for (int i = 0; i < pot.size(); i++) {
					if (pot[i].end == next) {
						insert = &pot[i];
					}
				}
				route.push_front(*insert);
				totalDistanceTravelled += distanceEarthMiles(next, insert->start);
				next = insert->start;
				//cerr << next.latitudeText << next.longitudeText << endl;
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