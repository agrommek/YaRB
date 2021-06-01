/**
 * @file    yarb.h
 * @brief   Header file for the YaRB ring buffer
 * @author  Andreas Grommek
 * @version 1.0.0
 * @date    2021-006-01
 * 
 * @section license_yarb_h License
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

#ifndef yarb_h
#define yarb_h

#include "yarb_interface.h"

/**
 * @class   YaRB
 * @brief   Ring buffer implementation using dynamic allocated array and
 *          two indices.
 * @details Unlike most implementations, it is possible to use the full
 *          number of elements allocated. This is made possible by calculating
 *          the indices <em> mod(2*capacity)</em>. The implementation was 
 *          inspired by this article and the discussion in the comments:
 *          https://www.snellman.net/blog/archive/2016-12-13-ring-buffers/
 * @warning This class is @b not interrupt-safe, even with only a single
 *          interrupt priority (as on AVR Arduinos) and when only adding 
 *          to it in an ISR and removing from it in loop() (or vice versa).
 *          This is due to the fact that the assignment operation for data
 *          type size_t is not atomic on some platforms.
 */
class YaRB : public YaRB_interface {
    public:
        // constructor
        YaRB(size_t capacity);
        
        // copy constructor
        YaRB(const YaRB &rb);
        
        // destructor
        ~YaRB(void);
        
        // do not allow assignments
        YaRB& operator= (const YaRB &rb) = delete;

        // put element(s) into ring buffer
        bool   put(uint8_t new_element);
        bool   put(const uint8_t *new_elements, size_t nbr_elements);

        // get/remove element(s) from ring buffer
        bool   get(uint8_t *returned_element);
        bool   get(uint8_t *returned_elements, size_t nbr_elements);
        
        // look at next element in ring buffer
        // note: there is no multi-byte-version!
        bool   peek(uint8_t *peeked_element) const; 
        
        // discard some elements from ring buffer, 
        // return number of discarded elements
        size_t discard(size_t nbr_elements);

        size_t size(void) const;     // return number of slots in use
        size_t free(void) const;     // return number of free slots
        size_t capacity(void) const; // return total number of slots

        bool   isFull(void) const;   // return true when buffer is full
        bool   isEmpty(void) const;  // return true when buffer is empty
        void   flush(void);          // clear all elements from buffer
        
        static size_t limit(void);   // return maximum possible number of elements on a given platform

    private:
        const size_t  cap;     ///< store capacity of ring buffer
        size_t  readindex;     ///< index for get()
        size_t  writeindex;    ///< index for put()
        uint8_t *arraypointer; ///< pointer to array which holds the elements
        
        size_t  modcap(size_t val) const;
        size_t  modcap2(size_t val) const;
};

#endif // yarb_h


