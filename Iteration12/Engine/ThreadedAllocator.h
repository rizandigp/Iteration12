#pragma once

#include "Prerequisites.h"


template<typename T> 
class ThreadedAllocator {
public:
  // types
  typedef std::size_t    size_type;        // Holds the size of objects. 
  typedef std::ptrdiff_t difference_type;  // Holds the number of elements between two pointers. 
  typedef T *            pointer;          // A pointer to an object of type T. 
  typedef const T *      const_pointer;    // A pointer to a constant object of type T. 
  typedef T &            reference;        // A reference to an object of type T. 
  typedef const T &      const_reference;  // A reference to a constant object of type T. 
  typedef T              value_type;       // The type of memory allocated by this allocator. 

  // Retrieve the type of an allocator similar to this allocator but for a different value type.   template<typename U> 
  struct rebind {
    // types
    typedef allocator< U > other;
  };

  // construct/copy/destruct
  allocator();
  allocator(const allocator &);
  template<typename U> allocator(const allocator< U > &);
  ~allocator();

  // public member functions
  pointer address(reference) const;
  const_pointer address(const_reference) const;
  pointer allocate(size_type, allocator< void >::const_pointer = 0) ;
  void deallocate(pointer, size_type) ;
  size_type max_size() const;
  void construct(pointer, const T &) ;
  void destroy(pointer) ;
};