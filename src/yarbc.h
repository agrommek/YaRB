/**
 * @file    yarbc.h
 * @brief   Header file for a ring buffer implementation with counter
 * @author  Andreas Grommek
 * @version 1.5.0
 * @date    2021-10-02
 * 
 * @section license_yarbc_h License
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

#ifndef yarbc_h
#define yarbc_h

#include "yarb_interface.h"

/**
 * @class   YaRBc
 * @brief   Classic ring buffer implementation using dynamic allocated array and
 *          two indices with additional tracking of delimiter bytes.
 * @warning This class is @b not interrupt-safe, even with only a single
 *          interrupt priority (as on AVR Arduinos) and when only adding 
 *          to it in an ISR and removing from it in loop() (or vice versa).
 *          This is due to the fact that the assignment operation for data
 *          type size_t is not atomic on some platforms.
 */
class YaRBc : public IYaRB {
    public:
        // constructor
        YaRBc(size_t capacity=63, uint8_t delimiter=0);
        
        // copy constructor
        YaRBc(const YaRBc &rb);
        
        // destructor
        virtual ~YaRBc(void);
        
        // Genrally do not allow assignments:
        // It is unclear how to handle assignment of ring buffers with
        // different capacities.
        YaRBc& operator= (const YaRBc &rb) = delete;

        // put element(s) into ring buffer
        virtual size_t put(uint8_t new_element) override;
        virtual size_t put(const uint8_t *new_elements, size_t nbr_elements, bool only_complete) override;

        // get/remove element(s) from ring buffer
        virtual size_t get(uint8_t *returned_element) override;
        virtual size_t get(uint8_t *returned_elements, size_t nbr_elements) override;
        
        // look at next element in ring buffer
        // note: there is no multi-byte-version!
        virtual size_t peek(uint8_t *peeked_element) const override; 
        
        // discard some elements from ring buffer, 
        // return number of discarded elements
        virtual size_t discard(size_t nbr_elements) override;

        virtual size_t size(void) const override;     // return number of slots in use
        virtual size_t free(void) const override;     // return number of free slots
        virtual size_t capacity(void) const override; // return total number of slots

        // function *not* from interface, but special to this class
        virtual size_t count(void) const;             // return count of messages

        virtual bool   isFull(void) const override;   // return true when buffer is full
        virtual bool   isEmpty(void) const override;  // return true when buffer is empty
        virtual void   flush(void) override;          // clear all elements from buffer
        
        // no override for static functions...
        static size_t limit(void);   // return maximum possible number of elements on a given platform

    private:
        const size_t  cap;     ///< store size of internaly array
        const uint8_t delim;   ///< delimiter for messages 

        size_t  readindex;     ///< index for get()
        size_t  writeindex;    ///< index for put()
        uint8_t *arraypointer; ///< pointer to array which holds the elements
        size_t  ct;            ///< counter for delimiter bytes
};


/**
 * @class   YaRBct
 * @brief   Classic ring buffer implementation using a template and two indices.
 *          With additional tracking of delimiter bytes.
 * @note    The template parameter specifies the @b effective, i.e. usable 
 *          capacity of the ring buffer. Internally, one additional byte is
 *          allocated.
 * @warning This class is @b not interrupt-safe, even with only a single
 *          interrupt priority (as on AVR Arduinos) and when only adding 
 *          to it in an ISR and removing from it in loop() (or vice versa).
 *          This is due to the fact that the assignment operation for data
 *          type size_t is not atomic on some platforms.
 */


template <size_t CAPACITY = 63> 
class YaRBct : public IYaRB {
    public:
        // sanity checking
        static_assert(CAPACITY > 0, "not allowed to instantiate template with CAPACITY=0");
        
        // constructor
        YaRBct(uint8_t delimiter=0);
        
        // copy constructor
        YaRBct(const YaRBct<CAPACITY> &rb);
        
        // destructor
        virtual ~YaRBct(void) = default;
        
        // Do not allow assignments, even in templated version.
        // It does not make sense to change delimting byte after construction.
        YaRBct<CAPACITY>& operator= (const YaRBct<CAPACITY> &rb) = delete;

        // put element(s) into ring buffer
        virtual size_t put(uint8_t new_element) override;
        virtual size_t put(const uint8_t *new_elements, size_t nbr_elements, bool only_complete) override;

        // get/remove element(s) from ring buffer
        virtual size_t get(uint8_t *returned_element) override;
        virtual size_t get(uint8_t *returned_elements, size_t nbr_elements) override;
        
        // look at next element in ring buffer
        // note: there is no multi-byte-version!
        virtual size_t peek(uint8_t *peeked_element) const override; 
        
        // discard some elements from ring buffer, 
        // return number of discarded elements
        virtual size_t discard(size_t nbr_elements) override;

        virtual size_t size(void) const override;     // return number of slots in use
        virtual size_t free(void) const override;     // return number of free slots
        virtual size_t capacity(void) const override; // return total number of slots

        // function *not* from interface, but special to this class --> no override
        virtual size_t count(void) const;             // return count of messages
        
        virtual bool   isFull(void) const override;   // return true when buffer is full
        virtual bool   isEmpty(void) const override;  // return true when buffer is empty
        virtual void   flush(void) override;          // clear all elements from buffer
        
        // no override for static functions...
        static size_t limit(void);   // return maximum possible number of elements on a given platform

    private:
        const uint8_t delim;         ///< delimiter for messages 

        size_t  readindex;           ///< index for get()
        size_t  writeindex;          ///< index for put()
        uint8_t arr[CAPACITY+1];     ///< array which holds the elements
        size_t  ct;                  ///< counter for delimiter bytes
};

// include imlementation file for template here
#include "yarbct.hpp"

#endif // yarbc_h
