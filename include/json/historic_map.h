#ifndef HISTORIC_MAP_H
#define HISTORIC_MAP_H

#include <list>
#include <map>
#include <iterator>
#include <utility>

template <class Key, 
          class Value,
          class Compare = std::less<Key>
         >
class historic_map
{
public:
  typedef Key                                                   key_type;
  typedef Value                                                 mapped_type;
  typedef std::pair<const Key, Value>                           value_type;
  typedef value_type&                                           reference;
  typedef value_type const&                                     const_reference;
  typedef size_t                                                size_type;
  typedef Compare                                               value_compare;

private:
  typedef std::list<value_type>                                 storage_type;
  typedef typename storage_type::iterator                       storage_iterator;
  typedef typename storage_type::const_iterator                 const_storage_iterator;

  typedef std::map<key_type, storage_iterator>                  index_type;
  typedef typename index_type::iterator                         index_iterator;
  typedef typename index_type::const_iterator                   const_index_iterator;

public:
  typedef storage_iterator                                      iterator;
  typedef const_storage_iterator                                const_iterator;
  typedef std::reverse_iterator<iterator>                       reverse_iterator;
  typedef std::reverse_iterator<const_iterator>                 const_reverse_iterator;

  historic_map(){}

  historic_map(const historic_map& hm)
      : m_store(hm.m_store)
  {
      for (auto it = m_store.begin(), ite = m_store.end(); it != ite; ++it)
      {
          m_index[it->first] = it;
      }
  }

  historic_map(historic_map&& hm)
      : m_store(std::move(hm.m_store))
      , m_index(std::move(hm.m_index))
  {}
  ~historic_map(){}

  historic_map& operator=(const historic_map& hm)
  {
      if(&hm != this)
      {
          historic_map tmp(hm);
          std::swap(tmp, *this);
      }
      return (*this);
  }

  historic_map& operator=(historic_map&& hm)
  {
      historic_map tmp(std::move(hm));
      std::swap(tmp, *this);
      return (*this);
  }


  const Value& at(const Key& key)
  {
      auto it = find(key);
      if (it == end())
          throw std::out_of_range("key not presented");
      return it->second;
  }

  Value& operator[](const Key& key)
  {
      auto it = find(key);
      if (it == end())
          it = insert(std::make_pair(key, mapped_type())).first;
      return it->second;
  }


  iterator begin()  { return m_store.begin(); }
  const_iterator begin() const { return m_store.begin(); }
  iterator end() { return m_store.end(); }
  const_iterator end() const { return m_store.end(); }

  reverse_iterator rbegin() { return m_store.rbegin(); }
  const_reverse_iterator rbegin() const { return m_store.rbegin(); }
  reverse_iterator rend() { return m_store.rend(); }
  const_reverse_iterator rend() const { return m_store.rend(); }

  const_iterator cbegin() const { return m_store.cbegin(); }
  const_iterator cend() const { return m_store.cend(); }
  const_reverse_iterator crbegin() const { return m_store.crbegin(); }
  const_reverse_iterator crend() const { return m_store.crend(); }


  bool empty() const 
  {
      return m_store.empty();
  }
  
  size_type size() const 
  {
      return m_store.size();
  }


  void clear()
  {
      m_store.clear();
      m_index.clear();
  }

  
  std::pair<iterator, bool> insert(const value_type& value)
  {
      auto it = m_index.find(value.first);
      if (it != m_index.end())
          return std::make_pair(it->second, false);
      return std::make_pair(m_index[value.first] = m_store.insert(m_store.end(), value), true);
  }

  iterator insert(const_iterator hint, const value_type& value)
  {
      return insert(value).first;
  }

  template <class... Args> 
  std::pair<iterator, bool> emplace(Args&&... args) 
  {
    return insert(value_type(std::forward<Args>(args)...));
  }

  size_type erase(const Key& key)
  {
      auto it = m_index.find(key);
      if (it == m_index.end())
          return 0;
      if (it->second != m_store.end())
          m_store.erase(it->second);
      m_index.erase(it);
      return 1;
  }

  iterator erase(const_iterator it)
  {
      m_index.erase(m_index.find(it->first));
      return m_store.erase(it);
  }

  
  void swap(historic_map& other)
  {
      std::swap(m_store, other.m_store);
      std::swap(m_index, other.m_index);
  }

  
  size_type count(const Key& key) const
  {
      return find(key) == end() ? 0 : 1;
  }

  
  const_iterator find(const Key& key) const
  {
      auto it = m_index.find(key);
      if (it != m_index.end())
          return it->second;
      return end();
  }
  
  iterator find(const Key& key)
  {
      auto it = m_index.find(key);
      if (it != m_index.end())
          return it->second;
      return end();
  }

  
  std::pair<const_iterator, const_iterator> equal_range(const Key& key) const
  {
      auto it = find(key);
      if (it == end())
          return std::make_pair(it, it);
      auto ite = it;
      return std::make_pair(it, ++ite);
  }
  
  std::pair<iterator, iterator> equal_range(const Key& key)
  {
      auto it = find(key);
      if (it == end())
          return std::make_pair(it, it);
      auto ite = it;
      return std::make_pair(it, ++ite);
  }
  
