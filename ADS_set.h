#ifndef ADS_SET_H
#define ADS_SET_H

// ADS_set.h Double Hashing
// 
// VU Algorithmen und Datenstrukturen - SS 2017 Universitaet Wien
// Container - Projekt
// https://cewebs.cs.univie.ac.at/ADS/ss17/
//
// Beispielimplementierung mit "Double Hashing - Linear Probing" 
// Erstellt in der Lehrveranstaltung vom 4.4.2017, Konstruktoren und 
// Zuweisungsoperatoren wurden vervollständigt.
//
// Nicht optimiert und eventuell fehlerhaft (Fehler bitte melden)

#include <functional>
#include <algorithm>
#include <iostream>
#include <stdexcept>

template <typename Key, size_t N = 7>
class ADS_set {
public:
  class Iterator;
  using value_type = Key;
  using key_type = Key;
  using reference = key_type&;
  using const_reference = const key_type&;
  using size_type = size_t;
  using difference_type = std::ptrdiff_t;
  using iterator = Iterator;
  using const_iterator = Iterator;
  using key_equal = std::equal_to<key_type>; // Hashing
  using hasher = std::hash<key_type>;        // Hashing
private:
  enum class Mode {free, freeagain, inuse, end};
  struct element {
    key_type key;
    element *next;
   // Mode mode {Mode::free};
  };
  element ** table {nullptr};
  size_type sz {0}, max_sz {0};
  float max_lf {0.7};

  size_type hash_idx(const key_type& k) const { return hasher{}(k) % max_sz; }
  //size_type next_idx(const key_type&, size_type pos) const { return (pos+1) % max_sz; }
  
  element *insert_unchecked(const key_type& k) {
    size_type idx {hash_idx(k)};
    table[idx]=new element{k, table[idx]};
    sz++;
    return table[idx];
  }

  element *find_pos(const key_type& k) const {
    
    size_type idx {hash_idx(k)};
    element *current_ptr;
    current_ptr = table[idx];
    
    while(current_ptr){
      if(key_equal{} (current_ptr->key,k))
        return current_ptr;
      else {
        current_ptr = current_ptr->next;
      }
    }
    return nullptr;
  }

  void rehash(size_type n) {

      element **old_table {table};
      size_type old_max_sz=max_sz;
      max_sz = n;
      table = new element*[max_sz+1];
      for(size_type i{0}; i <= max_sz; ++i){
        table[i]=nullptr;
      }
      sz=0;
      for(size_type i{0}; i < old_max_sz; ++i){
        element *current_ptr;
        current_ptr = old_table[i];
        while(current_ptr != nullptr) {
          insert_unchecked(current_ptr->key);
          current_ptr = current_ptr->next;
        }
      }
      
      for(size_type i{0}; i < old_max_sz; ++i){
      	while(old_table[i] != nullptr){
      		element*current_ptr = old_table[i];
      		old_table[i] = old_table[i]->next;
      		delete current_ptr;
       	}
      }
    delete[] old_table;
  }

  void reserve(size_type n) {
    if (n > max_sz * max_lf) {
      size_type new_max_sz {max_sz};
      while (n > new_max_sz * max_lf) new_max_sz = new_max_sz * 2 + 1;
      rehash(new_max_sz);
    }
  }

public:
  ADS_set() {     
    max_sz=N;
    table = new element*[max_sz+1];
      for(size_type i=0;i <= max_sz;++i){
        table[i]=nullptr;
      }
   }
  ADS_set(std::initializer_list<key_type> ilist): ADS_set{} {
    insert(ilist);
  }
  template<typename InputIt> ADS_set(InputIt first, InputIt last): ADS_set{} {
    insert(first, last);
  }
  ADS_set(const ADS_set& other) {
    rehash(other.max_sz); 
    for (const auto& k: other) 
   		insert_unchecked(k);
  }
  ~ADS_set() {
  	for(size_type i{0}; i < max_sz; ++i){
      	while(table[i] != nullptr){
      		element*current_ptr = table[i];
      		table[i] = table[i]->next;
      		delete current_ptr;
       	}
  	}
  delete[] table;
}

  ADS_set& operator=(const ADS_set& other) {
    if (this == &other) return *this;
    clear();
    reserve(other.sz); 
    for (const auto& k: other) insert_unchecked(k);
    return *this;
  }
  ADS_set& operator=(std::initializer_list<key_type> ilist) {
    clear();
    insert(ilist);
    return *this;
  }

  size_type size() const { return sz; }
  bool empty() const { return !sz; }

  size_type count(const key_type& key) const {
    return !!find_pos(key);
  }
  iterator find(const key_type& key) const {
  	if(empty()) return end();
    if (element *pos {find_pos(key)}) return const_iterator{pos, this};
    return end();
  }
  void clear() {
	for(size_type i{0}; i < max_sz; ++i){
      	while(table[i] != nullptr){
      		element*current_ptr = table[i];
      		table[i] = table[i]->next;
      		delete current_ptr;
       	}
  	}
  	sz=0;
  	/*
  	for(size_type i{0}; i <= max_sz; ++i){
        table[i]=nullptr;
      }
      sz=0;
      */
  }
  void swap(ADS_set& other) {
    using std::swap;
    swap(sz, other.sz);
    swap(max_sz, other.max_sz);
    swap(max_lf, other.max_lf);
    swap(table, other.table);
  }

