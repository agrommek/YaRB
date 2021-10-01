/**
 * @file    yarbt.hpp
 * @brief   Implementation file for the classic ring buffers in a template version
 * @author  Andreas Grommek
 * @version 1.3.0
 * @date    2021-09-29
 * 
 * @section license_yarb_cpp License
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
 * @details This is the default constructor with no arguments. Capacity 
 *          is not given as a parameter to the constructor, but as a
 *          template parameter
 */
template <size_t CAPACITY>
YaRBt<CAPACITY>::YaRBt(void) 
    : readindex{0}, writeindex{0}, arr{0} {
}

/**
 * @brief   The copy constructor.
 * @param   rb
 *          Reference to class instance to copy.
 */
template <size_t CAPACITY>
YaRBt<CAPACITY>::YaRBt(const YaRBt &rb)
    : readindex{rb.readindex}, writeindex{rb.writeindex}, arr{nullptr} {
    memcpy(arr, &(rb.arr), CAPACITY+1);        
}

template <size_t CAPACITY>
size_t YaRBt<CAPACITY>::put(uint8_t new_element) {
    if (this->isFull()) {
        return 0;
    }
    else {
        arr[writeindex] = new_element;
        writeindex = (writeindex + 1) % (CAPACITY+1);
        return 1;
    }
}

template <size_t CAPACITY>
size_t YaRBt<CAPACITY>::put(const uint8_t *new_elements, size_t nbr_elements, bool only_complete) {
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
          arr[writeindex] = *new_elements;
          writeindex = (writeindex + 1) % (CAPACITY+1);
          new_elements++;
    }
    return nbr_elements;
}

template <size_t CAPACITY>
size_t YaRBt<CAPACITY>::peek(uint8_t *peeked_element) const {
    // check for emptyness and validity of output pointer (may be nullptr)
    if (this->isEmpty() || !peeked_element) {
        return 0;
    }
    else {
        *peeked_element = arr[readindex];
        return 1;
    }
}

template <size_t CAPACITY>
size_t YaRBt<CAPACITY>::discard(size_t nbr_elements) {
    if (this->size() > nbr_elements) { // there will be remaining elements in buffer
        // Due to danger of integer overflow, we cannot just do
        // readindex = (readindex + nbr_elements) % (CAPACITY+1):
        // readindex + nbr_elements *might* be larger than SIZE_MAX, which
        // would then overflow, giving wrong results after modulus.
        // --> Do modulus calculation "manually".
        //
        // The difference between current readindex and (CAPACITY+1) is always >= 0 (i.e. at least 1)
        size_t diff_to_max = (CAPACITY+1) - readindex;
        if (nbr_elements > diff_to_max) {
            readindex = nbr_elements - diff_to_max;
        }
        else { // adding nbr_elements to readindex does not wrap
            readindex += nbr_elements;
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
size_t YaRBt<CAPACITY>::get(uint8_t *returned_element) {
    // check for emptyness and validity of output pointer (may  be nullptr)
    if (this->isEmpty() || !returned_element) {
        return 0;
    }
    else {
        *returned_element = arr[readindex];
        readindex = (readindex + 1) % (CAPACITY+1);
        return 1;
    }
}

template <size_t CAPACITY>
size_t YaRBt<CAPACITY>::get(uint8_t *returned_elements, size_t nbr_elements) {
    // check for nullptr
    if (!returned_elements) {
        return 0;
    }
    else {
        // only get at most size() elements from buffer
        if (nbr_elements > this->size()) {
            nbr_elements = this->size();
        }
        for (size_t i=0; i<nbr_elements; i++) {    
            *returned_elements = arr[readindex];
            readindex = (readindex+1) % (CAPACITY+1);
            returned_elements++;
        }
        return nbr_elements;
    }
}
        
template <size_t CAPACITY>
size_t YaRBt<CAPACITY>::size(void) const {
    if (writeindex >= readindex) {
        return writeindex - readindex;
    }
    else {
        return (CAPACITY+1) - (readindex - writeindex);
    }
}

template <size_t CAPACITY>
size_t YaRBt<CAPACITY>::free(void) const {
    return this->capacity() - this->size();
}

template <size_t CAPACITY>
size_t YaRBt<CAPACITY>::capacity(void) const {
    return CAPACITY;
}

template <size_t CAPACITY>
bool YaRBt<CAPACITY>::isFull(void) const {
    return readindex == (writeindex + 1) % (CAPACITY+1);
}

template <size_t CAPACITY>
bool YaRBt<CAPACITY>::isEmpty(void) const {
    return readindex == writeindex;
}

template <size_t CAPACITY>
void YaRBt<CAPACITY>::flush(void) {
    // fast-forward readindex to position of writeindex
    readindex = writeindex;
}

template <size_t CAPACITY>
size_t YaRBt<CAPACITY>::limit(void) {
    return SIZE_MAX - 1;
}
