#ifndef ALCOVE_ID_H_
#define ALCOVE_ID_H_

#include <vector>

// The structure for the coordinates of a given relatively open simplex (alcove)
namespace Gudhi {
struct Alcove_id {
  // typedef typename std::vector<int>::iterator iterator;
  typedef typename std::vector<int>::const_iterator const_iterator;

  Alcove_id(double level)
    : level_(level), dimension_(0) {}

  Alcove_id(double level, int dimension)
    : level_(level), dimension_(dimension) {}
 
  // Alcove_id& operator=(const Alcove_id& other) {
  //   return *this;
  // }
  
  int operator[] (std::size_t i) const {
    return coords_[i];
  }
  
  void push_back(int value, bool fixed = false) {
    coords_.push_back(value);
    fixed_.push_back(fixed);
  }

  void pop_back() {
    coords_.pop_back();
    fixed_.pop_back();
  }
    
  const_iterator begin() const {
    return coords_.begin();
  }

  const_iterator end() const {
    return coords_.end();
  }

  std::size_t size() const {
    return coords_.size();
  }

  std::size_t empty() const {
    return coords_.size() == 0;
  }
  
  void reserve(std::size_t new_cap) {
    coords_.reserve(new_cap);
    fixed_.reserve(new_cap);
  }
  
  void resize(std::size_t new_size) {
    coords_.resize(new_size);
    fixed_.resize(new_size);
  }

  double level() const {
    return level_;
  }
  
  double dimension() const {
    return dimension_;
  }
  
  bool is_fixed(std::size_t i) const {
    return fixed_[i];
  }

  double level_;
  int dimension_;
  std::vector<int> coords_;
  std::vector<bool> fixed_;
};

  bool operator< (const Alcove_id& lhs, const Alcove_id& rhs) {
    return lhs.coords_ < rhs.coords_;
  }

  std::ostream& operator<<(std::ostream & os, const Alcove_id& a_id) {
    os << "[";
    if (a_id.empty())
      return os;
    if (a_id.is_fixed(0))
        os << "\033[1;31m" << a_id[0] << "\033[0m";
      else
        os << a_id[0];
    for (std::size_t i = 1; i < a_id.size(); ++i)
      if (a_id.is_fixed(i))
        os << ", \033[1;31m" << a_id[i] << "\033[0m";
      else
        os << ", " << a_id[i];
    std::cout << "]";
    return os;
  }

  
}

#endif