  const_iterator lower_bound(const Key& key) const
  {
      return find(key);
  }
  
  iterator lower_bound(const Key& key)
  {
      return find(key);
  }
  
  const_iterator upper_bound(const Key& key) const
  {
      auto it = find(key);
      if (it == end())
          return it;
      return ++it;
  }
  
  iterator upper_bound(const Key& key)
  {
      auto it = find(key);
      if (it == end())
          return it;
      return ++it;
  }

  const_iterator min_key() const
  {
      return empty() ? end() : m_index.begin()->second;
  }

  iterator min_key()
  {
      return empty() ? end() : m_index.begin()->second;
  }


  const_iterator max_key() const
  {
      return empty() ? end() : m_index.rbegin()->second;
  }

  iterator max_key()
  {
      return empty() ? end() : m_index.rbegin()->second;
  }


  storage_type  m_store;
  index_type    m_index;
  value_compare m_comp;
};

template <class Key,
    class Value,
    class Compare = std::less<Key>
>
bool operator<(const historic_map<Key, Value, Compare> & lhs, const historic_map<Key, Value, Compare> & rhs)
{
  auto lb = lhs.m_index.begin();
  auto le = lhs.m_index.end();
  auto rb = rhs.m_index.begin();
  auto re = rhs.m_index.end();
  for (; lb != le && rb != re; ++lb, ++rb) {
    if (*lb->second < *rb->second)
      return true;
    if (*rb->second < *lb->second)
      return false;
  }
  return (lb == le) && (rb != re);
}

template <class Key,
    class Value,
    class Compare = std::less<Key>
>
bool operator==(const historic_map<Key, Value, Compare> & lhs, const historic_map<Key, Value, Compare> & rhs)
{
  if (lhs.size() != rhs.size())
      return false;
  auto lb = lhs.m_index.begin();
  auto le = lhs.m_index.end();
  auto rb = rhs.m_index.begin();
  auto re = rhs.m_index.end();
  for (; lb != le && rb != re; ++lb, ++rb) {
    if (*lb->second != *rb->second)
        return false;
  }
  return true;
}
template <class Key,
    class Value,
    class Compare = std::less<Key>
>
bool operator!=(const historic_map<Key, Value, Compare>& lhs, const historic_map<Key, Value, Compare>& rhs)
{
    return !(lhs == rhs);
}
template <class Key,
    class Value,
    class Compare = std::less<Key>
>
bool operator>(const historic_map<Key, Value, Compare>& lhs, const historic_map<Key, Value, Compare>& rhs)
{
    return rhs < lhs;
}
template <class Key,
    class Value,
    class Compare = std::less<Key>
>
bool operator<=(const historic_map<Key, Value, Compare>& lhs, const historic_map<Key, Value, Compare>& rhs)
{
    return !(lhs > rhs);
}
template <class Key,
    class Value,
    class Compare = std::less<Key>
>
bool operator>=(const historic_map<Key, Value, Compare>& lhs, const historic_map<Key, Value, Compare>& rhs)
{
    return !(lhs<rhs);
}

template <class Key,
    class Value,
    class Compare = std::less<Key>
>
auto begin(const historic_map<Key, Value, Compare>& hm)
{
    return hm.begin();
}
template <class Key,
    class Value,
    class Compare = std::less<Key>
>
auto cbegin(const historic_map<Key, Value, Compare>& hm)
{
    return hm.cbegin();
}
template <class Key,
    class Value,
    class Compare = std::less<Key>
>
auto begin(historic_map<Key, Value, Compare>& hm)
{
    return hm.begin();
}
template <class Key,
    class Value,
    class Compare = std::less<Key>
>
auto rbegin(const historic_map<Key, Value, Compare>& hm)
{
    return hm.rbegin();
}
template <class Key,
    class Value,
    class Compare = std::less<Key>
>
auto crbegin(const historic_map<Key, Value, Compare>& hm)
{
    return hm.crbegin();
}
template <class Key,
    class Value,
    class Compare = std::less<Key>
>
auto rbegin(historic_map<Key, Value, Compare>& hm)
{
    return hm.rbegin();
}
template <class Key,
    class Value,
    class Compare = std::less<Key>
>
auto end(const historic_map<Key, Value, Compare>& hm)
{
    return hm.end();
}
template <class Key,
    class Value,
    class Compare = std::less<Key>
>
auto cend(const historic_map<Key, Value, Compare>& hm)
{
    return hm.cend();
}
template <class Key,
    class Value,
    class Compare = std::less<Key>
>
auto end(historic_map<Key, Value, Compare>& hm)
{
    return hm.end();
}
template <class Key,
    class Value,
    class Compare = std::less<Key>
>
auto rend(const historic_map<Key, Value, Compare>& hm)
{
    return hm.rend();
}
template <class Key,
    class Value,
    class Compare = std::less<Key>
>
auto crend(const historic_map<Key, Value, Compare>& hm)
{
    return hm.crend();
}
template <class Key,
    class Value,
    class Compare = std::less<Key>
>
auto rend(historic_map<Key, Value, Compare>& hm)
{
    return hm.rend();
}

#endif