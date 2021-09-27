/**
 * @file    yarb_interface.h
 * @brief   Header file for the YaRB interface
 * @author  Andreas Grommek
 * @version 1.1.0
 * @date    2021-09-27
 * 
 * @section license_yarb_interface_h License
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

/**
 * @mainpage Arduino Ring Buffer library
 *
 * @section intro_sec Introduction
 * 
 * This library provides classes to implement Ring Buffers.
 * 
 * @subsection reason Why another Ring Buffer implementation?
 * 
 * There are already about a gazillion implementations of Ring Buffers 
 * out there. The Arduino Core even has one built in.
 * 
 * This is my own solution of the problem. There were several reasons to 
 * make my own instead of using someone else's implementation, which might
 * even be less bug-riddled, smaller, faster and simply overall bettter.
 *
 * @li I did't like the interfaces of the implementations I found. Most 
 *     lack the possibility to determine if an operation was successful or not.
 * @li I wanted to learn how to write an abstact base class in C++. The 
 *     best way to learn something is just doing it.
 * @li I wanted to learn how to write a class which implements an abstract
 *     base class in C++. Again, by doing it.
 * @li By creating a Ring Buffer interface, I can (maybe later, probably
 *     never?) create several other implementations which have all the 
 *     same "look & feel", comparing them, benchmarking them, ...
 * @li I wanted to learn how to use Doxygen to document class hierarchies
 *     and abstact base classes.
 *
 * So, there you have it. If you don't like my implementation, use someone 
 * else's. I had a lot of fun writing my own. :-)
 *
 * @subsection use_case My use case
 *
 * I use my Ring Buffer implementation to receive messages over a serial 
 * connection with high speeds (2 Mbit/s with an Adafruit Feather M0). 
 * In my project, different messages can have different length and are 
 * zero-delimited. Before a message can be processed, it must be received 
 * completely. The bytes belonging to a message are temporaryly stored in
 * a Ring Buffer before beeing batch-processed when the message is complete.
 *
 * @subsection compatibilty  Compatibility
 * 
 * Although written originally for the Arduino platform, there is nothing
 * which prevents the library from being used on any other platform.
 * The code is pure C++. Feel free to adapt to your needs.
 *
 * @subsection usage Usage
 * 
 * Add the line `#include <yarb.h>` somwhere at the top of your Arduino sketch (or your .cpp file on other platforms).
 * 
 * The followng table summarized the usage of the YaRB class:
 * 
 * |Method | Short description |
 * | :---- | :---------------- |
 * | `size_t put(uint8_t new_element)` | Add a single byte to the Ring Buffer. |
 * | `size_t put(uint8_t * new_elements, size_t nbr_elements)` | All several bytes from an array to the Ring Buffer |
 * | `size_t get(uint_8 * returned_element)` | Get a single byte back out from the Ring Buffer |
 * | `size_t get(uint_8 * returned_elements, size_t nbr_elements)` | Get several bytes back out from the Ring Buffer and write them to an array. |
 * | `size_t peek(uint_8 * returned_element)` | Get next element from the ring buffer while *not* removing it from the buffer. |
 * | `size_t discard(size_t nbr_elements)`| Discard one or more bytes from the Ring Buffer. |
 * | `size_t size(void)` | Return the number of stored bytes. |
 * | `size_t free(void)` | Return the nuber of free slots. |
 * | `size_t capacity(void)` | Return the total capacity of the Ring Buffer. |
 * | `bool isEmpty(void)` | Return `true` if Ring Buffer is empty. |
 * | `bool isFull(void)` | Return `true` if Ring Buffer is full. |
 * | `size_t limit(void)` | Return the maximum theoretical capacity of a Ring Buffer on a given platform. Note that the real limit is most probably *much* smaller. |
 * 
 * For Details, please refer to the Doxygen-generated documentation.
 * 
 * Note that many arguments are *pointers*. If you don't understand pointers, 
 * go learn some more C++ first - it's not really *that* difficult. 
 * [This](https://www.learncpp.com/) is a great resouce to teach 
 * yourself C++, IMHO. 
 * 
 * @subsection interrupts A few word on interrupts and thread safety
 * 
 * The current inplementation is **not interrupt-safe**. This means that one 
 * should **not** use the same Ring Buffer instance inside and outside an
 * interrupt service routine (ISR). To make a Ring Buffer implementation 
 * really interrupt-safe the following conditions must be met.
 * 
 * @li Never update the same member variables in `put()` and `get()`. Otherwise race conditions can ocurr. This is already the case with my implementation
 * @li Declare member variables which can be changed inside ISR *volatile*. Easily doable, but not enough.
 * @li With implementations using an array and to pointers into an array, only ever update the pointers after the actual read/write operations on the array. This is already the case with my implementation.
 * @li Only use `get()` in the ISR and `put()` in your main program or vice versa. Having two instances of `get()` or `put()`at the same time is asking for trouble.
 * @li The update of the pointers into the array must be atomic.
 *
 * The last point is the real bummer. For this condition to be fulfilled,
 * one must be able to absolutely guarantee that assignments are executed
 * within a single CPU cycle. Depending on platform and data type, this is
 * by no means always the case. On 8-bit architectures (like Arduino AVR),
 * assignments for data types larger than, say, `int8_t` or `char`or `uint8_t`
 * take more than one CPU cycle. The data type used in this library, `size_t`,
 * is 16 bit wide on AVR Arduinos and 32 bit on ARM boards. There are workarounds, 
 * like wrapping critical code sections in `interrups()`/`noInterrups()`. 
 * This, however, is not always enough. Maybe we cannot simply disable and
 * re-enable global interrupts, because somthing else will then break? 
 * Or there are other subtle pitfalls? ARM Cortex CPUs, for example, have
 * a notion of interrupt priorities - interrupts with higher priority can
 * interrupt currently running ISRs with lower priority...). What if an 
 * ISR currently updating a Ring Buffer is interrupted by another ISR 
 * which wants to update the same buffer? Personally, I wouldn't like to
 * open that particular can of worms.
 * 
 * Long story short: Making a Ring Buffer interrupt-safe and cross-platform
 * is a difficult task. I dare to claim than most Ring Buffer implementations
 * out there are maybe not as interrupt-safe as advertised, given the right
 * circumstances. I am not satified with promises like "should work with
 * interrupts most of the time". Either it is guaranteed to work in all 
 * cases and under all circumstances or not.
 * 
 * Therefore, I say it right up front: **Do not use my Ring Buffer
 * implementation within ISRs.**
 * 
 * Maybe I will try to implement an interrupt-safe version later. The
 * abstact base class is now there, after all. ;-)
 * 
 * @subsection thanks Thanks and Credits
 * 
 * The implementation is not quite textbook-like. With most implementations, an array of N bytes is allocated, but only N-1 bytes can be used. This implementation can use the whole range of allocated space for only very slight additional runtime overhead. The idea was inspired by [this article](https://www.snellman.net/blog/archive/2016-12-13-ring-buffers/) and the discussion in the comments section underneath it.
 * 
 * @section author Author
 *
 * Andreas Grommek
 *
 * @section license License
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
 *
 * @version  1.0.0
 * @date     2021-06-01
 */
 
