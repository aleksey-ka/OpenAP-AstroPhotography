#include "Tests.FirstTest.h"

Sample::Sample()
{

}

Sample::~Sample()
{

}

void Sample::test_case1()
{
    QVERIFY( 3 == 3 );
}

void Sample::test_case2()
{
    QVERIFY( 3 == 4 );
}

bool run( int argc, char* argv[] )
{
    Sample test;
    return QTest::qExec(&test, argc, argv);
}
