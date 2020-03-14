// ExpandableHashMap.h

// Skeleton for the ExpandableHashMap class template.  You must implement the first six
// member functions.
#include <list>
#include <vector>
#include <iterator>

template<typename KeyType, typename ValueType>
class ExpandableHashMap
{
public:
	ExpandableHashMap(double maximumLoadFactor = 0.5);
	~ExpandableHashMap();
	void reset();
	int size() const;
	void associate(const KeyType& key, const ValueType& value);

	// for a map that can't be modified, return a pointer to const ValueType
	const ValueType* find(const KeyType& key) const;

	// for a modifiable map, return a pointer to modifiable ValueType
	ValueType* find(const KeyType& key)
	{
		return const_cast<ValueType*>(const_cast<const ExpandableHashMap*>(this)->find(key));
	}

	// C++11 syntax for preventing copying and assignment
	ExpandableHashMap(const ExpandableHashMap&) = delete;
	ExpandableHashMap& operator=(const ExpandableHashMap&) = delete;
private:
	struct Node //creating private struct to represent the the key and associated value
	{
	public:
		KeyType key;
		ValueType value;

		Node(KeyType key, ValueType value)
		{
			this->value = value;
			this->key = key;
		}
	};

	double maxLoad; //declaring private data members
	int m_numBuckets;
	int m_numItems;
	std::vector<std::list<Node*>> stuff; //container to the hash map

	int getBucketNum(const KeyType& key, int num) const; //helper function to call hasher function
};

template<typename KeyType, typename ValueType>
ExpandableHashMap<KeyType, ValueType>::ExpandableHashMap(double maximumLoadFactor)
{
	maxLoad = maximumLoadFactor;
	m_numBuckets = 8;
	m_numItems = 0;
	for (int i = 0; i < m_numBuckets; i++) { //inserting 8 empty buckets (lists) into the container
		std::list<Node*> bucket;
		stuff.push_back(bucket);
	}
}

template<typename KeyType, typename ValueType>
ExpandableHashMap<KeyType, ValueType>::~ExpandableHashMap()
{
	for (int i = 0; i < m_numBuckets; i++) {
		for (auto it = stuff[i].begin(); it != stuff[i].end(); it++) { //destructing all dynamically allocated nodes
			delete* it; 
		}
		stuff[i].clear(); //clearing the container
	}

	stuff.clear();
}

template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::reset()
{
	for (int i = 0; i < m_numBuckets; i++) {
		for (auto it = stuff[i].begin(); it != stuff[i].end(); it++) {
			delete* it; //destructing all dynamically allocated nodes
		}
		stuff[i].clear();
	}

	stuff.clear();

	m_numBuckets = 8;
	m_numItems = 0;
	for (int i = 0; i < m_numBuckets; i++) { //putting in 8 new, empty buckets into the new, empty hash map
		std::list<Node*> bucket;
		stuff.push_back(bucket);
	}
}

template<typename KeyType, typename ValueType>
int ExpandableHashMap<KeyType, ValueType>::size() const
{
	return m_numItems;
}

template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::associate(const KeyType& key, const ValueType& value)
{
	if (find(key) != nullptr) { //if the key is already part of the hash map, change into new value
		*(find(key)) = value;
	}
	else {
		if (((double)m_numItems + 1) / m_numBuckets > maxLoad) { //if adding one new item exceeds the max load then resize
			std::vector<std::list<Node*>> bigger;
			for (int i = 0; i < m_numBuckets * 2; i++) { //create a new hash map that contains double the number of buckets
				std::list<Node*> bucket;
				bigger.push_back(bucket);
			}

			for (int i = 0; i < m_numBuckets; i++) { //splice in all the nodes from the original hash map
				if (!(stuff[i].empty())) {
					for (auto it = stuff[i].begin(); it != stuff[i].end(); it++) {
						int hash = getBucketNum((*it)->key, m_numBuckets * 2);
						std::list<Node*> newList;
						newList.push_back((*it));
						bigger[hash].splice(bigger[hash].begin(), newList); //insert the nodes into the new hash bucket
					}
				}
			}
			for (int i = 0; i < m_numBuckets; i++) { //clear everything from the original hash map
				stuff[i].clear();
			}
			stuff.clear();
			m_numBuckets *= 2;
			stuff = bigger; //set the original hahs map to the resized hash map
			Node* insert = new Node(key, value); //insert the new association
			int bucketNum = getBucketNum(key, m_numBuckets);
			stuff[bucketNum].push_back(insert);
		}
		else {
			Node* insert = new Node(key, value);
			int bucketNum = getBucketNum(key, m_numBuckets);
			stuff[bucketNum].push_back(insert);
		}
		m_numItems++;
	}
}

template<typename KeyType, typename ValueType>
const ValueType* ExpandableHashMap<KeyType, ValueType>::find(const KeyType& key) const
{
	int bucketNum = getBucketNum(key, m_numBuckets); //find the appropriate bucket for the key
	for (auto it = stuff[bucketNum].begin(); it != stuff[bucketNum].end(); it++) { //iterate through the elements in the bucket
		if ((*it)->key == key) { //is that key is within the list, return the value of the key
			ValueType* val = &((*it)->value);
			return val;
		}
	}
	return nullptr;
}

template<typename KeyType, typename ValueType>
int ExpandableHashMap<KeyType, ValueType>::getBucketNum(const KeyType& key, int num) const
{
	unsigned int hasher(const KeyType & k); // prototype
	unsigned int h = hasher(key); //get the bucket number given the size of the hash map
	return (h % num);
}

