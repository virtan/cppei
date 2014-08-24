#include <iostream>
#include <assert.h>
#include "types.h"

using namespace cppei;
using namespace std;

int main() {
    cout << "base_t base type constructor\n";
    atom_t a1("hello_world");

    cout << "base_t operator ==\n";
    assert(a1 == "hello_world");

    cout << "base_t operator == (to base type)\n";
    assert(a1 == string("hello_world"));

    cout << "base_t copy constructor\n";
    atom_t a2(a1);
    assert(a1 == "hello_world");
    assert(a2 == "hello_world");

    cout << "base_t operator == (same type)\n";
    assert(a1 == a2);

    cout << "base_t move constructor\n";
    atom_t a3(move(a2));
    assert(a1 == "hello_world");
    assert(!a2.is_bound());
    assert(a3 == "hello_world");

    cout << "base_t move from string\n";
    string s("appp");
    assert(s == "appp");
    atom_t a35(move(s));
    assert(a35 == "appp");
    assert(s != "appp");

    cout << "base_t default constructor\n";
    atom_t a4;

    cout << "base_t operator =\n";
    a4 = a3;
    assert(a4 == "hello_world");
    assert(a3 == "hello_world");
    assert(a3 == a4);

    cout << "base_t move constructor 2\n";
    atom_t a5;
    a5 = move(a4);
    assert(a5 == "hello_world");
    assert(!a4.is_bound());
    try {
        assert(a4 != a5);
        assert(false);
    } catch(std::exception &) {
    }

    cout << "base_t pod-based type\n";
    floating_t f1(5.3);
    assert(f1 == 5.3);
    assert(f1 < 5.4);
    assert(f1 >= 5.2);
    floating_t f2(f1);
    assert(f2 == f1);
    assert(f2 == 5.3);
    floating_t f3(move(f2));
    assert(f3 == 5.3);
    floating_t f4;
    f4 = f3;
    assert(f4 == 5.3);
    floating_t f5;
    f5 = move(f4);
    assert(f5 == 5.3);

    cout << "base_t not assigned\n";
    integer_t i6;
    try {
        i6 == 13;
        assert(false);
    } catch(std::exception &) {
    }

    {
    cout << "term_t default constructor\n";
    term_t t1;

    cout << "term_t operator =(atom_t)\n";
    t1 = atom_t("oppa");
    assert(t1 == atom_t("oppa"));
    assert(t1 != floating_t(12));

    cout << "term_t atom_t constructor\n";
    term_t t2(atom_t("oppa"));
    assert(t1 == t2);

    cout << "term_t copy constructor\n";
    term_t t3(t2);
    assert(t3 == atom_t("oppa"));
    assert(t2 == atom_t("oppa"));
    assert(t3 == t2);

    cout << "term_t move constructor\n";
    term_t t4(move(t3));
    assert(t4 == t2);
    assert(t4 == atom_t("oppa"));
    assert(!t3.is_bound());
    try {
        assert(t4 != t3);
        assert(false);
    } catch(std::exception &) {
    }

    cout << "term_t multiple assignment\n";
    atom_t a6("ggg");
    assert(a6.is_bound());
    term_t t5(move(a6));
    assert(t5 == atom_t("ggg"));
    assert(t5.is_bound());
    assert(!a6.is_bound());
    floating_t a7(3.5);
    t5 = a7;
    assert(t5 != atom_t("ggg"));
    assert(t5 == floating_t(3.5));
    assert(a7.is_bound());
    integer_t a8(11);
    t5 = move(a8);
    assert(t5 != floating_t(3.5));
    assert(t5 == integer_t(11));
    assert(!a8.is_bound());

    }

    {
    cout << "base_t from term_t copy\n";
    term_t t1(atom_t("some string"));
    atom_t a1;
    assert(t1.is_bound());
    assert(!a1.is_bound());
    a1 = t1;
    assert(t1.is_bound());
    assert(a1.is_bound());
    assert(a1 == "some string");

    cout << "base_t from term_t move\n";
    term_t t2(atom_t("some string 2"));
    atom_t a2;
    assert(t2.is_bound());
    assert(!a2.is_bound());
    a2 = move(t2);
    assert(!t2.is_bound());
    assert(a2.is_bound());
    assert(a2 == "some string 2");
    }

    {
    cout << "tuple_t operations\n";
    term_t t = tuple_t(atom_t("hello"), binary_t("world"));
    assert(t.cast<tuple_t>().is_bound());
    cout << "tuple_t size\n";
    assert(t.cast<tuple_t>().size() == 2);
    cout << "tuple_t front access\n";
    assert(t.cast<tuple_t>().front().is_bound());
    cout << "tuple_t front comparison\n";
    assert(t.cast<tuple_t>().front() == atom_t("hello"));

    cout << "list_t operations\n";
    term_t t2 = list_t(atom_t("hello"), binary_t("world"));
    assert(t2.cast<list_t>().is_bound());
    cout << "list_t size\n";
    assert(t2.cast<list_t>().size() == 2);
    cout << "list_t front access\n";
    assert(t2.cast<list_t>().front().is_bound());
    cout << "list_t front comparison\n";
    assert(t2.cast<list_t>().front() == atom_t("hello"));

    cout << "tuple_t and list_t are different types\n";
    assert(t != t2);
    }

    {
    cout << "matching\n";
    term_t t = tuple_t(atom_t("hello"), tuple_t(atom_t("hello"), binary_t("world")));
    atom_t a("hello"); binary_t b;
    assert(a.is_bound());
    assert(!b.is_bound());
    //assert((tuple_t(a, tuple_t(a, b)) = t) == true);
    assert(a == atom_t("hello"));
    assert(b.is_bound());
    assert(b == binary_t("world"));
    }


    /*
    atom_t a2(a1);
    atom_t a3;
    atom_t a4(a3);
    atom_t a5("other");

    floating_t f1(3.12);
    floating_t f2(f1);
    floating_t f3;
    floating_t f4(f3);

    cout << (a1 == a2 ? "equal" : "differ") << "\n";

    term_t t;
    t = f1;
    t = a1;
    a3 = t;
    cout << "hm\n";
    t = a5;
    cout << (t == a5 ? "true" : "false") << endl;
    cout << (t == f2 ? "true" : "false") << endl;
    cout << "place1\n";
    {
        term_t t;
        t = a1;
        a3 = t;
        a3 = t;
        cout << (t == a1 ? "true" : "false") << endl;
        cout << (a3 == t ? "true" : "false") << endl;
        cout << (t == f2 ? "true" : "false") << endl;
        cout << (t == t ? "true" : "false") << endl;
    }
    cout << (string&) a3 << endl;
    cout << (string&) a1 << endl;
    cout << "place2\n";
    */
}
