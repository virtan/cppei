CPPEI
=====

// under development

C++ bindings for EI (Erlang Interface)

Example:
```
using namespace std;
using namespace cppei;

int main() {
    cint >> length4;
    cout << length4;
    while(true) {
        term_t t;
        cin >> t;
        atom_t a("service_1");
        binary_t b;
        if(tuple(a, b) = t) {
            cout << tuple(atom_t("echo"), b);
        }

    }
}
```
