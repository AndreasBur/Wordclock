// <array> -*- C++ -*-

// Copyright (C) 2007-2015 Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 3, or (at your option)
// any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Under Section 7 of GPL version 3, you are granted additional
// permissions described in the GCC Runtime Library Exception, version
// 3.1, as published by the Free Software Foundation.

// You should have received a copy of the GNU General Public License and
// a copy of the GCC Runtime Library Exception along with this program;
// see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
// <http://www.gnu.org/licenses/>.

/** @file include/array
 *  This is a Standard C++ Library header.
 */

#ifndef _GLIBCXX_ARRAY
#define _GLIBCXX_ARRAY 1

//#pragma GCC system_header

#include "algorithm"
#include "stddef.h"

namespace std
{

  template<typename _Tp, size_t _Nm>
    struct __array_traits
    {
      typedef _Tp _Type[_Nm];

      static constexpr _Tp&
      _S_ref(const _Type& __t, size_t __n) noexcept
      { return const_cast<_Tp&>(__t[__n]); }
    };

 template<typename _Tp>
   struct __array_traits<_Tp, 0>
   {
     struct _Type { };

     static constexpr _Tp&
     _S_ref(const _Type&, size_t) noexcept
     { return *static_cast<_Tp*>(nullptr); }
   };

  /**
   *  @brief A standard container for storing a fixed size sequence of elements.
   *
   *  @ingroup sequences
   *
   *  Meets the requirements of a <a href="tables.html#65">container</a>, a
   *  <a href="tables.html#66">reversible container</a>, and a
   *  <a href="tables.html#67">sequence</a>.
   *
   *  Sets support random access iterators.
   *
   *  @tparam  Tp  Type of element. Required to be a complete type.
   *  @tparam  N  Number of elements.
  */
  template<typename _Tp, size_t _Nm>
    struct array
    {
      typedef _Tp 	    			      value_type;
      typedef value_type*			      pointer;
      typedef const value_type*                       const_pointer;
      typedef value_type&                   	      reference;
      typedef const value_type&             	      const_reference;
      typedef value_type*          		      iterator;
      typedef const value_type*			      const_iterator;
      typedef size_t                    	      size_type;
      typedef ptrdiff_t                   	      difference_type;
      //typedef reverse_iterator<iterator>	      reverse_iterator;
      //typedef reverse_iterator<const_iterator>   const_reverse_iterator;

      // Support for zero-sized arrays mandatory.
      typedef __array_traits<_Tp, _Nm> _AT_Type;
      typename _AT_Type::_Type                         _M_elems;

      // No explicit construct/copy/destroy for aggregate type.

      // DR 776.
      void
      fill(const value_type& __u)
      { fill_n(begin(), size(), __u); }

      //void
      //swap(array& __other)
      //noexcept(noexcept(swap(declval<_Tp&>(), declval<_Tp&>())))
      //{ swap_ranges(begin(), end(), __other.begin()); }

      // Iterators.
      iterator
      begin() noexcept
      { return iterator(data()); }

      const_iterator
      begin() const noexcept
      { return const_iterator(data()); }

      iterator
      end() noexcept
      { return iterator(data() + _Nm); }

      const_iterator
      end() const noexcept
      { return const_iterator(data() + _Nm); }

      //reverse_iterator 
      //rbegin() noexcept
      //{ return reverse_iterator(end()); }
//
      //const_reverse_iterator 
      //rbegin() const noexcept
      //{ return const_reverse_iterator(end()); }
//
      //reverse_iterator 
      //rend() noexcept
      //{ return reverse_iterator(begin()); }
//
      //const_reverse_iterator 
      //rend() const noexcept
      //{ return const_reverse_iterator(begin()); }

      const_iterator
      cbegin() const noexcept
      { return const_iterator(data()); }

      const_iterator
      cend() const noexcept
      { return const_iterator(data() + _Nm); }

      //const_reverse_iterator 
      //crbegin() const noexcept
      //{ return const_reverse_iterator(end()); }
//
      //const_reverse_iterator 
      //crend() const noexcept
      //{ return const_reverse_iterator(begin()); }

      // Capacity.
      constexpr size_type 
      size() const noexcept { return _Nm; }

      constexpr size_type 
      max_size() const noexcept { return _Nm; }

      constexpr bool 
      empty() const noexcept { return size() == 0; }

      // Element access.
      reference
      operator[](size_type __n) noexcept
      { return _AT_Type::_S_ref(_M_elems, __n); }

