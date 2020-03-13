#include "provided.h"
#include <vector>
#include "ExpandableHashMap.h"
#include <list>
#include <vector>
using namespace std;

class DeliveryPlannerImpl
{
public:
	DeliveryPlannerImpl(const StreetMap* sm);
	~DeliveryPlannerImpl();
	DeliveryResult generateDeliveryPlan(
		const GeoCoord& depot,
		const vector<DeliveryRequest>& deliveries,
		vector<DeliveryCommand>& commands,
		double& totalDistanceTravelled) const;
private:
	const StreetMap* streetmap;
};

DeliveryPlannerImpl::DeliveryPlannerImpl(const StreetMap* sm)
{
	streetmap = sm;
}

DeliveryPlannerImpl::~DeliveryPlannerImpl()
{
}

DeliveryResult DeliveryPlannerImpl::generateDeliveryPlan(
	const GeoCoord& depot,
	const vector<DeliveryRequest>& deliveries,
	vector<DeliveryCommand>& commands,
	double& totalDistanceTravelled) const
{
	vector<list<StreetSegment>> pathways;
	vector<double> totalDistance;
	PointToPointRouter router(streetmap);
	DeliveryOptimizer op(streetmap);
	vector<DeliveryRequest> path = deliveries;
	int deliveryNum = 0;
	double old = 0;
	double newDist = 0;
	op.optimizeDeliveryOrder(depot, path, old, newDist);

	if (!(path.empty())) {
		list <StreetSegment> startt;
		DeliveryResult startR;
		double startDist = 0;
		startR = router.generatePointToPointRoute(depot, path[0].location, startt, startDist);
		if (startR == BAD_COORD) {
			cerr << "Bad Coord";
			return BAD_COORD;
		}
		else if (startR == NO_ROUTE) {
			cerr << "No route";
			return NO_ROUTE;
		}
		pathways.push_back(startt);
		totalDistance.push_back(startDist);
		for (int i = 0; i < path.size() - 1; i++) {
			GeoCoord start;
			GeoCoord end;
			start = path[i].location;
			end = path[i + 1].location;
			double dist;
			list<StreetSegment> insert;
			DeliveryResult temp;
			temp = router.generatePointToPointRoute(start, end, insert, dist);
			if (temp == BAD_COORD) {
				cerr << "Bad Coord";
				return BAD_COORD;
			}
			else if (temp == NO_ROUTE) {
				cerr << "No route";
				return NO_ROUTE;
			}
			pathways.push_back(insert);
			totalDistance.push_back(dist);
		}
		list <StreetSegment> endt;
		DeliveryResult endR;
		double endDist = 0;
		endR = router.generatePointToPointRoute(path[path.size() - 1].location, depot, endt, endDist);
		if (endR == BAD_COORD) {
			cerr << "Bad Coord";
			return BAD_COORD;
		}
		else if (endR == NO_ROUTE) {
			cerr << "No route";
			return NO_ROUTE;
		}
		pathways.push_back(endt);
		totalDistance.push_back(endDist);
	}


	for (int i = 0; i < pathways.size(); i++) {
		bool justDelivered = true;
		bool justTurned = false;
		for (auto it = pathways[i].begin(); it != pathways[i].end(); it++) {
			bool turning = false;
			auto check = it;
			check++;
			auto backCheck = it;
			if (it != pathways[i].begin()) {
				if (!(justTurned)) {
					it--;
					backCheck = it;
					it++;
					if (it->name != backCheck->name) {
						if ((angleBetween2Lines(*it, *backCheck) >= 1) &&
							(angleBetween2Lines(*it, *backCheck) <= 359)) {
							turning = true;
						}
					}
				}
			}

			if (deliveryNum < path.size()) {
				if (it->start == path[deliveryNum].location) {
					DeliveryCommand insert;
					insert.initAsDeliverCommand(path[deliveryNum].item);
					commands.push_back(insert);
					justDelivered = true;
					justTurned = false;
					deliveryNum++;
				}
			}

			if (it == pathways[i].begin() || justDelivered || !(turning)) {
				double angle = 0;
				string direction = "";
				angle = angleOfLine(*it);
				if (angle >= 0 && angle < 22.5) {
					direction = "east";
				}
				else if (angle >= 22.5 && angle < 67.5) {
					direction = "northeast";
				}
				else if (angle >= 67.5 && angle < 112.5) {
					direction = "north";
				}
				else if (angle >= 112.5 && angle < 157.5) {
					direction = "northwest";
				}
				else if (angle >= 157.5 && angle < 202.5) {
					direction = "west";
				}
				else if (angle >= 202.5 && angle < 247.5) {
					direction = "southwest";
				}
				else if (angle >= 247.5 && angle < 292.5) {
					direction = "south";
				}
				else if (angle >= 292.5 && angle < 337.5) {
					direction = "southeast";
				}
				else if (angle >= 337.5) {
					direction = "east";
				}
				DeliveryCommand insert;
				double smallDist = 0;
				list<StreetSegment> ignore;
				if (it != pathways[i].begin()) {
					it--;
					auto combine = it;
					it++;
					if (combine->name == it->name) {
						if (!(justDelivered)) {
							router.generatePointToPointRoute((*combine).start, (*it).end, ignore, smallDist);
							commands.pop_back();
							insert.initAsProceedCommand(direction, it->name, smallDist);
							commands.push_back(insert);
							justTurned = false;
							justDelivered = false;
						}
						else {
							router.generatePointToPointRoute((*it).start, (*check).start, ignore, smallDist);
							insert.initAsProceedCommand(direction, it->name, smallDist);
							commands.push_back(insert);
							justTurned = false;
							justDelivered = false;
						}
					}
					else {
						if (check == pathways[i].end()) {
							break;
						}
						router.generatePointToPointRoute((*it).start, (*check).start, ignore, smallDist);
						insert.initAsProceedCommand(direction, it->name, smallDist);
						commands.push_back(insert);
						justTurned = false;
						justDelivered = false;
					}
				}
				else {
					if (check == pathways[i].end()) {
						break;
					}
					router.generatePointToPointRoute((*it).start, (*check).start, ignore, smallDist);
					insert.initAsProceedCommand(direction, it->name, smallDist);
					commands.push_back(insert);
					justTurned = false;
					justDelivered = false;
				}
			}
			else if (angleBetween2Lines(*it, *backCheck) >= 1 &&
				angleBetween2Lines(*it, *backCheck) < 180) {
				DeliveryCommand insert;
				insert.initAsTurnCommand("right", it->name);
				commands.push_back(insert);
				it--;
				justTurned = true;
				justDelivered = false;
			}
			else if (angleBetween2Lines(*it, *backCheck) >= 180 &&
				angleBetween2Lines(*it, *backCheck) <= 359) {
				DeliveryCommand insert;
				insert.initAsTurnCommand("left", it->name);
				commands.push_back(insert);
				it--;
				justTurned = true;
				justDelivered = false;
			}


		}

	}

	totalDistanceTravelled = 0;
	for (int i = 0; i < totalDistance.size(); i++) {
		totalDistanceTravelled += totalDistance[i];
	}
	return DELIVERY_SUCCESS;

}

//******************** DeliveryPlanner functions ******************************

// These functions simply delegate to DeliveryPlannerImpl's functions.
// You probably don't want to change any of this code.

DeliveryPlanner::DeliveryPlanner(const StreetMap* sm)
{
	m_impl = new DeliveryPlannerImpl(sm);
}

DeliveryPlanner::~DeliveryPlanner()
{
	delete m_impl;
}

DeliveryResult DeliveryPlanner::generateDeliveryPlan(
	const GeoCoord& depot,
	const vector<DeliveryRequest>& deliveries,
	vector<DeliveryCommand>& commands,
	double& totalDistanceTravelled) const
{
	return m_impl->generateDeliveryPlan(depot, deliveries, commands, totalDistanceTravelled);
}