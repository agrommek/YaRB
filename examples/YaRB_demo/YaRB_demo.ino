/*
    YaRB Demo

    This example demonstrates the use of the YaRB ring buffer library.
    https://github.com/agrommek/YaRB

    This example code is in the public domain.
*/

// include the library
#include <yarb.h>

// select capacity of ring buffer
const size_t RB_CAPACITY = 20;

// Create a ring buffer able to hold RB_CAPACITY bytes

// Select here if you want to use the version with dynamically allocated memory or the templated version.

YaRB ringbuf(RB_CAPACITY);     // dynamically allocated storage
//YaRB2 ringbuf(RB_CAPACITY);    // dynamically allocated storage, classic implementation
//YaRB2t<RB_CAPACITY> ringbuf;   // statically allocated storage, template version

// Note: If you want to use the default size of 64, use something like this:
//    YaRB2t<> ringbuf     <-- for C++11 (Arduino & Co)
//    YaRB2t   ringbuf     <-- for C++14 and newer

// declare some variable to use for this test
uint8_t x;
uint8_t y;

void setup() {

    Serial.begin(115200);
    while (!Serial); // wait for serial connections

    Serial.println(F("\n\n === Starting YaRB demo === \n\n\n"));
    
    // Show the theoretical maximum ring buffer size for this platform
    // The practical limit is much smaller.
    Serial.println(F("Theoretical maximum capacity of YaRB on this platform:"));
    // We can use the limit() function on the class, as it is static...
    Serial.print(F("YaRB::limit():   "));
    Serial.println(YaRB::limit(), DEC);

    // ...or we can call it on an instance:
    Serial.print(F("ringbuf.limit(): "));
    Serial.println(ringbuf.limit(), DEC);

    // show current properties of ringbuf
    Serial.println(F("Status of initial ring buffer:"));
    print_rb_properties(ringbuf);

    // now add some bytes (values 0..255) to the ring buffer
    Serial.println(F("Adding some values one-by-one..."));
    Serial.println(F("ringbuf.put(100)"));
    ringbuf.put(100);
    Serial.println(F("ringbuf.put(101)"));
    ringbuf.put(101);
    Serial.println(F("ringbuf.put(102)"));
    ringbuf.put(102);

    // show current properties of ringbuf
    Serial.println(F("Status of ring buffer after adding some elements:"));
    print_rb_properties(ringbuf);

    // look at the first element without removing it
    // Note that we use the "address-of" operator here to give peek() a pointer to x
    Serial.println(F("peek()-ing at the next element in ring buffer..."));
    ringbuf.peek(&x);
    Serial.print(F("The next element we can get from the the ring buffer is "));
    Serial.println(x, DEC);

    // show current properties of ringbuf
    Serial.println(F("Status of ring buffer after peek() is unchanged:"));
    print_rb_properties(ringbuf);

    // get the next element from the ring buffer
    // Note use of pointer again
    Serial.println(F("Get a single element back from ring buffer (ringbuf.get(&y))"));
    ringbuf.get(&y);
    Serial.print(F("y was removed from ring buffer using get(): "));
    Serial.println(y, DEC);

    // show current properties of ringbuf
    Serial.println(F("Status of ring buffer after get(). Ring buffer has changed:"));
    print_rb_properties(ringbuf);

    // demonstrate return value of get()
    // get() returns false when there is nothing to get, i.e. when ringbuffer is empty
    Serial.println(F("Use return value of get() in a while() loop."));
    Serial.println(F("Excecute get(&x) until ring buffer is empty..."));
    while (ringbuf.get(&x)) {
        Serial.print(F("  Call to get() successful. x = "));
        Serial.println(x, DEC);
    }
    // show current properties of ringbuf
    Serial.println(F("Status of ring buffer after the loop shows empty buffer:"));
    print_rb_properties(ringbuf);

    // allocate some array to hold new data for the ring buffer
    uint8_t a[2*RB_CAPACITY];
    // initialize a: 200, 201, ...
    for (size_t i=0; i<sizeof(a); i++) {
        a[i] = (uint8_t)(200 + i);
    }
    size_t nbr = RB_CAPACITY / 3; 
    size_t retval = 0;
    
    Serial.println(F("Add some elements to ringbuf in one go:"));
    // Note: a[] decays to a pointer here
    retval = ringbuf.put(a, nbr);
    if (retval) Serial.println(F("adding to ring buffer was successful"));
    else         Serial.println(F("Could not add elements to ring buffer."));
    Serial.println(F("Status of ring buffer after adding some content from a[]:"));
    print_rb_properties(ringbuf);

    Serial.println(F("Add some more elements to ringbuf in one go:"));
    // Note: Start somewhere in the middle of the buffer, use "address-of" operator on array element
    retval = ringbuf.put(&a[nbr], nbr);
    if (retval) Serial.println(F("adding to ring buffer was successful"));
    else         Serial.println(F("Could not add elements to ring buffer."));
    Serial.println(F("Status of ring buffer after adding some content from a[]:"));
    print_rb_properties(ringbuf);

    Serial.println(F("Add some more elements to ringbuf in one go:"));
    // Note: Start somewhere in the middle of the buffer, using yet another notation (pointer arithmetic)
    retval = ringbuf.put(a+nbr, 2*nbr); // not enough room in ring buffer for another 2*nbr elements! returns false
    if (retval) Serial.println(F("adding to ring buffer was successful"));
    else         Serial.println(F("Could not add elements to ring buffer."));
    Serial.println(F("Status of ring buffer after adding some content from a[] with return value of 'false' (--> unchanged!):"));
    print_rb_properties(ringbuf);
    // Note that ring buffer is unchanged --> put() is an "all-or-nothing" transformation

    // Now get some elements back from ring buffer and write them to an array
    uint8_t b[2*RB_CAPACITY];
    Serial.println(F("Get some elements back from ring buffer and write them to an array 'b[]'"));
    retval = ringbuf.get(b, 3*nbr/2);
    if (retval) {
        Serial.println(F("returning some elements from ring buffer was successful"));
        for (size_t i=0; i<(3*nbr/2); i++) {
            Serial.print(F("  b["));
            Serial.print(i, DEC);
            Serial.print(F("]: "));
            Serial.println(b[i], DEC);
        }
    }
    else {
        Serial.println(F("Could not get elements from ring buffer. Ring buffer not changed."));   
    }
    Serial.println(F("Status of ring buffer after removing some content:"));
    print_rb_properties(ringbuf);    
    
    // now try to remove more elements than are left in the ring buffer
    Serial.print(F("Trying to remove "));
    Serial.print(nbr, DEC);
    Serial.println(F(" more elements from ring buffer."));
    retval = ringbuf.get(b, nbr);
    Serial.print(F("received "));
    Serial.print(retval, DEC);
    Serial.println(F(" elements from ring buffer."));
    if (retval) {
        Serial.println(F("returning some elements from ring buffer was successful"));
        for (size_t i=0; i<(retval); i++) {
            Serial.print(F("b["));
            Serial.print(i, DEC);
            Serial.print(F("]: "));
            Serial.println(b[i], DEC);
        }
    }
    else {
        Serial.println(F("Could not get elements from ring buffer. Ring buffer not changed."));   
    }
    Serial.println(F("Status of ring buffer after (trying to) remove some more content:"));
    print_rb_properties(ringbuf);

    // discard the next two elements
    Serial.println(F("Trying to discard the next 2 elements..."));
    x = ringbuf.discard(2);
    Serial.print(F("Did discard "));
    Serial.print(x, DEC);
    Serial.println(F(" elements from ring buffer"));
    Serial.println(F("Status of ring buffer after discarding 2 elements:"));
    print_rb_properties(ringbuf);

    // try to discard more elements than are in the buffer
    Serial.println(F("Trying to discard the next 10 elements..."));
    x = ringbuf.discard(10);
    Serial.print(F("Did discard "));
    Serial.print(x, DEC);
    Serial.println(F(" elements from ring buffer"));
    Serial.println(F("Status of ring buffer after trying to discard 10 additional elements:"));
    print_rb_properties(ringbuf);
    Serial.println(F("Ring buffer becomes empty when trying to discard more elements than are currently stored."));

    // add some elements back in
    Serial.println(F("Add back some elements..."));
    Serial.println(F("ringbuf.put(50)"));
    ringbuf.put(50);
    Serial.println(F("ringbuf.put(55)"));
    ringbuf.put(55);
    Serial.println(F("ringbuf.put(53)"));
    ringbuf.put(53);
    Serial.println(F("Current status of ring buffer after adding some elements:"));
    print_rb_properties(ringbuf);
    Serial.println(F("Now flush the ring buffer:"));
    ringbuf.flush();
    Serial.println(F("Status of ring buffer after flush():"));
    print_rb_properties(ringbuf);
    Serial.println(F("Ring buffer is empty after flush()."));

   // end of demo

} // end of setup()

void loop() {
} // end of loop()

// helper function to conveniently print the current properties of a ring buffer
void print_rb_properties(const IYaRB &rb) {
    Serial.print(F("  capacity(): ")); Serial.println( rb.capacity() );
    Serial.print(F("  size():     ")); Serial.println( rb.size() );
    Serial.print(F("  free():     ")); Serial.println( rb.free() );
    Serial.print(F("  isEmpty():  "));
    if (rb.isEmpty()) Serial.println(F("true"));
    else              Serial.println(F("false"));
    Serial.print(F("  isFull():   "));
    if (rb.isFull()) Serial.println(F("true"));
    else             Serial.println(F("false"));
    Serial.println();
}
