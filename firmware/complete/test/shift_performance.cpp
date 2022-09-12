#include <unity.h>
#include <Arduino.h>

union t
{
  byte s[4];
  int  t[2];
  unsigned long x;
} volatile temp;

void setUp() {

}

void tearDown() {

}

void test_original_shift() {
  unsigned int start;
  temp.x = 0xAA555555;
  volatile byte y = 0;
  volatile t temp;
  Serial.println("====================");
  Serial.println("Original shift 0-31");
  Serial.println("====================");
  for (int a=0; a <32; a++)
  {
    start = millis();
    for (long  i=0; i< 100000L; i++)
    {
        y = temp.x >> a;
    }
    Serial.println(millis() - start);
  }
    TEST_ASSERT_TRUE(true);
}

void test_optimized_shift() {
    //https://forum.arduino.cc/t/bitshifting-problem-slows-down/66515/10
  unsigned int start;
  temp.x = 0xAA555555;
  volatile byte y = 0;
  volatile t temp;
  Serial.println("====================");
  Serial.println("Optimized shift 0-31");
  Serial.println("====================");
  for (int a=0; a <32; a++)
  {
    start = millis();
    for (long  i=0; i< 100000L; i++)
    {
      if (a > 23) y = temp.s[0] >> (a-24);
      else if (a > 15) y = temp.t[0] >> (a-16);
      else if (a > 7) 
      {
        temp.s[3] = temp.s[2];
        temp.s[2] = temp.s[1];
        temp.s[1] = temp.s[0];
        temp.s[0] = 0;
        y = temp.x >> (a-8);
      }
      else y = temp.x >> a;
    }
    Serial.println(millis() - start);
  }
    TEST_ASSERT_TRUE(true);
}

int main( int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_original_shift);
    RUN_TEST(test_optimized_shift);
    UNITY_END();
}