# Yet another Ring Buffer (YaRB)

## Why another Ring Buffer implementation?

There are already about a gazillion implementations of Ring Buffers out there. The Arduino Core even has one built in.

This is my own solution of the problem. There were several reasons to make my own instead of using someone else's implementation, which might even be less bug-riddled, smaller, faster and simply overall bettter.

 - I did't like the interfaces of the implementations I found. Most lack the possibility to determine if an operation was successful or not.
 - I wanted to learn how to write an abstact base class in C++. The best way to learn something is just doing it.
 - I wanted to learn how to write a class which implements an abstract base class in C++. Again, by doing it.
 - By creating a Ring Buffer interface, I can (maybe later, probably never?) create several other implementations which have all the same "look & feel", comparing them, benchmarking them, ...
 - I wanted to learn how to use Doxygen to document class hierarchies and abstact base classes.

So, there you have it. If you don't like my implementation, use someone else's. I had a lot of fun writing my own. :-)

## My use case

I use my Ring Buffer implementation to receive messages over a serial connection with high speeds (2 Mbit/s with an Adafruit Feather M0). In my project, different messages can have different length and are zero-delimited. Before a message can be processed, it must be received completely. The bytes belonging to a message are temporaryly stored in a Ring Buffer before beeing batch-processed when the message is complete.

## Compatibility

Although written originally for the Arduino platform, there is nothing which prevents the library from being used on any other platform. The code is pure C++. Feel free to adapt to your needs.

## Usage

Add the line `#include <yarb.h>` somwhere at the top of your Arduino sketch (or your .cpp file on other platforms).

The following table summarized the usage of the YaRB interface:


|Method | Short description |
| :---- | :---------------- |
| `size_t put(uint8_t new_element)` | Add a single byte to the Ring Buffer. |
| `size_t put(uint8_t * new_elements, size_t nbr_elements, bool only_complete)` | Add several bytes from an array to the Ring Buffer. |
| `size_t get(uint_8 * returned_element)` | Get a single byte back out from the Ring Buffer. |
| `size_t get(uint_8 * returned_elements, size_t nbr_elements)` | Get several bytes back out from the Ring Buffer and write them to an array. |
| `size_t peek(uint_8 * returned_element)` | Get next element from the ring buffer while *not* removing it from the buffer. |
| `size_t discard(size_t nbr_elements)`| Discard one or more bytes from the Ring Buffer. |
| `size_t size(void)` | Return the number of stored bytes. |
| `size_t free(void)` | Return the nuber of free slots. |
| `size_t capacity(void)` | Return the total capacity of the Ring Buffer. |
| `bool isEmpty(void)` | Return `true` if Ring Buffer is empty. |
| `bool isFull(void)` | Return `true` if Ring Buffer is full. |
| `size_t limit(void)` | Return the maximum theoretical capacity of a Ring Buffer on a given platform. Note that the real limit is most probably *much* smaller. |

For Details, please refer to the Doxygen-generated documentation.

Note that many arguments are *pointers*. If you don't understand pointers, go learn some more C++ first - it's not really *that* difficult. [This](https://www.learncpp.com/) is a great resouce to teach yourself C++, IMHO. 

## A few word on interrupts and thread safety

The current inplementation in **not interrupt-safe**. This means that one should **not** use the same Ring Buffer instance inside and outside an interrupt service routine (ISR). To make a Ring Buffer implementation really interrupt-safe the following conditions must be met.

 - Never update the same member variables in `put()` and `get()`. Otherwise race conditions can ocurr. This is already the case with my implementation
 - Declare member variables which can be changed inside ISR *volatile*. Easily doable, but not enough.
 - With implementations using an array and to pointers into an array, only ever update the pointers after the actual read/write operations on the array. This is already the case with my implementation.
 - Only use `get()` in the ISR and `put()` in your main program or vice versa. Having two instances of `get()` or `put()`at the same time is asking for trouble.
 - The update of the pointers into the array must be atomic.

The last point is the real bummer. For this condition to be fulfilled, one must be able to absolutely guarantee that assignments are executed within a single CPU cycle. Depending on platform and data type, this is by no means always the case. On 8-bit architectures (like Arduino AVR), assignments for data types larger than, say, `int8_t` or `char`or `uint8_t` take more than one CPU cycle. The data type used in this library, `size_t`, is 16 bit wide on AVR Arduinos and 32 bit on ARM boards. There are workarounds, like wrapping critical code sections in `interrups()`/`noInterrups()`. This, however, is not always enough. Maybe we cannot simply disable and re-enable global interrupts, because somthing else will then break? Or there are other subtle pitfalls? ARM Cortex CPUs, for example, have a notion of interrupt priorities - interrupts with higher priority can interrupt currently running ISRs with lower priority...). What if an ISR currently updating a Ring Buffer is interrupted by another ISR which wants to update the same buffer? Personally, I wouldn't like to open that particular can of worms.

