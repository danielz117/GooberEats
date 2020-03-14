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
	if (!(deliveries.empty())) { //computes old crow distance
		oldCrowDistance += distanceEarthMiles(depot, deliveries[0].location); //include from depot to first delivery
	}
	for (int i = 0; i < deliveries.size() - 1; i++) {
		oldCrowDistance += distanceEarthMiles(deliveries[i].location, deliveries[i + 1].location);
	}
	if (!(deliveries.empty())) {
		oldCrowDistance += distanceEarthMiles(deliveries[deliveries.size() - 1].location, depot); //include from last to depot
	}
	vector<DeliveryRequest> simulations = deliveries;
	double probability = 90; //probability that we will take a worse value
	double newDist = 0;
	vector<DeliveryRequest> saved;
	double oldCompare = oldCrowDistance;
	int nonimprovementIterations = 0;
	int randomNum = 0;
	while (true) { //infinite loop
		randomNum = rand() % 100;
		simulations = deliveries;
		newDist = 0;
		int rand1;
		int rand2;
		rand1 = rand() % simulations.size();
		rand2 = rand() % simulations.size(); //randomly swapping two values within the vector of delivery requests
		DeliveryRequest temp(simulations[rand1].item, simulations[rand1].location);
		simulations[rand1] = simulations[rand2];
		simulations[rand2] = temp;
		if (!(simulations.empty())) { //calculates the new crow distance in this randomly simulated vector of delivery requests
			newDist += distanceEarthMiles(depot, simulations[0].location);
		}
		for (int i = 0; i < simulations.size() - 1; i++) {
			newDist += distanceEarthMiles(simulations[i].location, simulations[i + 1].location);
		}
		if (!(simulations.empty())) {
			newDist += distanceEarthMiles(simulations[simulations.size() - 1].location, depot);
		}
		if (newDist < oldCompare) { //will take the simulated vector if it has a smaller distance
			deliveries = simulations;
			nonimprovementIterations = 0;
			oldCompare = newDist;
		}
		else if (randomNum < probability) { //if the probability variable is greater than random, it will take the worse value
			saved = deliveries; //store the better value under saved
			deliveries = simulations;
			nonimprovementIterations = 0;
			oldCompare = newDist;
		}
		else {
			nonimprovementIterations++; //otherwise, it will increment number of times no improvements  occur
		}
		if (nonimprovementIterations > 25) { //once a certain # of iterations occur without improvement
			if (saved.empty()) { //check to see if we have a saved simulation
				break; //if not, we have found our best value
			}
			else {
				deliveries = saved; //if we do have a better saved value, we will take it
				oldCompare = 0; //compute the nnew oldCompare distance
				if (!(deliveries.empty())) {
					oldCompare += distanceEarthMiles(depot, deliveries[0].location);
				}
				for (int i = 0; i < deliveries.size() - 1; i++) {
					oldCompare += distanceEarthMiles(deliveries[i].location, deliveries[i + 1].location);
				}
				if (!(deliveries.empty())) {
					oldCompare += distanceEarthMiles(deliveries[deliveries.size() - 1].location, depot);
				}
				saved.clear();
				nonimprovementIterations = 0;
			}
		}
		probability--; //decrement probability with every iteration
	}
	newCrowDistance = oldCompare;
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