  void insert(std::initializer_list<key_type> ilist) {
    reserve(sz+ilist.size());
    for (const auto& k: ilist) 
      if (!find_pos(k)) insert_unchecked(k);
  }

  std::pair<iterator,bool> insert(const key_type& key) {
    if (element *pos {find_pos(key)})
      return std::make_pair(const_iterator{pos,this}, false);
    reserve(sz+1);
    return std::make_pair(const_iterator{insert_unchecked(key),this}, true);
  }
  
  template<typename InputIt> void insert(InputIt first, InputIt last) {
    for (auto it = first; it != last; ++it) {
      if (!find_pos(*it)) {
        reserve(sz+1);
        insert_unchecked(*it);
      }
    }
  }

/*size_type eraseFirst(const key_type& key){
	size_type idx {hash_idx(key)};
	element *help;
	help=table[idx];
	table[idx]=table[idx]->next;
	sz--;
	delete[] help;
	return 1;
}*/

  size_type erase(const key_type& key) {

  	if (element *pos {find_pos(key)}){
  		size_type idx {hash_idx(key)};
  		
  		if(key_equal{}(table[idx]->key,key)) {
  			element * current_ptr= table[idx];
			table[idx]=table[idx]->next;
			delete current_ptr;
			sz--;
  			return 1;
  		}
  		
  		element *current_ptr;
  		element *previous_ptr;
  		current_ptr=table[idx];
  		previous_ptr=table[idx];

  		while(current_ptr!=nullptr){
	  		if (key_equal{}(current_ptr->key,key)){
	  			previous_ptr->next=current_ptr->next;
	  			delete current_ptr;
	  			sz--;
	  			return 1;
	  		}
	  		
	  	previous_ptr=current_ptr;
	  	current_ptr=current_ptr->next;
  		}

  	}
  		return 0;
  }

  const_iterator begin() const { 
  	if(sz>0){
	for(size_type i=0; i < max_sz; i++){
  		if(table[i]!=nullptr) {
  			return const_iterator{table[i], this};
  		}
  	}
  }
  	return const_iterator{nullptr, this};
  }
  

  const_iterator end() const { 
  	 /*if(sz>0) {
  		for(size_type i=max_sz; i>=0; i--){
	  		if(table[i]!=nullptr) {
	  			if(table[i]->next==nullptr)
	  				return const_iterator{table[i], this};
	  			else{
					element * help;
	  				help=table[i];
	  				while(help->next != nullptr){
	  					help=help->next;
	  				}
	  				return const_iterator{help, this};
	  			}
	  		}
  		}
  	}
  	else {*/
  		return const_iterator{nullptr, this};
  	//}
  }

  void dump(std::ostream& o = std::cerr) const {
    o << "\nNumber of elements: " << sz << ", Size of table: " << max_sz << '\n';

    if(max_sz == 0)
      o << '\n' << "Table is empty." << '\n';

    else{
      for (size_type i{0}; i < max_sz; ++i) {
        o <<"[" << i <<"]";
        element *help;
        help=table[i];
        while(help){
          o << " > " << help->key;
          help=help->next;
        }
        //delete help;
        o << '\n';
      }
    } 
  }

  friend bool operator==(const ADS_set& lhs, const ADS_set& rhs) {
    if (lhs.sz != rhs.sz) return false;
    for (const auto& k: lhs) 
      if (!rhs.find_pos(k)) return false;
    return true;
  }
  friend bool operator!=(const ADS_set& lhs, const ADS_set& rhs) { return !(lhs==rhs); }
};

template <typename Key, size_t N>
class ADS_set<Key,N>::Iterator {
  element * pos;
  const ADS_set<Key,N> * set;
  void skip_free() { while (pos == nullptr) ++pos; }
public:
  using value_type = Key;
  using difference_type = std::ptrdiff_t;
  using reference = const value_type&;
  using pointer = const value_type*;
  using iterator_category = std::forward_iterator_tag;

  //explicit Iterator(element * pos = nullptr, const ADS_set<Key,N> * set ): pos{pos}, set{set} {}
  //explicit Iterator(element * pos = nullptr): pos{pos} { if (pos) skip_free(); }
  

  Iterator(element * pos, const ADS_set<Key,N> * set): pos{pos}, set{set} {}



  reference operator*() const { return pos->key; }
  pointer operator->() const { return &pos->key; }
  
  Iterator& operator++() {
   	if(pos->next != nullptr) {
		pos=pos->next;
  	/*
  	size_type idx;
  	idx = set->hash_idx(pos->key);
  	pos=set->table[++idx];
  	*/
     	return *this;	
  	}
  	else{
  		size_type idx;
  		idx = set->hash_idx(pos->key) + 1;
  		while(set->table[idx] == nullptr && idx < set->max_sz){
  			idx++;
  		}
  		//if(set->table[idx]!=nullptr){
  			pos=set->table[idx];
  			return *this;
  		//}
  	}
  	//return *this;
  }

  Iterator operator++(int) {
    auto rc = *this;
    ++*this;
    return rc;
  }
  
  friend bool operator==(const Iterator& lhs, const Iterator& rhs) { return lhs.pos == rhs.pos; }
  friend bool operator!=(const Iterator& lhs, const Iterator& rhs) { return lhs.pos != rhs.pos; }
};

template <typename Key, size_t N> void swap(ADS_set<Key,N>& lhs, ADS_set<Key,N>& rhs) { lhs.swap(rhs); }

#endif // ADS_SET_H