      constexpr const_reference
      operator[](size_type __n) const noexcept
      { return _AT_Type::_S_ref(_M_elems, __n); }

      reference 
      front() noexcept
      { return *begin(); }

      constexpr const_reference 
      front() const noexcept
      { return _AT_Type::_S_ref(_M_elems, 0); }

      reference 
      back() noexcept
      { return _Nm ? *(end() - 1) : *end(); }

      constexpr const_reference 
      back() const noexcept
      { 
	return _Nm ? _AT_Type::_S_ref(_M_elems, _Nm - 1) 
 	           : _AT_Type::_S_ref(_M_elems, 0);
      }

      pointer
      data() noexcept
      { return __addressof(_AT_Type::_S_ref(_M_elems, 0)); }

      const_pointer
      data() const noexcept
      { return __addressof(_AT_Type::_S_ref(_M_elems, 0)); }
    };

  // Array comparisons.
  template<typename _Tp, size_t _Nm>
    inline bool 
    operator==(const array<_Tp, _Nm>& __one, const array<_Tp, _Nm>& __two)
    { return equal(__one.begin(), __one.end(), __two.begin()); }

  template<typename _Tp, size_t _Nm>
    inline bool
    operator!=(const array<_Tp, _Nm>& __one, const array<_Tp, _Nm>& __two)
    { return !(__one == __two); }

  template<typename _Tp, size_t _Nm>
    inline bool
    operator<(const array<_Tp, _Nm>& __a, const array<_Tp, _Nm>& __b)
    { 
      return lexicographical_compare(__a.begin(), __a.end(),
					  __b.begin(), __b.end()); 
    }

  template<typename _Tp, size_t _Nm>
    inline bool
    operator>(const array<_Tp, _Nm>& __one, const array<_Tp, _Nm>& __two)
    { return __two < __one; }

  template<typename _Tp, size_t _Nm>
    inline bool
    operator<=(const array<_Tp, _Nm>& __one, const array<_Tp, _Nm>& __two)
    { return !(__one > __two); }

  template<typename _Tp, size_t _Nm>
    inline bool
    operator>=(const array<_Tp, _Nm>& __one, const array<_Tp, _Nm>& __two)
    { return !(__one < __two); }

  // Specialized algorithms.
  template<typename _Tp, size_t _Nm>
    inline void
    swap(array<_Tp, _Nm>& __one, array<_Tp, _Nm>& __two)
    noexcept(noexcept(__one.swap(__two)))
    { __one.swap(__two); }

  template<size_t _Int, typename _Tp, size_t _Nm>
    constexpr _Tp&
    get(array<_Tp, _Nm>& __arr) noexcept
    {
      static_assert(_Int < _Nm, "index is out of bounds");
      return __array_traits<_Tp, _Nm>::
	_S_ref(__arr._M_elems, _Int);
    }

  template<size_t _Int, typename _Tp, size_t _Nm>
    constexpr _Tp&&
    get(array<_Tp, _Nm>&& __arr) noexcept
    {
      static_assert(_Int < _Nm, "index is out of bounds");
      return move(get<_Int>(__arr));
    }

  template<size_t _Int, typename _Tp, size_t _Nm>
    constexpr const _Tp&
    get(const array<_Tp, _Nm>& __arr) noexcept
    {
      static_assert(_Int < _Nm, "index is out of bounds");
      return __array_traits<_Tp, _Nm>::
	_S_ref(__arr._M_elems, _Int);
    }


} // namespace std

namespace std
{
  // Tuple interface to class template array.

  /// tuple_size
  template<typename _Tp> 
    class tuple_size;

  /// Partial specialization for array
  //template<typename _Tp, size_t _Nm>
    //struct tuple_size<array<_Tp, _Nm>>
    //: public integral_constant<size_t, _Nm> { };

  /// tuple_element
  template<size_t _Int, typename _Tp>
    class tuple_element;

  /// Partial specialization for array
  template<size_t _Int, typename _Tp, size_t _Nm>
    struct tuple_element<_Int, array<_Tp, _Nm>>
    {
      static_assert(_Int < _Nm, "index is out of bounds");
      typedef _Tp type;
    };

} // namespace std

#ifdef _GLIBCXX_DEBUG
# include <debug/array>
#endif

#ifdef _GLIBCXX_PROFILE
# include <profile/array>
#endif


#endif
