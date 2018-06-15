#include "csv.h"
#include "unity.h"

void test_CSVRecordInitDestroy(void) {
	csvrecord record = csvrecord_init();
	TEST_ASSERT_NOT_NULL(record);

	csvrecord_close(&record);
	TEST_ASSERT_NULL(record);
}

int main(void) {
	UNITY_BEGIN();

	RUN_TEST(test_CSVRecordInitDestroy);

	return UNITY_END();
}