#ifndef yarb_interface_h
#define yarb_interface_h

#include <stddef.h> // needed for size_t data type
#include <stdint.h> // needed for uint8_t data type

/**
 * @brief   Abstract base class for ring buffers.
 * @details This class defines the interface for ring buffers. There are
 *          several possible implementations with differing properties 
 *          for ring buffers. To be able to use these differing implementations
 *          interchangibly, a common interface is hereby defined.
 */
class IYaRB {
    public:
        virtual size_t put(uint8_t new_element) = 0;
        virtual size_t put(const uint8_t *new_elements, size_t nbr_elements) = 0;

        virtual size_t get(uint8_t *returned_element) = 0;
        virtual size_t get(uint8_t *returned_elements, size_t nbr_elements) = 0;
        
        virtual size_t peek(uint8_t *peeked_element) const = 0;

        virtual size_t discard(size_t nbr_elements) = 0;

        virtual size_t size(void) const = 0;
        virtual size_t free(void) const = 0;
        virtual size_t capacity(void) const = 0;

        virtual bool   isFull(void) const = 0;
        virtual bool   isEmpty(void) const = 0;
        
        virtual void   flush(void) = 0;
        
        static  size_t limit(void);
        
        // Virtual destructor to make sure, that derived objects are
        // destroyed properly, even when called via pointer to base class.
        virtual ~IYaRB() = default;
        
        IYaRB& operator= (const IYaRB &yarb) = delete; ///< Do not allow implicit creation of assignment operator in derived classes.
};

// put() single
/**
 * @fn         virutal bool IYaRB::put(uint8_t new_element)
 * @brief      Add a single element to ring buffer.
 * @param      new_element
 *             The new element to add to the ring buffer.
 * @return     number of elements added to ring buffer (either 0 or 1)
 */

// put() multiple
/**
 * @fn         virtual bool IYaRB::put(const uint8_t *new_elements, size_t nbr_elements)
 * @brief      Add several new elements to ring buffer.
 * @details    This functions adds several elements to the ring buffer in
 *             an "all or nothing" manner. If the ring buffer is big enough
 *             to hold nbr_elements additional elements, @b all requested 
 *             elements will be added to the ring buffer. Otherwise, @b no 
 *             elements are added at all (i.e. the ring buffer remains
 *             unchanged).
 * @param[in]  new_elements
 *             Pointer to array with new elements to add to the ring buffer.
 * @param      nbr_elements
 *             Number of elements to add from array new_elements to ring buffer.
 *             If the ring buffer is not big enough to hold all additional new
 *             elements @b no element is added at all.
 * @return     number of elements added to ring buffer
 */

