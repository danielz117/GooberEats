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
			oldCrowDistance += distanceEarthMiles(deliveries[i].location, deliveries[i+1].location);
		}
	}
	vector<DeliveryRequest> simulations = deliveries;
	double probability = 90;
	int iterations = 1000;
	int newDist = 0;
	while (iterations > 0) {
		simulations = deliveries;
		newDist = 0;
		int rand1; 
		int rand2;
		rand1 = rand() % simulations.size();
		rand2 = rand() % simulations.size();
		/*GeoCoord temp;
		std::string thing;
		simulations[rand1].item = thing;
		simulations[rand1].location = temp;
		simulations[rand1].item = simulations[rand2].item;
		simulations[rand1].location = simulations[rand2].location;
		simulations[rand2].item = thing;
		simulations[rand2].location = temp;*/
		DeliveryRequest temp(simulations[rand1]);
		simulations[rand1] = simulations[rand2];
		simulations[2] = temp;
		for (int i = 0; i < deliveries.size(); i++) {
			if (i == 0) {
				newDist += distanceEarthMiles(deliveries[i].location, depot);
			}
			else if (i == deliveries.size() - 1) {
				newDist += distanceEarthMiles(deliveries[i].location, depot);
			}
			else {
				newDist += distanceEarthMiles(deliveries[i].location, deliveries[i + 1].location);
			}
		}
		cerr << newDist << endl;
		if (newDist < oldCrowDistance) {
			deliveries = simulations;
		}
		else if (rand() % 100 < probability) {
			deliveries = simulations;
		}
		probability--;
		iterations--;
	}





	//newCrowDistance = oldCrowDistance;
	

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
