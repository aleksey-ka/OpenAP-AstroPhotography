#ifndef SAMPLE_H
#define SAMPLE_H

#include <QtTest>

class Sample : public QObject
{
    Q_OBJECT

public:
    Sample();
    ~Sample();

private slots:
    void test_case1();
    void test_case2();
};

#endif // SAMPLE_H