// get() single
/**
 * @fn         virtual bool IYaRB::get(uint8_t *returned_element)
 * @brief      Get a single element from the ring buffer, thereby removing
 *             it from the buffer.
 * @details    If the ring buffer does not hold any elements (i.e. it is
 *             empty), nothing is written to the memory address of
 *             returned_element.
 * @param[out] returned_element
 *             Pointer to a uint8_t. The returned element is stored in the
 *             memory address this pointer points to.
 * @return     number of elements removed from ring buffer and copied to
 *             returned_elements (either 0 or 1)
 */

// get() multiple
/**
 * @fn         virtual bool IYaRB::get(uint8_t *returned_elements, size_t nbr_elements)
 * @brief      Get several elements from the ring buffer, thereby removing
 *             them from the buffer.
 * @details    This is an "all or nothing" operation. Either @b all 
 *             requested elements will be taken from the ring buffer and written
 *             to returned_elements (in which case the function returns 
               @em true) or @b no elements are removed at all (i.e. the ring
 *             buffer remains unchanged) and the function returns @em false.
 * @param[out] returned_elements
 *             Pointer to a uint8_t. The returend elements are stored  in
 *             an array starting at the memory address this pointer points to.
 * @param      nbr_elements
 *             Number of elements to get out of the ring buffer and write
 *             to returned_elements. 
 *             If the ring buffer does not have enough elements stored,
 *             @b no elements are returned at all.
 * @return     number of elements removed from ring buffer and copied to
 *             returned_elements
 */

// peek()
/**
 * @fn         virtual bool IYaRB::peek(uint8_t *peeked_element) const
 * @brief      Return next element from the ring buffer while @b not removing it
 *             from the buffer.
 * @param[out] peeked_element
 *             Pointer to a uint8_t. The peeked element is stored 
 *             at the memory address this pointer points to.
 * @return     number of elements peeked, i.e. copied to peeked_element
 *             (either 0 or 1)
 */

// discard()
/**
 * @fn         virtual size_t IYaRB::discard(size_t nbr_elements)
 * @brief      Discard (i.e. remove) some elements from ring buffer without
 *             writing them to some output buffer.
 * @param      nbr_elements
 *             Number of elements to discard (i.e. remove) from ring buffer.
 * @return     Number of elements removed from ring buffer. This is
 *             nbr_elements if nbr_elements >= size(), otherwise size().
 */
 
// size()
/**
 * @fn         virtual bool IYaRB::size(void) const
 * @brief      Get number of currently used slots in ring buffer.
 * @return     Number of elements which can be gotten out of the to ring
 *             buffer (using get()) before it is empty.
 */

// free()
/**
 * @fn         virtual bool IYaRB::free(void) const
 * @brief      Get number of currently free slots in ring buffer.
 * @return     Number of elements which can be added to ring buffer (using
 *             put()) before it is full.
 */

// capacity()
/**
 * @fn         virtual bool IYaRB::capacity(void) const
 * @brief      Get the total size of this ring buffer instance, i.e. the
 *             maximum number of elements this instance can store.
 * @return     Number of elements which can be stored in this ring buffer
 *             instance. Is semantically equal to size() + free(), but faster.
 */

// isFull()
/**
 * @fn         virtual bool IYaRB::isFull(void) const
 * @brief      Determine if ring buffer is full.
 * @details    put() will return @em false if called on a full ring buffer.
 * @note       Calling isFull() is semantically identical to <em>size() ==
 *             capacity()</em>. However, using isFull() is more conventient
 *             and also has better performance.
 * @return     @em true if there is at least room for one more element in
 *             ring buffer, @em false otherwise.
 */

// isEmpty()
/**
 * @fn         virtual bool IYaRB::isEmpty(void) const
 * @brief      Determine if ring buffer is empty.
 * @details    get() will return @em false if called on an empty ring buffer.
 * @note       Calling isEmpty() is semantically identical to <em>size() ==
 *             0</em>. However, using isEmpty() is more conventient
 *             and also has slightly better performance.
 * @return     @em true if there no element stored in ring buffer,
 *             @em false otherwise.
 */

/**
 * @fn         virtual bool IYaRB::flush(void)
 * @brief      Clear out all elements stored in the ring buffer.
 * @details    After a call to flush(), isEmpty() will return @em true.
 */

// limit()
/**
 * @fn         static size_t IYaRB::limit(void)
 * @brief      Show the (theoretical) limit of ring buffer size.
 * @details    The acutual maximum size will be most probably much lower
 *             than this.
 * @return     Absolute maximum number of element for a ring buffer of this 
 *             type.
 */
#endif // yarb_interface_h
