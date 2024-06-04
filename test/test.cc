#include <iostream>
#include <stdio.h>
#include <limits>
#include <string>
#include <memory>
#include <map>
#include <vector>
#include <algorithm>
using namespace std;

class A {
public:
    void func() {
        cout << "A::func()" << endl;
        func1();
    }
    virtual void func1() = 0;
};

void A::func1() {
    cout << "A::func1()" << endl;
}

class B : public A{
public:
    void func(int a) {
        cout << "B::func(int)" << endl;
    }
    void func1() {
        cout << "B::func1()" << endl;
    }
};

int main() {

    B b;

    b.func(1);

    return 0;
}



// g++ test.cc