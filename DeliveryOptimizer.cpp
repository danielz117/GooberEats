#include <vector>
#include "provided.h"
#include <random>
using namespace std;

class DeliveryOptimizerImpl
{
public:
	DeliveryOptimizerImpl(const StreetMap* sm);
	~DeliveryOptimizerImpl();
	void optimizeDeliveryOrder(
		const GeoCoord& depot,
		vector<DeliveryRequest>& deliveries,
		double& oldCrowDistance,
		double& newCrowDistance) const;
private:
	const StreetMap* streetmap;
};

DeliveryOptimizerImpl::DeliveryOptimizerImpl(const StreetMap* sm)
{
	streetmap = sm;
}

DeliveryOptimizerImpl::~DeliveryOptimizerImpl()
{
}

void DeliveryOptimizerImpl::optimizeDeliveryOrder(
	const GeoCoord& depot,
	vector<DeliveryRequest>& deliveries,
	double& oldCrowDistance,
	double& newCrowDistance) const
{
	oldCrowDistance = 0;
	newCrowDistance = 0;
	for (int i = 0; i < deliveries.size(); i++) {
		if (i == 0) {
			oldCrowDistance += distanceEarthMiles(deliveries[i].location, depot);
		}
		else if (i == deliveries.size() - 1) {
			oldCrowDistance += distanceEarthMiles(deliveries[i].location, depot);
		}
		else {
			oldCrowDistance += distanceEarthMiles(deliveries[i].location, deliveries[i + 1].location);
		}
	}
	vector<DeliveryRequest> simulations = deliveries;
	double probability = 90;
	double newDist = 0;
	vector<DeliveryRequest> saved;
	double oldCompare = oldCrowDistance;
	int nonimprovementIterations = 0;
	int randomNum = 0;
	while (true) {
		randomNum = rand() % 100;
		simulations = deliveries;
		newDist = 0;
		int rand1;
		int rand2;
		rand1 = rand() % simulations.size();
		rand2 = rand() % simulations.size();
		DeliveryRequest temp(simulations[rand1].item, simulations[rand1].location);
		simulations[rand1] = simulations[rand2];
		simulations[rand2] = temp;
		for (int i = 0; i < simulations.size(); i++) {
			if (i == 0) {
				newDist += distanceEarthMiles(simulations[i].location, depot);
			}
			else if (i == simulations.size() - 1) {
				newDist += distanceEarthMiles(simulations[i].location, depot);
			}
			else {
				newDist += distanceEarthMiles(simulations[i].location, simulations[i + 1].location);
			}
		}
		if (newDist < oldCompare) {
			deliveries = simulations;
			nonimprovementIterations = 0;
			oldCompare = newDist;
		}
		else if (randomNum < probability) {
			saved = deliveries;
			deliveries = simulations;
			nonimprovementIterations = 0;
			oldCompare = newDist;
		}
		else {
			nonimprovementIterations++;
		}
		if (nonimprovementIterations > 25) {
			if (saved.empty()) {
				break;
			}
			else {
				deliveries = saved;
				oldCompare = 0;
				for (int i = 0; i < saved.size(); i++) {
					if (i == 0) {
						oldCompare += distanceEarthMiles(saved[i].location, depot);
					}
					else if (i == saved.size() - 1) {
						oldCompare += distanceEarthMiles(saved[i].location, depot);
					}
					else {
						oldCompare += distanceEarthMiles(saved[i].location, saved[i + 1].location);
					}
				}
				saved.clear();
			}
		}
		probability--;
	}
}

//******************** DeliveryOptimizer functions ****************************

// These functions simply delegate to DeliveryOptimizerImpl's functions.
// You probably don't want to change any of this code.

DeliveryOptimizer::DeliveryOptimizer(const StreetMap* sm)
{
	m_impl = new DeliveryOptimizerImpl(sm);
}

DeliveryOptimizer::~DeliveryOptimizer()
{
	delete m_impl;
}

void DeliveryOptimizer::optimizeDeliveryOrder(
	const GeoCoord& depot,
	vector<DeliveryRequest>& deliveries,
	double& oldCrowDistance,
	double& newCrowDistance) const
{
	return m_impl->optimizeDeliveryOrder(depot, deliveries, oldCrowDistance, newCrowDistance);
}