Long story short: Making a Ring Buffer interrupt-safe and cross-platform is a difficult task. I dare to claim than most Ring Buffer implementations out there are maybe not as interrupt-safe as advertised, given the right circumstances. I am not satified with promises like "should work with interrupts most of the time". Either it is guaranteed to work in all cases and under all circumstances or not.

Therefore, I say it right up front: **Do not use my Ring Buffer implementation within ISRs**

Maybe I will try to implement an interrupt-safe version later. The abstact base class is now there, after all. ;-)

## Implementations of the interface

There are several implementations of the interface. More will probably added later.

### "Regular" vs. "templated" implementations

Most implementations exist in a "regular" and in a "templated" version.

In the "regular" version, the maximum capacity of the ring buffer is given as parameter to the constructor and the memory is dynamically allocated. In the "templated" version, the constructor does not take the capacity as an a parameter. The capacity is given as a non-type template parameter. Memory is statically allocated with templated versions.

Templated versions have advantages over regular version: They are typically a little bit faster and result in slightly smaller code for a given capacity. Memory is allocated on the stack, not on the heap. However, the compiler will have to generate separate code for every capacity in use. This may result in code bloat when many buffers with differing capacities are needed. Also, all capacities must be known at compile time.

Use templated versions if:

 - You need the assignment operator (`operator=`) to work. Assignment is only implemented for templated versions, because only there we can guarantee that capacities match.
 - There are only ring buffers in your program with few differing capacities, ideally only one capacity for all ring buffers in use. Using multiple templated ring buffers with different capacities may lead to code bloat.
 - All needed capacities are known at compile time. Non-type template parameters must be constexpr.
 - You worry about heap fragmentation and/or dynamic memory is too limited.
 - You want the very fastest performance and do not care about code bloat.

Use a regular version if:

 - You do not know the needed capacity of your ring buffer(s) at compile time.
 - You need many ring buffers with different capacities.
 - You do not worry about heap fragmentation.
 - Performance is not so critical.

### Classic implementation (YaRB & YaRBt)

This is a classic implementation using two indices into an array. There is always one more byte allocated than can be effectively used. `YaRB` is the regular implementation, `YaRBt`the templated version.

### Full array usage (YaRB2 & YaRB2t)

The implementations `YaRB2` and `YaRB2t` (normal and template version, see above) are not quite textbook-like. With most implementations (as with the "classic" version above), an array of N bytes is allocated, but only N-1 bytes can be used. These implementations can use the whole range of allocated space for only very slight additional runtime overhead. The idea was inspired by [this article](https://www.snellman.net/blog/archive/2016-12-13-ring-buffers/) and the discussion in the comments section underneath it.

In the end, this implementation tends to be slower *and* takes more storage than the "classic" implementation. It turned out not to be worth the effort as this implementation is not better than the classic implementation and not really useful for production code. But do your own benchmarks and see for yourself. However, it was a nice coding exercise. :-)

### Implementation with additional counter (YaRBc & YaRBct)

There is yet another implementation in `yarbc.h`("c" for "counter"). This version extends the `IYaRB` interface and adds a funcion `count()`, which returns the number of bytes with a predefined value. The reasoning behind this is as follows: I use this ring buffer to temporarily store data which comes in through a serial connection. The messages are zero-delimited (encoded by the COBS algorithm, see [this repository](https://github.com/agrommek/cobs/)). For further processing we have to be able figure out if a messages was already received completely or not. If `count()` returns at least one, we know we can pull out at least one complete message from the ring buffer.

There were several implementation options:

 - Extend the `YaRB` and/or `YaRBt` classes.
 - Implement the `IYaRB` interface and simply add another function.
 - Create an "extended Interface" (i.e. `IYaRB` plus `count()` and implement *that* interface.

I used the second option, although this resulted in a lot of code duplication (the *const* methods are all identical to `YaRB`, the other ones only differ in a few lines of code each). With the first option it would have been possible to implement the additional functionality in termin of publicly accessible methods, but this would have been clunky and not very performant. The third option felt like overkill.

Note that by declaring `count()` in the derived class and not in the `IYaRB` interface we cannot access this functionality through a base class pointer.