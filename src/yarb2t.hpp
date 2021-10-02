/**
 * @file    yarb2t.hpp
 * @brief   Implementation file for the YaRB ring buffer in a template version
 * @author  Andreas Grommek
 * @version 1.5.0
 * @date    2021-10-02
 * 
 * @section license_yarb2t_hpp License
 * 
 * The MIT Licence (MIT)
 * 
 * Copyright (c) 2021 Andreas Grommek
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
 
#include <string.h>  // memcpy(), for copy constructor

/**
 * @brief   The constructor.
 * @details There is only a parameterless constructor. Size is given as
 *          template parameter.
 */
template <size_t CAPACITY>
YaRB2t<CAPACITY>::YaRB2t(void) 
    : readindex{0}, writeindex{0}, arr{0} {}

/**
 * @brief   The copy constructor.
 * @param   rb
 *          Reference to class instance to copy.
 */
template <size_t CAPACITY>
YaRB2t<CAPACITY>::YaRB2t(const YaRB2t<CAPACITY> &rb)
    : readindex{rb.readindex}, writeindex{rb.writeindex}, arr{0} {
    memcpy(arr, &(rb.arr), (CAPACITY * sizeof(uint8_t)) );        
}

/**
 * @brief   The assignment operator
 * @param   rb
 *          Reference to class instance to assign from
 * @note    This works because both operands are guaranteed to be of 
 *          same capacity when using a template.
 */
template <size_t CAPACITY>
YaRB2t<CAPACITY>& YaRB2t<CAPACITY>::operator=(const YaRB2t<CAPACITY> &rb) {
    // protect against self-assignment
    if (this == &rb) return *this;
    // copy indices verbatim
    readindex = rb.readindex;
    writeindex = rb.writeindex;
    const size_t r = modcap(readindex);
    const size_t w = modcap(writeindex);
    // Copy content depending of relative position of indices.
    // case 1: writeindex has not yet wrapped
    // --> copy size() bytes of data, starting from readindex
    if (r <= w) { 
        memcpy(arr+r, rb.arr+r, rb.size());
    }
    // case 2: writeindex has already wrapped around, readindex not yet
    // --> copy from readindex to end of array
    // --> copy from start of array to writeindex-1
    else {
        memcpy(arr+r, rb.arr+r, CAPACITY-r);
        memcpy(arr, rb.arr, w);
    }
    return *this;        
}

template <size_t CAPACITY>
size_t YaRB2t<CAPACITY>::put(uint8_t new_element) {
    if (this->isFull()) {
        return 0;
    }
    else {
        arr[modcap(writeindex)] = new_element;
        writeindex = modcap2(writeindex+1);
        return 1;
    }
}

template <size_t CAPACITY>
size_t YaRB2t<CAPACITY>::put(const uint8_t *new_elements, size_t nbr_elements, bool only_complete) {
    // check validity of input pointer (may be nullptr)
    if (!new_elements ) {
        return 0;
    }
    // only add at most free() elements to ring buffer
    if (nbr_elements > this->free()) {
        if (only_complete) return 0;
        nbr_elements = this->free();
    }
    for (size_t i=0; i<nbr_elements; i++) {
          // re-implementation is about 3-4 times faster than calling
          // single-element put()
          arr[modcap(writeindex)] = *new_elements;
          writeindex = modcap2(writeindex+1);
          new_elements++;
    }
    return nbr_elements;

}

template <size_t CAPACITY>
size_t YaRB2t<CAPACITY>::peek(uint8_t *peeked_element) const {
    // check for emptyness and validity of output pointer (may be nullptr)
    if (this->isEmpty() || !peeked_element) {
        return 0;
    }
    else {
        *peeked_element = arr[modcap(readindex)];
        return 1;
    }
}

template <size_t CAPACITY>
size_t YaRB2t<CAPACITY>::discard(size_t nbr_elements) {
    if (this->size() > nbr_elements) { // there will be remaining elements in buffer
        // Due to danger of integer overflow, we cannot just do
        // readindex = modcap2(readindex + nbr_elements):
        // readindex + nbr_elements *might* be larger than SIZE_MAX, which
        // would then overflow, giving wrong results after modcap2.
        // --> Do modulus calculation "manually".
        //
        // The difference between current readindex is always > 0 (i.e. at least 1)
        size_t diff_to_max = 2*CAPACITY - readindex;
        if (diff_to_max <= nbr_elements) { // readindex+nbr_elements > 2*cap --> danger of overflow
            readindex = nbr_elements - diff_to_max;
        }
        else { // adding nbr_elements to readindex stays in correct range
            // no need for modcap2(), adding nbr_elements keeps readindex
            // below 2*cap
            readindex = readindex + nbr_elements;
        }
        // return nbr_elements, no matter if we had to wrap around or not
        return nbr_elements;
    }
    else { // discard *all* elements --> flush()
        // we can only discard at many elements as are in the buffer
        // --> return size()
        size_t retval = this->size();
        this->flush();
        return retval;
    }
}

template <size_t CAPACITY>
size_t YaRB2t<CAPACITY>::get(uint8_t *returned_element) {
    // check for emptyness and validity of output pointer (may  be nullptr)
    if (this->isEmpty() || !returned_element) {
        return 0;
    }
    else {
        *returned_element = arr[modcap(readindex)];
        readindex = modcap2(readindex+1);
        return 1;
    }
}

template <size_t CAPACITY>
size_t YaRB2t<CAPACITY>::get(uint8_t *returned_elements, size_t nbr_elements) {
    // check nullptr
    if (!returned_elements) {
        return 0;
    }
    else {
        // only get at most size() elements from buffer
        if (nbr_elements > this->size()) {
            nbr_elements = this->size();
        }
        for (size_t i=0; i<nbr_elements; i++) {
            // re-implementation is about 2 times faster than calling
            // single-element get()        
            *returned_elements = arr[modcap(readindex)];
            readindex = modcap2(readindex+1);
            returned_elements++;
        }
        return nbr_elements;
    }
}

template <size_t CAPACITY>
size_t YaRB2t<CAPACITY>::size(void) const {
    return modcap2(writeindex-readindex);
}

template <size_t CAPACITY>
size_t YaRB2t<CAPACITY>::free(void) const {
    return CAPACITY - this->size();
}

template <size_t CAPACITY>
size_t YaRB2t<CAPACITY>::capacity(void) const {
    return CAPACITY;
}

template <size_t CAPACITY>
bool YaRB2t<CAPACITY>::isFull(void) const {
    return this->size() == CAPACITY;
}

template <size_t CAPACITY>
bool YaRB2t<CAPACITY>::isEmpty(void) const {
    return readindex == writeindex;
}

template <size_t CAPACITY>
void YaRB2t<CAPACITY>::flush(void) {
    // fast-forward readindex to position of writeindex
    readindex = writeindex;
}

template <size_t CAPACITY>
size_t YaRB2t<CAPACITY>::limit(void) {
    return SIZE_MAX / 2;
}

template <size_t CAPACITY>
inline size_t YaRB2t<CAPACITY>::modcap(size_t val) const {
    return (val % CAPACITY);
}

template <size_t CAPACITY>
inline size_t YaRB2t<CAPACITY>::modcap2(size_t val) const {
    return (val % (2 * CAPACITY));
}
