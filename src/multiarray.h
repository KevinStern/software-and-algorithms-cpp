/* Copyright (c) 2012 Kevin L. Stern
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#pragma once

#include <cstring>
#include <cstdarg>
#include <cstdint>
#include <iostream>
#include <limits>

template<class T, uint32_t D, uint32_t E>
class MultiArrayView;

// An implementation of a multi-dimensional array with the standard array-access syntax. This
// implementation stores data within a single array allocated on the heap.
//
// @author Kevin L. Stern
template<class T, uint32_t D>
class MultiArray {
private:
  /**
   * Helper class which uses compile time template recursion to extrapolate dimensions and extract
   * elements from a MultiArray initializer list.
   */
  template <typename ElemType, uint32_t Dim>
  struct InitializerHelper {
    typedef std::initializer_list<typename InitializerHelper<ElemType, Dim - 1>::Type> Type;

    static void populate_extents(const Type& initializer, uint32_t* extents) {
      extents[D - Dim] = initializer.size();
      InitializerHelper<ElemType, Dim - 1>::populate_extents(*initializer.begin(), extents);
    }

    static ElemType* populate_elements(const Type& initializer, ElemType* array) {
      for (typename Type::iterator iter = initializer.begin(); iter != initializer.end(); ++iter) {
        array = InitializerHelper<ElemType, Dim - 1>::populate_elements(*iter, array);
      }
      return array;
    }
  };

  template <typename ElemType>
  struct InitializerHelper<ElemType, 1> {
    typedef std::initializer_list<ElemType> Type;

    static void populate_extents(const Type& initializer, uint32_t* extents) {
      extents[D - 1] = initializer.size();
    }

    static ElemType* populate_elements(const Type& initializer, ElemType* array) {
      for (typename Type::iterator iter = initializer.begin(); iter != initializer.end(); ++iter) {
        *(array++) = *iter;
      }
      return array;
    }
  };

public:
  // Varargs version of constructor; construct a MultiArray with the specified extents.
  // Note that D extents must be given.
  MultiArray(uint32_t extent, ...) {
    va_list ap;
    va_start(ap, extent);
    for (uint32_t i = 0; i < D; ++i) {
      extent_[i] = extent;
      extent = va_arg(ap, uint32_t);
    }
    va_end(ap);
    multiplier_[D - 1] = 1;
    size_t total = 1;
    for (uint32_t i = D - 2; ; --i) {
      total *= extent_[i + 1];
      multiplier_[i] = total;
      if (i == 0) {
        break;
      }
    }
    total *= extent_[0];
    array_ = new T[total];
    memset(array_, 0, total * sizeof(T));
  }

  // Construct a MultiArray with the specified extents.
  MultiArray(uint32_t extent[D]) {
    memcpy(extent_, extent, D * sizeof(uint32_t));
    multiplier_[D - 1] = 1;
    size_t total = 1;
    for (uint32_t j = D - 2; j != UINT32_MAX_VALUE; --j) {
      total *= extent_[j + 1];
      multiplier_[j] = total;
    }
    total *= extent_[0];
    array_ = new T[total];
  }

  // Initializer list version of the constructor. Construct a MultiArray with the data specified in
  // initializer.
  //
  // For example, to initialize a two dimensional double array with the data:
  //     [1.1, 2.2]
  //     [3.3, 4.4]
  // The following syntax may be used:
  //     MultiArray<double, 2> array({{1.1, 2.2}, {3.3, 4.4}});
  MultiArray(const typename InitializerHelper<T, D>::Type& initializer) {
    InitializerHelper<T, D>::populate_extents(initializer, extent_);
    multiplier_[D - 1] = 1;
    size_t total = 1;
    for (uint32_t j = D - 2; j != UINT32_MAX_VALUE; --j) {
      total *= extent_[j + 1];
      multiplier_[j] = total;
    }
    total *= extent_[0];
    array_ = new T[total];
    InitializerHelper<T, D>::populate_elements(initializer, array_);
  }

  // Construct a MultiArray by copying from other.
  MultiArray(const MultiArray<T, D>& other) {
    memcpy(extent_, other.extent_, D * sizeof(uint32_t));
    multiplier_[D - 1] = 1;
    size_t total = 1;
    for (uint32_t j = D - 2; j != UINT32_MAX_VALUE; --j) {
      total *= extent_[j + 1];
      multiplier_[j] = total;
    }
    total *= extent_[0];
    array_ = new T[total];
    memcpy(array_, other.array_, total * sizeof(T));
  }

  // Construct a MultiArray by moving from other.
  MultiArray(MultiArray<T, D>&& other) {
    memcpy(extent_, other.extent_, D * sizeof(uint32_t));
    multiplier_[D - 1] = 1;
    size_t total = 1;
    for (uint32_t j = D - 2; j != UINT32_MAX_VALUE; --j) {
      total *= extent_[j + 1];
      multiplier_[j] = total;
    }
    total *= extent_[0];
    array_ = other.array_;
    other.array_ = nullptr;
  }

  ~MultiArray() {
    delete[] array_;
  }

  // Get the size of dimension i.
  uint32_t size(uint32_t i) const {
    if (i >= D) {
      throw std::out_of_range("i >= D");
    }
    return extent_[i];
  }

  // Get the size of dimension 0.
  uint32_t size() const {
    return extent_[0];
  }

  T* data() {
    return array_;
  }

  MultiArrayView<T, D, 2> operator[](uint32_t index) {
    if (index >= extent_[0]) {
      throw std::out_of_range("i >= extent");
    }
    // Dimension 1 is taken care of by this class, so we return a view at dimension 2.
    return MultiArrayView<T, D, 2>(*this, index * multiplier_[0]);
  }

  const MultiArrayView<T, D, 2> operator[](uint32_t index) const {
    if (index >= extent_[0]) {
      throw std::out_of_range("i >= extent");
    }
    // Dimension 1 is taken care of by this class, so we return a view at dimension 2.
    return MultiArrayView<T, D, 2>(*this, index * multiplier_[0]);
  }

