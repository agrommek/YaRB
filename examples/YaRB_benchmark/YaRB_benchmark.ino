/*
    YaRB benchmark

    This example sketch benchmarks the YaRB ring buffer library.
    Run it on your board and see what numbers you get.

    Some number from my tests:

    Arduino Pro Mini (ATmega 328p, 5V, 16 MHz):          27.7 us/byte put() and 27.7 us/byte get()
    Adafruit Feather M0 (SAMD21, 3.3V, 48 MHz):           1.6 us/byte put() and  1.7 us/byte get()
    Adafruit Feather M4 Express (SAMD51, 3.3V, 120 MHz): 0.25 us/byte put() and 0.25 us/byte get()
    
    This example code is in the public domain.
*/

#include <yarb.h>

constexpr size_t some_power_of_two = 256; // 2^8
constexpr size_t some_non_power_of_two = 257;

// Ring Buffer with capacity of some power-of-two
//YaRB rb_power2(some_power_of_two);
YaRBt<some_power_of_two> rb_power2;
//YaRB2 rb_power2(some_power_of_two);
//YaRB2t<some_power_of_two> rb_power2;

// Ring Buffer with capacity not power-of-two
//YaRB rb_generic(some_non_power_of_two);
YaRBt<some_non_power_of_two> rb_generic;
//YaRB2 rb_generic(some_non_power_of_two);
//YaRB2t<some_non_power_of_two> rb_generic;

size_t ITER = 32;

void setup() {

    Serial.begin(115200);
    while (!Serial);

    Serial.println(F("\n\nStarting benchmarking sketch for YaRB\n"));
    
    Serial.println(F("Benchmarking writes with power-of-two sized ring buffer..."));
    benchmark_writes(rb_power2, ITER);

    Serial.println(F("Benchmarking writes with non-power-of-two sized ring buffer..."));
    benchmark_writes(rb_generic, ITER);

    Serial.println(F("Benchmarking reads with power-of-two sized ring buffer..."));
    benchmark_reads(rb_power2, ITER);

    Serial.println(F("Benchmarking reads with non-power-of-two sized ring buffer..."));
    benchmark_reads(rb_generic, ITER);

    Serial.println(F("YaRB benchmark finished."));
    
} // end of setup()

void loop() {
}

void benchmark_writes(IYaRB &rb, size_t iterations) {
    Serial.println(F("Starting write benchmark..."));

    // declare source array and fill it with random data
    uint8_t src_array[rb.capacity()];
    for (size_t i=0; i<sizeof(src_array); i++) {
        src_array[i] = static_cast<uint8_t>(random(0, 256));
    }
    
    uint32_t start, stop, diff;
    diff = 0;
    for (size_t i=0; i<iterations; i++) {
        start = micros();
        rb.put(src_array, rb.capacity(), false);
        stop = micros();
        diff += stop - start;
        rb.flush();
    }

    Serial.print(F("Adding "));
    Serial.print(rb.capacity()*iterations, DEC);
    Serial.print(F(" bytes to ring buffer took "));
    Serial.print(diff, DEC);
    Serial.print(F(" microseconds."));
    Serial.println();

    float performance = static_cast<float>(diff) / (rb.capacity() * iterations);
    Serial.print(F("This is "));
    Serial.print(performance, 3);
    Serial.print(F(" microseconds per byte."));
    Serial.println();

    Serial.println();       
}

void benchmark_reads(IYaRB &rb, size_t iterations) {
    Serial.println(F("Starting read benchmark..."));

    uint8_t dst_array[rb.capacity()];
    uint32_t start, stop, diff;
    diff = 0;
    size_t sum = 0;

    for (size_t i=0; i<iterations; i++) {
        // fill ring buffer with random data
        while ( rb.put( (uint8_t)random(0, 256) ));

        // benchmark a complete read
        start = micros();
        rb.get(dst_array, rb.capacity());
        stop = micros();
        diff += stop - start;

        // do something with data in dst_array to prevent compiler optimizations
        for (size_t i=0; i<sizeof(dst_array); i++) sum += dst_array[i];
    }
    rb.put(sum);
    rb.flush();

    Serial.print(F("Reading "));
    Serial.print(rb.capacity() * iterations, DEC);
    Serial.print(F(" bytes from ring buffer took "));
    Serial.print(diff, DEC);
    Serial.print(F(" microseconds."));
    Serial.println();

    float performance = static_cast<float>(diff) / (rb.capacity() * iterations);
    Serial.print(F("This is "));
    Serial.print(performance, 3);
    Serial.print(F(" microseconds per byte."));
    Serial.println();

    Serial.println();       
}
