/**
 * @file    yarb.cpp
 * @brief   Implementation file for the YaRB ring buffer
 * @author  Andreas Grommek
 * @version 1.0.0
 * @date    2021-06-01
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
 
#include "yarb.h"
#include <string.h>  // memcpy(), for copy constructor

/**
 * @brief   The constructor.
 * @details There is no default (i.e. parameter-less) constructor for
 *          this class.
 * @param   capacity
 *          The target capacity of the ring buffer. The array to hold all
 *          elements is allocated upon construction. The size is constant
 *          and cannot be changed afterwards.
 */
YaRB::YaRB(size_t capacity) 
    : cap{capacity}, readindex{0}, writeindex{0}, arraypointer{nullptr} {
    arraypointer = new uint8_t[cap];
}

/**
 * @brief   The copy constructor.
 * @param   rb
 *          Reference to class instance to copy.
 */
YaRB::YaRB(const YaRB &rb)
    : cap{rb.cap}, readindex{rb.readindex}, writeindex{rb.writeindex}, arraypointer{nullptr} {
    arraypointer = new uint8_t[cap];
    memcpy(arraypointer, &rb, (cap * sizeof(uint8_t)) );        
}

/**
 * @brief   The destructor.
 */
YaRB::~YaRB() {
    delete[] arraypointer;
}

bool YaRB::put(uint8_t new_element) {
    if (this->isFull()) {
        return false;
    }
    else {
        arraypointer[modcap(writeindex)] = new_element;
        writeindex = modcap2(writeindex+1);
        return true;
    }
}

bool YaRB::put(const uint8_t *new_elements, size_t nbr_elements) {
    // check for space in buffer and validity of input pointer (may be nullptr)
    if (this->free() < nbr_elements || !new_elements ) {
        return false;
    }
    else {
        for (size_t i=0; i<nbr_elements; i++) {
              // re-implementation is about 3-4 times faster than calling
              // single-element put()
              arraypointer[modcap(writeindex)] = *new_elements;
              writeindex = modcap2(writeindex+1);
              new_elements++;
        }
        return true;
    }
}

bool YaRB::peek(uint8_t *peeked_element) const {
    // check for emptyness and validity of output pointer (may be nullptr)
    if (this->isEmpty() || !peeked_element) {
        return false;
    }
    else {
        *peeked_element = arraypointer[modcap(readindex)];
        return true;
    }
}

size_t YaRB::discard(size_t nbr_elements) {
    if (this->size() > nbr_elements) { // there will be remaining elements in buffer
        // Due to danger of integer overflow, we cannot just do
        // readindex = modcap2(readindex + nbr_elements):
        // readindex + nbr_elements *might* be larger than SIZE_MAX, which
        // would then overflow, giving wrong results after modcap2.
        // --> Do modulus calculation "manually".
        //
        // The difference  between current readinday is always > 0 (i.e. at least 1)
        size_t diff_to_max = 2*cap - readindex;
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

bool YaRB::get(uint8_t *returned_element) {
    // check for emptyness and validity of output pointer (may  be nullptr)
    if (this->isEmpty() || !returned_element) {
        return false;
    }
    else {
        *returned_element = arraypointer[modcap(readindex)];
        readindex = modcap2(readindex+1);
        return true;
    }
}

bool YaRB::get(uint8_t *returned_elements, size_t nbr_elements) {
    // check for size and nullptr
    if (nbr_elements > this->size() || !returned_elements) {
        return false;
    }
    else {
        for (size_t i=0; i<nbr_elements; i++) {
            // re-implementation is about 2 times faster than calling
            // single-element get()        
            *returned_elements = arraypointer[modcap(readindex)];
            readindex = modcap2(readindex+1);
            returned_elements++;
        }
        return true;
    }
}
        
size_t YaRB::size(void) const {
    return modcap2(writeindex-readindex);
}

size_t YaRB::free(void) const {
    return cap - this->size();
}

size_t YaRB::capacity(void) const {
    return cap;
}

bool YaRB::isFull(void) const {
    return this->size() == cap;
}

bool YaRB::isEmpty(void) const {
    return readindex == writeindex;
}

void YaRB::flush(void) {
    // fast-forward readindex to position of writeindex
    readindex = writeindex;
}

size_t YaRB::limit(void) {
    return SIZE_MAX / 2;
}

inline size_t YaRB::modcap(size_t val) const {
    return (cap == 0) ? 0 : (val % cap);
}

inline size_t YaRB::modcap2(size_t val) const {
    return (cap == 0) ? 0 : (val % (2 * cap));
}
