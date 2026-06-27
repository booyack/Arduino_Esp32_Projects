/* Minimal PlatformIO/Unity test to satisfy "Nothing to build" error */
#include <Arduino.h>
#include <unity.h>

void test_always_passes() {
    TEST_ASSERT_EQUAL_INT(1, 1);
}

void setup() {
    delay(2000); // allow serial to initialize on some boards
    UNITY_BEGIN();
    RUN_TEST(test_always_passes);
    UNITY_END();
}

void loop() {
    // empty
}
