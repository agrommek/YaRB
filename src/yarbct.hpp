/**
 * @file    yarbct.hpp
 * @brief   Implementation file for a ring buffer template implementation with counter.
 * @author  Andreas Grommek
 * @version 1.5.0
 * @date    2021-10-02
 * 
 * @section license_yarbct_hpp License
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
 * @details This is the default constructor with one optional argument,
 *          the delimiting byte. 
 *          Capacity is not given as a parameter to the constructor, but
 *          as a template parameter
 */
template <size_t CAPACITY>
YaRBct<CAPACITY>::YaRBct(uint8_t delimiter) 
    : delim{delimiter}, readindex{0}, writeindex{0}, arr{0}, ct{0} {
}

/**
 * @brief   The copy constructor.
 * @param   rb
 *          Reference to class instance to copy.
 */
template <size_t CAPACITY>
YaRBct<CAPACITY>::YaRBct(const YaRBct<CAPACITY> &rb)
    : delim{rb.delim}, readindex{rb.readindex}, writeindex{rb.writeindex}, arr{0}, ct{rb.ct} {
    memcpy(arr, &(rb.arr), CAPACITY+1);        
}

// modified
template <size_t CAPACITY>
size_t YaRBct<CAPACITY>::put(uint8_t new_element) {
    if (this->isFull()) {
        return 0;
    }
    else {
        if (new_element == delim) ct++;
        arr[writeindex] = new_element;
        writeindex = (writeindex + 1) % (CAPACITY+1);
        return 1;
    }
}

// modified
template <size_t CAPACITY>
size_t YaRBct<CAPACITY>::put(const uint8_t *new_elements, size_t nbr_elements, bool only_complete) {
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
          if (*new_elements == delim) ct++;
          arr[writeindex] = *new_elements;
          writeindex = (writeindex + 1) % (CAPACITY+1);
          new_elements++;
    }
    return nbr_elements;
}

template <size_t CAPACITY>
size_t YaRBct<CAPACITY>::peek(uint8_t *peeked_element) const {
    // check for emptyness and validity of output pointer (may be nullptr)
    if (this->isEmpty() || !peeked_element) {
        return 0;
    }
    else {
        *peeked_element = arr[readindex];
        return 1;
    }
}

// modified
template <size_t CAPACITY>
size_t YaRBct<CAPACITY>::discard(size_t nbr_elements) {
    if (this->size() > nbr_elements) { // there will be remaining elements in buffer
        // use loop instead just index shifting
        for (size_t i=0; i<nbr_elements; i++) {    
            if (arr[readindex] == delim) ct--;
            readindex = (readindex + 1 ) % (CAPACITY+1);
        }
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
size_t YaRBct<CAPACITY>::get(uint8_t *returned_element) {
    // check for emptyness and validity of output pointer (may  be nullptr)
    if (this->isEmpty() || !returned_element) {
        return 0;
    }
    else {
        if (arr[readindex] == delim) ct--;
        *returned_element = arr[readindex];
        readindex = (readindex + 1) % (CAPACITY+1);
        return 1;
    }
}

// modified
template <size_t CAPACITY>
size_t YaRBct<CAPACITY>::get(uint8_t *returned_elements, size_t nbr_elements) {
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
            if (arr[readindex] == delim) ct--;
            *returned_elements = arr[readindex];
            readindex = (readindex+1) % (CAPACITY+1);
            returned_elements++;
        }
        return nbr_elements;
    }
}
        
template <size_t CAPACITY>
size_t YaRBct<CAPACITY>::size(void) const {
    if (writeindex >= readindex) {
        return writeindex - readindex;
    }
    else {
        return (CAPACITY+1) - (readindex - writeindex);
    }
}

template <size_t CAPACITY>
size_t YaRBct<CAPACITY>::free(void) const {
    return this->capacity() - this->size();
}

template <size_t CAPACITY>
size_t YaRBct<CAPACITY>::capacity(void) const {
    return CAPACITY;
}

template <size_t CAPACITY>
bool YaRBct<CAPACITY>::isFull(void) const {
    return readindex == (writeindex + 1) % (CAPACITY+1);
}

template <size_t CAPACITY>
bool YaRBct<CAPACITY>::isEmpty(void) const {
    return readindex == writeindex;
}

template <size_t CAPACITY>
void YaRBct<CAPACITY>::flush(void) {
    // fast-forward readindex to position of writeindex
    readindex = writeindex;
    ct = 0;
}

template <size_t CAPACITY>
size_t YaRBct<CAPACITY>::limit(void) {
    return SIZE_MAX - 1;
}

/**
 * @brief      Get the count of delimiter bytes within ring buffer.
 * @return     Number of delimiter bytes currently stored in ring buffer.
 */
template <size_t CAPACITY>
size_t YaRBct<CAPACITY>::count(void) const {
    return ct;
}
