/**
 * @file    yarb.h
 * @brief   Header file for a ring buffer implementations
 * @author  Andreas Grommek
 * @version 1.3.0
 * @date    2021-09-29
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
 * @brief   Classic ring buffer implementation using dynamic allocated array and
 *          two indices.
 * @warning This class is @b not interrupt-safe, even with only a single
 *          interrupt priority (as on AVR Arduinos) and when only adding 
 *          to it in an ISR and removing from it in loop() (or vice versa).
 *          This is due to the fact that the assignment operation for data
 *          type size_t is not atomic on some platforms.
 */
class YaRB : public IYaRB {
    public:
        // constructor
        YaRB(size_t capacity=63);
        
        // copy constructor
        YaRB(const YaRB &rb);
        
        // destructor
        virtual ~YaRB(void);
        
        // Genrally do not allow assignments:
        // It is unclear how to handle assignment of ring buffers with
        // different capacities.
        YaRB& operator= (const YaRB &rb) = delete;

        // put element(s) into ring buffer
        size_t put(uint8_t new_element) override;
        size_t put(const uint8_t *new_elements, size_t nbr_elements, bool only_complete) override;

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
        const size_t  cap;     ///< store size of internaly array
        size_t  readindex;     ///< index for get()
        size_t  writeindex;    ///< index for put()
        uint8_t *arraypointer; ///< pointer to array which holds the elements
};


/**
 * @class   YaRBt
 * @brief   Classic ring buffer implementation using a template and two indices.
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
class YaRBt : public IYaRB {
    public:
        // sanity checking
        static_assert(CAPACITY > 0, "not allowed to instantiate template with CAPACITY=0");
        
        // constructor
        YaRBt(void);
        
        // copy constructor
        YaRBt(const YaRBt &rb);
        
        // destructor
        virtual ~YaRBt(void) = default;
        
        // allow assignments for templated version, as CAPACITY is constant
        YaRBt<CAPACITY>& operator= (const YaRBt<CAPACITY> &rb);

        // put element(s) into ring buffer
        size_t put(uint8_t new_element) override;
        size_t put(const uint8_t *new_elements, size_t nbr_elements, bool only_complete) override;

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
        uint8_t arr[CAPACITY+1];       ///< array which holds the elements
};

// include imlementation file for template here
#include "yarbt.hpp"

/**
 * @class   YaRB2
 * @brief   Alternative ring buffer implementation using dynamic allocated array and
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
class YaRB2 : public IYaRB {
    public:
        // constructor
        YaRB2(size_t capacity=63);
        
        // copy constructor
        YaRB2(const YaRB2 &rb);
        
        // destructor
        virtual ~YaRB2(void);
        
        // do not allow assignments
        YaRB2& operator= (const YaRB2 &rb) = delete;

        // put element(s) into ring buffer
        size_t put(uint8_t new_element) override;
        size_t put(const uint8_t *new_elements, size_t nbr_elements, bool only_complete) override;

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
        const size_t  cap;     ///< store capacity of ring buffer
        size_t  readindex;     ///< index for get()
        size_t  writeindex;    ///< index for put()
        uint8_t *arraypointer; ///< pointer to array which holds the elements
        
        size_t  modcap(size_t val) const;
        size_t  modcap2(size_t val) const;
};

/**
 * @class   YaRB2t
 * @brief   Alternative ring buffer implementation using a template and two indices.
 * @details Unlike most implementations, it is possible to use the full
 *          number of elements allocated. This is made possible by calculating
 *          the indices <em> mod(2*capacity)</em>. The implementation was 
 *          inspired by this article and the discussion in the comments:
 *          https://www.snellman.net/blog/archive/2016-12-13-ring-buffers/
 * @note    The template parameter specifies the @b effective, i.e. usable 
 *          capacity of the ring buffer.
 * @warning This class is @b not interrupt-safe, even with only a single
 *          interrupt priority (as on AVR Arduinos) and when only adding 
 *          to it in an ISR and removing from it in loop() (or vice versa).
 *          This is due to the fact that the assignment operation for data
 *          type size_t is not atomic on some platforms.
 */
template <size_t CAPACITY = 64>
class YaRB2t : public IYaRB {
    public:
        // sanity checking
        static_assert(CAPACITY > 0, "not allowed to instantiate template with CAPACITY=0");
        
        // constructor
        YaRB2t(void);
        
        // copy constructor
        YaRB2t(const YaRB2t &rb);
        
        // destructor
        virtual ~YaRB2t(void) = default;
        
        // do not allow assignments
        YaRB2t& operator= (const YaRB2t<CAPACITY> &rb) = delete;

        // put element(s) into ring buffer
        size_t put(uint8_t new_element) override;
        size_t put(const uint8_t *new_elements, size_t nbr_elements, bool only_complete) override;

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

// include imlementation file for template here
#include "yarb2t.hpp"

#endif // end of yarb_h


