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
	op.optimizeDeliveryOrder(depot, path, old, newDist); //optimize order of the delivery

	if (!(path.empty())) {
		list <StreetSegment> startt;
		DeliveryResult startR;
		double startDist = 0;
		if (!(path.empty())) { //generate a route from the depot to the first delivery
			startR = router.generatePointToPointRoute(depot, path[0].location, startt, startDist);
		}
		if (startR == BAD_COORD) { //ensure there is a route from depot to first delivery
			return BAD_COORD;
		}
		else if (startR == NO_ROUTE) {
			return NO_ROUTE;
		}
		pathways.push_back(startt); //push the route of streetsegments to pathways
		totalDistance.push_back(startDist);//get distance from depot to first delivery
		for (int i = 0; i < path.size() - 1; i++) {
			GeoCoord start; 
			GeoCoord end;
			start = path[i].location;
			end = path[i + 1].location;
			double dist;
			list<StreetSegment> insert;
			DeliveryResult temp; //get a route from current delivery location to the next delivery location
			temp = router.generatePointToPointRoute(start, end, insert, dist);
			if (temp == BAD_COORD) { //ensure all routes are possible
				return BAD_COORD;
			}
			else if (temp == NO_ROUTE) {
				return NO_ROUTE;
			}
			pathways.push_back(insert); //push the route and distances
			totalDistance.push_back(dist);
		}
		list <StreetSegment> endt;
		DeliveryResult endR;
		double endDist = 0;
		if (!(path.empty())) { //do the same for from the last delivery location back to the depot
			endR = router.generatePointToPointRoute(path[path.size() - 1].location, depot, endt, endDist);
		}
		if (endR == BAD_COORD) {
			return BAD_COORD;
		}
		else if (endR == NO_ROUTE) {
			return NO_ROUTE;
		}
		pathways.push_back(endt);
		totalDistance.push_back(endDist);
	}

	for (int i = 0; i < path.size(); i++) { //checking to see if any deliveries at the depot
		if (path[i].location == depot) {
			DeliveryCommand insert;
			insert.initAsDeliverCommand(path[i].item);
			commands.push_back(insert);
		}
	}

	for (int i = 0; i < pathways.size(); i++) {
		bool justDelivered = false;
		bool justTurned = false;
		for (auto it = pathways[i].begin(); it != pathways[i].end(); it++) {
			if (justDelivered && it!=pathways[i].begin()) { 
				it--; //if just delivered, iterate backwards for another command after delivery
			}
			bool turning = false;
			auto check = it;
			check++;
			auto backCheck = it;
			if (it != pathways[i].begin()) {
				if (!(justTurned)) { //check the previous streetsegment to see if current is a new street
					it--;
					backCheck = it;
					it++;
					if (it->name != backCheck->name) { //if different street name, check to see if the angle is turning
						if ((angleBetween2Lines(*it, *backCheck) >= 1) &&
							(angleBetween2Lines(*it, *backCheck) <= 359)) {
							turning = true;
						}
					}
				}
			}

			if (deliveryNum < path.size()) { //if there are still deliveries to make
				if (it->start == path[deliveryNum].location) { //if the location matches the delivery being routed to
					DeliveryCommand insert; //insert the delivery command
					insert.initAsDeliverCommand(path[deliveryNum].item);
					commands.push_back(insert);
					justDelivered = true;
					justTurned = false;
					deliveryNum++;
					continue;
				}
			}

			if (it == pathways[i].begin() || justDelivered || !(turning)) { //undergo proceed command
				double angle = 0;
				string direction = "";
				angle = angleOfLine(*it);
				if (angle >= 0 && angle < 22.5) { //calculate the direction
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
					it++; //check to see if the previous streetsegment has the same name while proceeding
					if (combine->name == it->name) {
						if (!(justDelivered)) { //if so, increase the distance of the command appropriately
							router.generatePointToPointRoute((*it).start, (*it).end, ignore, smallDist);
							commands.back().increaseDistance(smallDist);
							justTurned = false;
							justDelivered = false;
						}
						else { //if we just delivered, do not combine the distances
							router.generatePointToPointRoute((*it).start, (*check).start, ignore, smallDist);
							insert.initAsProceedCommand(direction, it->name, smallDist);
							commands.push_back(insert);
							justTurned = false;
							justDelivered = false;
						}
					}
					else {
						if (check == pathways[i].end()) { //means that iterator has reached the depot
							break;
						} //normal insertion of proceed command
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
					} //normal insertion of proceed command
					router.generatePointToPointRoute((*it).start, (*check).start, ignore, smallDist);
					insert.initAsProceedCommand(direction, it->name, smallDist);
					commands.push_back(insert);
					justTurned = false;
					justDelivered = false;
				}
			}
			else if (angleBetween2Lines(*backCheck, *it) >= 1 &&
				angleBetween2Lines(*backCheck, *it) < 180) {
				DeliveryCommand insert; //inserts a turn command
				insert.initAsTurnCommand("left", it->name);
				commands.push_back(insert);
				it--;
				justTurned = true;
				justDelivered = false;
			}
			else if (angleBetween2Lines(*backCheck, *it) >= 180 &&
				angleBetween2Lines(*backCheck, *it) <= 359) {
				DeliveryCommand insert; //inserts a turn command
				insert.initAsTurnCommand("right", it->name);
				commands.push_back(insert);
				it--;
				justTurned = true;
				justDelivered = false;
			}
		}
	}

	totalDistanceTravelled = 0;
	for (int i = 0; i < totalDistance.size(); i++) { //add together all the distances
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