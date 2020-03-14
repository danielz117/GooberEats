#include "provided.h"
#include <string>
#include <vector>
#include <functional>
#include "ExpandableHashMap.h"
#include <iostream>
#include <fstream>
#include <ctype.h>
#include <set>
using namespace std;

unsigned int hasher(const GeoCoord& g)
{
	return std::hash<string>()(g.latitudeText + g.longitudeText);
}

class StreetMapImpl
{
public:
	StreetMapImpl();
	~StreetMapImpl();
	bool load(string mapFile);
	bool getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const;
private:
	ExpandableHashMap<GeoCoord, vector<StreetSegment*>> container;
	vector<StreetSegment*> tracker; //tracks the dynamically allocated street segments
};

StreetMapImpl::StreetMapImpl()
{
}

StreetMapImpl::~StreetMapImpl()
{
	for (int i = 0; i < tracker.size(); ) { //deletes dynamically allocated street segments
		delete tracker[tracker.size() - 1];
		tracker.pop_back();
	}
}

bool StreetMapImpl::load(string mapFile)
{
	ifstream infile(mapFile); //if file is not found, return false
	if (!infile)
	{
		cerr << "Error: Cannot open map data!" << endl;
		return false;
	}

	string line;
	string streetName = "";
	int numSegments = -1;
	int counter = 0;
	while (getline(infile, line)) { //iterates through each line until the end of the file
		if (numSegments == -1) { //checks to see if the number of segments has been read yet
			if (streetName == "") { //gets the name of the street
				streetName = line;
			}
			else {
				int decimalPlace = 1;
				numSegments = 0;
				while (line.length() > 0) { //calculates the number of street segments for one street name
					numSegments += (line[line.length() - 1] - '0') * decimalPlace;
					decimalPlace *= 10;
					line = line.substr(0, line.length() - 1);
				}
			}
		}
		else if (counter < numSegments) {
			counter++;
			int pos = 0;
			string startLat;
			string startLong;
			string endLat;
			string endLong;
			while (line[pos] != ' ') { //stops at the first space
				startLat += line[pos]; //gets the starting latitude
				pos++;
			}
			pos++;
			while (line[pos] != ' ') { //gets the starting longitude 
				startLong += line[pos];
				pos++;
			}
			pos++;
			while (line[pos] != ' ') { //gets the ending latitude
				endLat += line[pos];
				pos++;
			}
			pos++;
			while (pos < line.length()) { //gets the ending longitude 
				endLong += line[pos];
				pos++;
			}
			GeoCoord start(startLat, startLong);
			GeoCoord end(endLat, endLong);
			StreetSegment* seg1 = new StreetSegment(start, end, streetName);
			StreetSegment* seg2 = new StreetSegment(end, start, streetName);
			tracker.push_back(seg1); //tracks dynamically allocated streetsegments
			tracker.push_back(seg2);
			if (container.find(start) != nullptr) { //if the geocoord has already been associated
				container.find(start)->push_back(seg1); //push into the vector containing all streetsegments starting with that geocoord
			}
			else {
				vector<StreetSegment*> vec1; //if the geocoord has not yet been associated
				vec1.push_back(seg1); //create a new vector of streetsegments and associate in hash map
				container.associate(start, vec1);
			}

			if (container.find(end) != nullptr) { //do the same for the reverse
				container.find(end)->push_back(seg2);
			}
			else {
				vector<StreetSegment*> vec2;
				vec2.push_back(seg2);
				container.associate(end, vec2);
			}

			if (counter >= numSegments) { //once all streetsegments for a street name are read, reset variables
				numSegments = -1;
				counter = 0;
				streetName = "";
			}
		}
	}
	return true;
}

bool StreetMapImpl::getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const
{
	if (container.find(gc) != nullptr) { //if a key is associated with a vector of streetsegments
		for (int i = 0; i < container.find(gc)->size(); i++) { //iterate through the vector of streetsegments
			StreetSegment insert = *((*container.find(gc))[i]);
			segs.push_back(insert); //push into parameter
		}
		return true;
	}
	else {
		return false;
	}
}

//******************** StreetMap functions ************************************

// These functions simply delegate to StreetMapImpl's functions.
// You probably don't want to change any of this code.

StreetMap::StreetMap()
{
	m_impl = new StreetMapImpl;
}

StreetMap::~StreetMap()
{
	delete m_impl;
}

bool StreetMap::load(string mapFile)
{
	return m_impl->load(mapFile);
}

bool StreetMap::getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const
{
	return m_impl->getSegmentsThatStartWith(gc, segs);
}