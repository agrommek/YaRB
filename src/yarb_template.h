/**
 * @file    yarb_template.h
 * @brief   Header file for the YaRB ring buffer in a template version
 * @author  Andreas Grommek
 * @version 1.2.0
 * @date    2021-09-28
 * 
 * @section license_yarb_template_h License
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

#ifndef yarb_template_h
#define yarb_template_h

#include "yarb_interface.h"

/**
 * @class   YaRB_template
 * @brief   Ring buffer implementation using a template and two indices.
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
template <size_t CAPACITY = 64>
class YaRB_template : public IYaRB {
    public:
        // sanity checking
        static_assert(CAPACITY > 0, "not allowed to instantiate template with CAPACITY=0");
        
        // constructor
        YaRB_template(void);
        
        // copy constructor
        YaRB_template(const YaRB_template &rb);
        
        // destructor
        virtual ~YaRB_template(void) = default;
        
        // do not allow assignments
        YaRB_template& operator= (const YaRB_template &rb) = delete;

        // put element(s) into ring buffer
        size_t put(uint8_t new_element) override;
        size_t put(const uint8_t *new_elements, size_t nbr_elements) override;

        // get/remove element(s) from ring buffer
        size_t get(uint8_t *returned_element) override;
        size_t get(uint8_t *returned_elements, size_t nbr_elements) override;
        
        // look at next element in ring buffer
        // note: there is no multi-byte-version!
        size_t peek(uint8_t *peeked_element) const override; 
        
        // discard some elements from ring buffer, 
        // return number of discarded elements
        size_t discard(size_t nbr_elements) override;

        size_t size(void) const override;     // return number of slots in use
        size_t free(void) const override;     // return number of free slots
        size_t capacity(void) const override; // return total number of slots

        bool   isFull(void) const override;   // return true when buffer is full
        bool   isEmpty(void) const override;  // return true when buffer is empty
        void   flush(void) override;          // clear all elements from buffer
        
        // no override for static functions...
        static size_t limit(void);   // return maximum possible number of elements on a given platform

    private:
        size_t  readindex;           ///< index for get()
        size_t  writeindex;          ///< index for put()
        uint8_t arr[CAPACITY];       ///< array which holds the elements
        
        size_t  modcap(size_t val) const;
        size_t  modcap2(size_t val) const;
};

// include imlementation file here
#include "yarb_template.hpp"

#endif // end of yarb_template_h


