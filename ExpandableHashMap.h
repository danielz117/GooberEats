// ExpandableHashMap.h

// Skeleton for the ExpandableHashMap class template.  You must implement the first six
// member functions.
#include <list>

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
	struct Node
	{
	public:
		KeyType key;
		ValueType value;
		Node* next;

		Node(KeyType key, ValueType value)
		{
			this->value = value;
			this->key = key;
		}
	};

	double maxLoad;
	int m_numBuckets;
	int m_numItems;
	Node** buckets;
	Node* rootNode;
};

template<typename KeyType, typename ValueType>
ExpandableHashMap<KeyType, ValueType>::ExpandableHashMap<KeyType, ValueType>(double maximumLoadFactor)
{
	maxLoad = maximumLoadFactor;
	rootNode = nullptr;
	m_numBuckets = 8;
	m_numItems = 0;
	buckets = new Node * [m_numBuckets];
	for (int i = 0; i < m_numBuckets; i++) {
		buckets[i] = nullptr;
	}
	rootNode = nullptr;
}

template<typename KeyType, typename ValueType>
ExpandableHashMap<KeyType, ValueType>::~ExpandableHashMap()
{
	for (int i = 0; i < m_numBuckets; i++) {
		Node* temp = buckets[i];
		while (temp != nullptr) {
			Node* dead = temp;
			temp = temp->next;
			delete dead;
		}
	}

	delete[] buckets;
}

template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::reset()
{
	for (int i = 0; i < m_numBuckets; i++) {
		Node* temp = buckets[i];
		while (temp != nullptr) {
			Node* dead = temp;
			temp = temp->next;
			delete dead;
		}
	}

	delete[] buckets;

	m_numBuckets = 8;
	m_numItems = 0;
	buckets = new Node * [m_numBuckets];
	for (int i = 0; i < m_numBuckets; i++) {
		buckets[i] = nullptr;
	}
	rootNode = nullptr;
}

template<typename KeyType, typename ValueType>
int ExpandableHashMap<KeyType, ValueType>::size() const
{
	return m_numItems;
}

template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::associate(const KeyType& key, const ValueType& value)
{
}

template<typename KeyType, typename ValueType>
const ValueType* ExpandableHashMap<KeyType, ValueType>::find(const KeyType& key) const
{
	unsigned int hash(const KeyType & k);
	unsigned int h = hash(key);
	int bucketNum = h % m_numBuckets;

	if;


	Node* search = buckets[bucketNum];
	while ( {
		if (buckets[bucketNum]->value == key) {
			return buckets[bucketNum];
		}
		else {
			Node* p = buckets[bucketNum + 1];
			while (p != nullptr) {
				if (p->value == key) {
					return p;
				}
				else {
					p = p->next;
				}
			}
			return nullptr;
		}
	}
}
