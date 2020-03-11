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
};

StreetMapImpl::StreetMapImpl()
{
}

StreetMapImpl::~StreetMapImpl()
{
}

bool StreetMapImpl::load(string mapFile)
{
	ifstream infile(mapFile);
	if (!infile)
	{
		cerr << "Error: Cannot open map data!" << endl;
		return false;
	}

	string line;
	string streetName = "";
	int numSegments = -1;
	int counter = 0;
	bool first = false;
	while (getline(infile, line)) {
		if (numSegments == -1) {
			if (streetName == "") {
				streetName = line;
			}
			else {
				int decimalPlace = 1;
				numSegments = 0;
				while (line.length() > 0) {
					numSegments += (line[line.length() - 1] - '0') * decimalPlace;
					decimalPlace *= 10;
					line = line.substr(0, line.length() - 1);
				}
				first = true;
			}
		}
		else if (counter < numSegments) {
			counter++;
			int pos = 0;
			string startLat;
			string startLong;
			string endLat;
			string endLong;
			while (line[pos] != ' ') {
				startLat += line[pos];
				pos++;
			}
			pos++;
			while (line[pos] != ' ') {
				startLong += line[pos];
				pos++;
			}
			pos++;
			while (line[pos] != ' ') {
				endLat += line[pos];
				pos++;
			}
			pos++;
			while (pos < line.length()) {
				endLong += line[pos];
				pos++;
			}
			GeoCoord start(startLat, startLong);
			GeoCoord end(endLat, endLong);
			StreetSegment* seg1 = new StreetSegment(start, end, streetName);
			StreetSegment* seg2 = new StreetSegment(end, start, streetName);
			if (container.find(start) != nullptr) {
				container.find(start)->push_back(seg1);
			}
			else {
				vector<StreetSegment*> vec1;
				vec1.push_back(seg1);
				container.associate(start, vec1);
			}

			if (container.find(end) != nullptr) {
				container.find(end)->push_back(seg2);
			}
			else {
				vector<StreetSegment*> vec2;
				vec2.push_back(seg2);
				container.associate(end, vec2);
			}

			if (counter >= numSegments) {
				numSegments = -1;
				counter = 0;
				streetName = "";
			}
		}
	}
	//container.printAll();
	return true;
}

bool StreetMapImpl::getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const
{
	if (container.find(gc) != nullptr) {
		for (int i = 0; i < container.find(gc)->size(); i++) {
			StreetSegment insert = *((*container.find(gc))[i]);
			segs.push_back(insert);
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
