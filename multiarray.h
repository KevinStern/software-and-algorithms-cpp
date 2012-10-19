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

#ifndef MULTIARRAY_H_
#define MULTIARRAY_H_

#include <cstring>
#include <cstdarg>
#include <stdint.h>
#include <limits>

template<class T, uint32_t D, uint32_t E>
class MultiArrayView;

/**
 * An implementation of a multi-dimensional array with the standard array-access
 * syntax. This implementation stores data within a single array allocated on
 * the heap.
 *
 * @author Kevin L. Stern
 */
template<class T, uint32_t D>
class MultiArray {
public:
	MultiArray(uint32_t extent, ...) {
		va_list ap;
		va_start(ap, extent);
		for (uint32_t j = 0; j < D; ++j) {
			this->extent[j] = extent;
			extent = va_arg(ap, uint32_t);
		}
		va_end(ap);
		this->multiplier[D - 1] = 1;
		size_t total = 1;
		for (uint32_t j = D - 2; j != UINT32_MAX_VALUE; --j) {
			total *= this->extent[j + 1];
			this->multiplier[j] = total;
		}
		total *= this->extent[0];
		array = new T[total];
		memset(array, 0, total * sizeof(T));
	}

	MultiArray(uint32_t extent[D]) {
		memcpy(this->extent, extent, D * sizeof(uint32_t));
		this->multiplier[D - 1] = 1;
		size_t total = 1;
		for (uint32_t j = D - 2; j != UINT32_MAX_VALUE; --j) {
			total *= extent[j + 1];
			this->multiplier[j] = total;
		}
		total *= extent[0];
		array = new T[total];
	}

	MultiArray(const MultiArray<T, D> &other) {
		memcpy(this->extent, other.extent, D * sizeof(uint32_t));
		this->multiplier[D - 1] = 1;
		size_t total = 1;
		for (uint32_t j = D - 2; j != UINT32_MAX_VALUE; --j) {
			total *= extent[j + 1];
			this->multiplier[j] = total;
		}
		total *= extent[0];
		array = new T[total];
		memcpy(array, other.array, total * sizeof(T));
	}

	~MultiArray() {
		delete[] array;
	}

	uint32_t size(uint32_t i) const {
		return extent[i];
	}

	uint32_t size() const {
		return extent[0];
	}

	T* data() {
		return array;
	}

	MultiArrayView<T, D, 2> operator[](uint32_t index) {
		return MultiArrayView<T, D, 2>(*this, index * multiplier[0]);
	}

	const MultiArrayView<T, D, 2> operator[](uint32_t index) const {
		return MultiArrayView<T, D, 2>(*this, index * multiplier[0]);
	}

private:
	static const uint32_t UINT32_MAX_VALUE;

	uint32_t extent[D];
	uint32_t multiplier[D];
	T *array;

	template<class, uint32_t, uint32_t>
	friend class MultiArrayView;
};
template<class T, uint32_t D>
const uint32_t MultiArray<T, D>::UINT32_MAX_VALUE =
		std::numeric_limits<uint32_t>::max();

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
	MultiArray(uint32_t extent) {
		this->extent = extent;
		this->array = new T[extent];
	}

	MultiArray(const MultiArray<T, 1> &other) {
		this->extent = other.extent;
		this->array = new T[extent];
		memcpy(array, other.array, extent * sizeof(T));
	}

	~MultiArray() {
		delete[] array;
	}

	uint32_t size(uint32_t i) const {
		return extent;
	}

	uint32_t size() const {
		return extent;
	}

	T* data() {
		return array;
	}

	T& operator[](uint32_t i) {
		return array[i];
	}

	const T& operator[](uint32_t i) const {
		return array[i];
	}

private:
	uint32_t extent;
	T *array;

	template<class, uint32_t, uint32_t>
	friend class MultiArrayView;
};

template<class T, uint32_t D, uint32_t E>
class MultiArrayView {
public:
	MultiArrayView(const MultiArray<T, D> &array, const uint32_t index) :
			multi(array), index(index) {

	}

	MultiArrayView(const MultiArrayView &view) :
			multi(view.multi), index(view.index) {

	}

	uint32_t size() const {
		return multi.extent[E - 1];
	}

	MultiArrayView<T, D, E + 1> operator[](uint32_t i) {
		return MultiArrayView<T, D, E + 1>(multi,
				index + i * multi.multiplier[E - 1]);
	}

	const MultiArrayView<T, D, E + 1> operator[](uint32_t i) const {
		return MultiArrayView<T, D, E + 1>(multi,
				index + i * multi.multiplier[E - 1]);
	}
private:
	const MultiArray<T, D> &multi;
	const uint32_t index;
};

template<class T, uint32_t D>
class MultiArrayView<T, D, D> {
public:
	MultiArrayView(const MultiArray<T, D> &array, const uint32_t index) :
			multi(array), index(index) {
	}

	MultiArrayView(const MultiArrayView &view) :
			multi(view.multi), index(view.index) {
	}

	uint32_t size() const {
		return multi.extent[D - 1];
	}

	T& operator[](uint32_t i) {
		return multi.array[index + i];
	}

	const T& operator[](uint32_t i) const {
		return multi.array[index + i];
	}

private:
	const MultiArray<T, D> &multi;
	const uint32_t index;
};

#endif /* MULTIARRAY_H_ */