private:
  static constexpr uint32_t UINT32_MAX_VALUE = std::numeric_limits<uint32_t>::max();

  uint32_t extent_[D];
  uint32_t multiplier_[D];
  T* array_;

  template<class, uint32_t, uint32_t>
  friend class MultiArrayView;

  // For pretty printing.
  friend std::ostream& operator<<(std::ostream& out, const MultiArray<T, D>& array) {
    out << "[";
    for (int i = 0; i < array.size(); ++i) {
      out << array[i];
      if (i < array.size() - 1) {
        out << ",";
      }
    }
    out << "]";
    return out;
  }
};

/**
 * An implementation of a uni-dimensional array with the standard array-access
 * syntax. This implementation stores data within a single array allocated on
 * the heap.
 *
 * @author Kevin L. Stern
 */
template<class T>
class MultiArray<T, 1> {
public:
  // Construct a MultiArray with the specified extent.
  MultiArray(uint32_t extent) {
    extent_ = extent;
    array_ = new T[extent_];
    memset(array_, 0, extent_ * sizeof(T));
  }

  // Initializer list version of the constructor. Construct a MultiArray with the data specified in
  // initializer.
  //
  // For example, to initialize a one dimensional double array with the data:
  //     [1.1, 2.2]
  // The following syntax may be used:
  //     MultiArray<double, 1> array({1.1, 2.2});
  MultiArray(std::initializer_list<T> initializer) {
    extent_ = initializer.size();
    array_ = new T[extent_];
    int i = 0;
    for (typename std::initializer_list<T>::iterator iter = initializer.begin();
        iter != initializer.end(); ++iter) {
      array_[i++] = *iter;
    }
  }

  // Construct a MultiArray by copying from other.
  MultiArray(const MultiArray<T, 1>& other) {
    extent_ = other.extent_;
    array_ = new T[extent_];
    memcpy(array_, other.array_, extent_ * sizeof(T));
  }

  // Construct a MultiArray by moving from other.
  MultiArray(MultiArray<T, 1>&& other) {
    extent_ = other.extent_;
    array_ = other.array_;
    other.array_ = nullptr;
  }

  ~MultiArray() {
    delete[] array_;
  }

