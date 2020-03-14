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
	struct Compare { //create compare struct for priority queue to analyze which geocoord has less f distance
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
	if (start == end) { //if the starting and ending coords are the same, then we are done
		route.clear();
		totalDistanceTravelled = 0;
		return DELIVERY_SUCCESS;
	}
	else if (streetmap->getSegmentsThatStartWith(start, test) == false ||
		streetmap->getSegmentsThatStartWith(end, test) == false) { //if either start or end coordinates are not in the map, return BAD
		return BAD_COORD;
	}
	else {
		bool routeFound = false;
		GeoCoord next;
		priority_queue<pair<GeoCoord, double>, vector<pair<GeoCoord, double>>, Compare> possible;

		vector<StreetSegment> beginning;
		streetmap->getSegmentsThatStartWith(start, beginning);
		GeoCoord best;

		pair<GeoCoord, double> initial;
		GeoCoord a;
		a.latitude = start.latitude;
		a.longitude = start.longitude;
		a.latitudeText = start.latitudeText;
		a.longitudeText = start.longitudeText;
		initial.first = a;
		initial.second = distanceEarthMiles(start, end);

		//possible.push(initial); //push the starting coordinate and its distance from end into the priority queue
		next = start;

		vector<StreetSegment> seg;
		do {
			seg.clear();
			if (next == end) { //if the next observed coord is the desired ending coordinate then break
				routeFound = true;
				break;
			}

			streetmap->getSegmentsThatStartWith(next, seg);
			double g = 0;
			double h = 0;
			double f = 0;
			for (int i = 0; i < seg.size(); i++) {
				if ((breadcrumbs.find(seg[i].end)) == nullptr) { //if the geocoord has not been visited yet
					g = distanceEarthMiles(next, seg[i].end); //calculate the g cost distance from current coord to next potential coord
					h = distanceEarthMiles(end, seg[i].end); //calculate h cost distance from next potential coord to ending coord
					f = g + h; //A* cost of each potential geocoord
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
			best = (possible.top().first); //top of priority queue will be geocoord with the least distance cost
			breadcrumbs.associate(possible.top().first, possible.top().first); //mark geocoord as visited already
			vector<StreetSegment> prev;
			streetmap->getSegmentsThatStartWith(best, prev);
			for (int i = 0; i < prev.size(); i++) {
				if ((breadcrumbs.find(prev[i].end)) != nullptr) {
					next = prev[i].end; //finds the previous coord of the best potential coord
				}
			}
			possible.pop();
			coords.associate(best, next); //hash map associates best next coord with its previous coord
			next = best;
		} while (!(possible.empty()));
		if (!(routeFound)) { //if priority queue is empty without founding a route, return NO ROUTE
			return NO_ROUTE;
		}
		else {
			while (next != start) { //if a route is found, iterate until the start coord is reached
				StreetSegment* insert;
				StreetSegment seg;
				insert = &seg;
				vector<StreetSegment> pot;
				streetmap->getSegmentsThatStartWith(*(coords.find(next)), pot);
				for (int i = 0; i < pot.size(); i++) { //find the previous coord corresponding with the path
					if (pot[i].end == next) {
						insert = &pot[i];
						break;
					}
				}
				route.push_front(*insert); //push to the front of the list of segments to preserve order
				totalDistanceTravelled += distanceEarthMiles(next, insert->start);
				next = insert->start;
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