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
	void printAll() const;
private:
	struct Node
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

	double maxLoad;
	int m_numBuckets;
	int m_numItems;
	std::vector<std::list<Node*>> stuff;


	int getBucketNum(const KeyType& key, int num) const;
	
};

template<typename KeyType, typename ValueType>
ExpandableHashMap<KeyType, ValueType>::ExpandableHashMap(double maximumLoadFactor)
{
	maxLoad = maximumLoadFactor;
	m_numBuckets = 8;
	m_numItems = 0;
	for (int i = 0; i < m_numBuckets; i++) {
		std::list<Node*> bucket;
		stuff.push_back(bucket);
	}
}

template<typename KeyType, typename ValueType>
ExpandableHashMap<KeyType, ValueType>::~ExpandableHashMap()
{
	for (int i = 0; i < m_numBuckets; i++) {
		for (auto it = stuff[i].begin(); it != stuff[i].end(); it++) {
			delete* it;
		}
		stuff[i].clear();
	}

	stuff.clear();
}

template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::reset()
{
	for (int i = 0; i < m_numBuckets; i++) {
		for (auto it = stuff[i].begin(); it != stuff[i].end(); it++) {
			delete* it;
		}
		stuff[i].clear();
	}

	stuff.clear();

	m_numBuckets = 8;
	m_numItems = 0;
	for (int i = 0; i < m_numBuckets; i++) {
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
	if (find(key) != nullptr) {
		*(find(key)) = value;
	}
	else {
		if (((double)m_numItems + 1) / m_numBuckets > maxLoad) {
			std::vector<std::list<Node*>> bigger;
			for (int i = 0; i < m_numBuckets * 2; i++) {
				std::list<Node*> bucket;
				bigger.push_back(bucket);
			}

			for (int i = 0; i < m_numBuckets; i++) {
				if (!(stuff[i].empty())) {
					for (auto it = stuff[i].begin(); it != stuff[i].end(); it++) {
						int hash = getBucketNum((*it)->key, m_numBuckets * 2);
						std::list<Node*> newList;
						newList.push_back((*it));
						bigger[hash].splice(bigger[hash].begin(), newList);
					}
				}
			}
			bigger;
			for (int i = 0; i < m_numBuckets; i++) {
				stuff[i].clear();
			}
			stuff.clear();
			m_numBuckets *= 2;
			stuff = bigger;
			Node* insert = new Node(key, value);
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
	int bucketNum = getBucketNum(key, m_numBuckets);
	for (auto it = stuff[bucketNum].begin(); it != stuff[bucketNum].end(); it++) {
		if ((*it)->key == key) {
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
	unsigned int h = hasher(key);
	return (h % num);
}

template<typename KeyType, typename ValueType>
inline void ExpandableHashMap<KeyType, ValueType>::printAll() const
{
	for (int i = 0; i < stuff.size(); i++) {
		std::string path;
		for (auto it = stuff[i].begin(); it != stuff[i].end(); it++) {
			path += (*it)->value[0]->name + '\n';
			path += (*it)->key.latitudeText + "," + (*it)->key.longitudeText + "->" + '\n';
			for (int i = 0; i < (*it)->value.size(); i++) {
				path += (*it)->value[i]->start.latitudeText + ' ';
				path += (*it)->value[i]->start.longitudeText + ' ';
				path += (*it)->value[i]->end.latitudeText + ' ';
				path += (*it)->value[i]->end.longitudeText + ' ' + '\n';
			}
			path += '\n';
		}
		std::cerr << path;
	}
}