  // Get the size of dimension i.
  uint32_t size(uint32_t i) const {
    if (i != 0) {
      throw std::out_of_range("i != 0");
    }
    return extent_;
  }

  // Get the size of dimension 0.
  uint32_t size() const {
    return extent_;
  }

  T* data() {
    return array_;
  }

  T& operator[](uint32_t i) {
    if (i >= extent_) {
      throw std::out_of_range("i >= extent");
    }
    return array_[i];
  }

  const T& operator[](uint32_t i) const {
    if (i >= extent_) {
      throw std::out_of_range("i >= extent");
    }
    return array_[i];
  }

private:
  uint32_t extent_;
  T* array_;

  template<class, uint32_t, uint32_t>
  friend class MultiArrayView;

  // For pretty printing.
  friend std::ostream& operator<<(std::ostream& out, const MultiArray<T, 1>& array) {
    out << "[";
    for (int i = 0; i < array.size(); ++i) {
      out << array[i];
      if (i < array.size() - 1) {
        out << ",";
      }
    }
    out << "]";
    return out;
  }
};

// *************************************************************************************************
// The MultiArrayView classes are dimensional views into an instance of MultiArray. MultiArray's
// operator[] returns a MultiArrayView, as does MultiArrayView's operator[], except, of course, for
// the high dimension instance's operator[], which returns the element at the appropriate index.
//
// The template arguments are the data type T, the total number of dimensions D and the current
// dimension E. Dimensions begin with 1 and continue through D.
template<class T, uint32_t D, uint32_t E>
class MultiArrayView {
public:
  MultiArrayView(const MultiArray<T, D>& array, const uint32_t index)
      : multi_(array), index_(index) {}

  MultiArrayView(const MultiArrayView& view) : multi_(view.multi_), index_(view.index_) {}

  uint32_t size() const {
    return multi_.extent_[E - 1];
  }

  MultiArrayView<T, D, E + 1> operator[](uint32_t i) {
    if (i >= multi_.extent_[E - 1]) {
      throw std::out_of_range("i >= extent");
    }
    return MultiArrayView<T, D, E + 1>(multi_, index_ + i * multi_.multiplier_[E - 1]);
  }

  const MultiArrayView<T, D, E + 1> operator[](uint32_t i) const {
    if (i >= multi_.extent_[E - 1]) {
      throw std::out_of_range("i >= extent");
    }
    return MultiArrayView<T, D, E + 1>(multi_, index_ + i * multi_.multiplier_[E - 1]);
  }
private:
  const MultiArray<T, D>& multi_;
  const uint32_t index_;

  // For pretty printing.
  friend std::ostream& operator<<(std::ostream& out, const MultiArrayView<T, D, E>& view) {
    out << "[";
    for (int i = 0; i < view.size(); ++i) {
      out << view[i];
      if (i < view.size() - 1) {
        out << ",";
      }
    }
    out << "]";
    return out;
  }
};

template<class T, uint32_t D>
class MultiArrayView<T, D, D> {
public:
  MultiArrayView(const MultiArray<T, D>& array, const uint32_t index)
      : multi_(array), index_(index) {}

  MultiArrayView(const MultiArrayView& view)
      : multi_(view.multi_), index_(view.index_) {}

  uint32_t size() const {
    return multi_.extent_[D - 1];
  }

  T& operator[](uint32_t i) {
    if (i >= multi_.extent_[D - 1]) {
      throw std::out_of_range("i >= extent");
    }
    return multi_.array_[index_ + i];
  }

  const T& operator[](uint32_t i) const {
    if (i >= multi_.extent_[D - 1]) {
      throw std::out_of_range("i >= extent");
    }
    return multi_.array_[index_ + i];
  }

private:
  const MultiArray<T, D>& multi_;
  const uint32_t index_;

  // For pretty printing.
  friend std::ostream& operator<<(std::ostream& out, const MultiArrayView<T, D, D>& view) {
    out << "[";
    for (int i = 0; i < view.size(); ++i) {
      out << view[i];
      if (i < view.size() - 1) {
        out << ",";
      }
    }
    out << "]";
    return out;
  }
};
