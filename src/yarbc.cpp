/**
 * @file    yarbc.cpp
 * @brief   Implementation file for a ring buffer implementation with counter.
 * @author  Andreas Grommek
 * @version 1.5.0
 * @date    2021-10-02
 * 
 * @section license_yarbc_cpp License
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

#include "yarbc.h"

#include <string.h>  // memcpy(), for copy constructor

/* YaRBc */

/*
 * Note:
 * Some method implementations are exactly the same as for YaRB, namely
 * all methods marked const:
 * 
 * peek(), size(), free(), capacity(), isFull(), isEmpty(), 
 * plus static limit()
 * 
 * The following methods are modified to facilitate the counter:
 * 
 * put(), get(), discard(), flush()
 * 
 * There is one new method not inherited from IYaRB: count()
 */
 
/**
 * @brief   The constructor.
 * @details There is no default (i.e. parameter-less) constructor for
 *          this class.
 * @param   capacity
 *          The target capacity of the ring buffer. The array to hold all
 *          elements is allocated upon construction. The size is constant
 *          and cannot be changed afterwards.
 * @param   delimiter
 *          A message delimiter. Putting a byte with this value will into
 *          the ring buffer will increase the value returned by count()
 *          by one. Removing a byte with this value (get(), discard(), 
 *          flush()) will decreases this value.
 * @note    capacity is the effectively usable capacity of the ring buffer.
 *          This implementation allocates one additional byte internally.
 */
YaRBc::YaRBc(size_t capacity, uint8_t delimiter) 
    : cap{capacity+1}, delim{delimiter}, readindex{0}, writeindex{0}, arraypointer{nullptr}, ct{0} {
    arraypointer = new uint8_t[cap];
}

/**
 * @brief   The copy constructor.
 * @param   rb
 *          Reference to class instance to copy.
 */
YaRBc::YaRBc(const YaRBc &rb)
    : cap{rb.cap}, delim{rb.delim}, readindex{rb.readindex}, writeindex{rb.writeindex}, arraypointer{nullptr}, ct{rb.ct} {
    arraypointer = new uint8_t[cap];
    memcpy(arraypointer, &(rb.arraypointer), cap);        
}

/**
 * @brief   The destructor.
 */
 // same as for YaRB
YaRBc::~YaRBc() {
    delete[] arraypointer;
}

// modified compared to YaRB
size_t YaRBc::put(uint8_t new_element) {
    if (this->isFull()) {
        return 0;
    }
    else {
        if (new_element == delim) ct++;
        arraypointer[writeindex] = new_element;
        writeindex = (writeindex + 1) % cap;
        return 1;
    }
}

// modified compared to YaRB
size_t YaRBc::put(const uint8_t *new_elements, size_t nbr_elements, bool only_complete) {
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
          arraypointer[writeindex] = *new_elements;
          writeindex = (writeindex + 1) % cap;
          new_elements++;
    }
    return nbr_elements;
}

// same as for YaRB
size_t YaRBc::peek(uint8_t *peeked_element) const {
    // check for emptyness and validity of output pointer (may be nullptr)
    if (this->isEmpty() || !peeked_element) {
        return 0;
    }
    else {
        *peeked_element = arraypointer[readindex];
        return 1;
    }
}

// modified compared to YaRB
size_t YaRBc::discard(size_t nbr_elements) {
    if (this->size() > nbr_elements) { // there will be remaining elements in buffer
        // use loop instead just index shifting
        for (size_t i=0; i<nbr_elements; i++) {    
            if (arraypointer[readindex] == delim) ct--;
            readindex = (readindex + 1 ) % cap;
        }
        return nbr_elements;
    }
    else { // discard *all* elements --> flush()
        // we can only discard as many elements as are in the buffer
        // --> return size()
        const size_t retval = this->size();
        this->flush();
        return retval;
    }
}

// modified compared to YaRB
size_t YaRBc::get(uint8_t *returned_element) {
    // check for emptyness and validity of output pointer (may  be nullptr)
    if (this->isEmpty() || !returned_element) {
        return 0;
    }
    else {
        if (arraypointer[readindex] == delim) ct--;
        *returned_element = arraypointer[readindex];
        readindex = (readindex + 1) % cap;
        return 1;
    }
}

// modified compared to YaRB
size_t YaRBc::get(uint8_t *returned_elements, size_t nbr_elements) {
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
            if (arraypointer[readindex] == delim) ct--;
            *returned_elements = arraypointer[readindex];
            readindex = (readindex + 1 ) % cap;
            returned_elements++;
        }
        return nbr_elements;
    }
}
        
// same as for YaRB
size_t YaRBc::size(void) const {
    if (writeindex >= readindex) {
        return writeindex - readindex;
    }
    else {
        return cap - (readindex - writeindex);
    }
}

// same as for YaRB
size_t YaRBc::free(void) const {
    return this->capacity() - this->size();
}

// same as for YaRB
size_t YaRBc::capacity(void) const {
    return cap-1;
}

// same as for YaRB
bool YaRBc::isFull(void) const {
    return readindex == (writeindex + 1) % cap;
}

// same as for YaRB
bool YaRBc::isEmpty(void) const {
    return readindex == writeindex;
}

// modified compared to YaRB
void YaRBc::flush(void) {
    // fast-forward readindex to position of writeindex
    readindex = writeindex;
    ct = 0;
}

// same as for YaRB
size_t YaRBc::limit(void) {
    return SIZE_MAX - 1;
}

/**
 * @brief      Get the count of delimiter bytes within ring buffer.
 * @return     Number of delimiter bytes currently stored in ring buffer.
 */
size_t YaRBc::count(void) const {
    return ct;
}
