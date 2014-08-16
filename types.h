#include <exception>
#include <type_traits>
#include <algorithm>
#include <typeinfo>
#include <string>
#include <cassert>

namespace cppei {

using namespace std;

class unbound : public std::exception {
    public:
        unbound() {}
        const char *what() const throw() { return "unbound variable"; }
};

class term_check {};

class term_t {
    public:
        term_t() : ptr(0) {}
        term_t(const term_t &o) : ptr(o.ptr ? o.ptr->clone() : 0) {}
        term_t(term_t &&o) : ptr(o.ptr) { o.ptr = 0; }
        template<class T> term_t(T &&o
                , typename enable_if<is_base_of<term_check, typename decay<T>::type>::value>::type* = 0)
            : ptr(new impl<typename decay<T>::type>(forward<T>(o))) {}
        term_t &operator=(const term_t &o) { delete ptr; ptr = o.ptr ? o.ptr->clone() : 0; return *this; }
        term_t &operator=(term_t &&o) { delete ptr; ptr = o.ptr; o.ptr = 0; return *this; }
        template<class T> term_t &operator=(T &&o) {
            typename enable_if<is_base_of<term_check, typename decay<T>::type>::value>::type *check;
            delete ptr;
            ptr = new impl<typename decay<T>::type>(forward<T>(o));
            return *this;
        }
        ~term_t() { delete ptr; }
        bool is_bound() const { return ptr != 0; }
        bool operator==(const term_t &t) const {
            if(!is_bound() || !t.is_bound()) throw unbound();
            if(ptr->type() != t.ptr->type()) {
                return false;
            } else {
                return ptr->operator==(t.ptr);
            }
        }
        template<class T> bool operator==(const T &t) const {
            typename enable_if<is_base_of<term_check, typename decay<T>::type>::value>::type *check;
            if(!is_bound()) throw unbound();
            impl<typename decay<T>::type> *p = dynamic_cast<impl<typename decay<T>::type>*>(ptr);
            if(p == 0) { // different types
                return false;
            } else {
                return p->val == t;
            }
        }
        template<class T> bool operator!=(const T &t) const { return !operator==(t); }
        template<class T> void clone(T &t
                , typename enable_if<is_base_of<term_check, typename decay<T>::type>::value>::type* = 0) const {
            if(!is_bound()) throw unbound();
            impl<typename decay<T>::type> *p = dynamic_cast<impl<typename decay<T>::type>*>(ptr);
            assert(p);
            t = p->val;
        }
        template<class T> void swap(T &t
                , typename enable_if<is_base_of<term_check, typename decay<T>::type>::value>::type* = 0) {
            if(!is_bound()) throw unbound();
            impl<typename decay<T>::type> *p = dynamic_cast<impl<typename decay<T>::type>*>(ptr);
            assert(p);
            std::swap(t, p->val);
        }
        
    private:
        struct placeholder {
            virtual ~placeholder() {}
            virtual const type_info &type() const = 0;
            virtual bool operator==(const placeholder *t) const = 0;
            virtual placeholder *clone() const = 0;
        };

        template<class T>
        struct impl : public placeholder {
            template<class T1> impl(T1 t) : val(t) {}
            const type_info &type() const { return typeid(T); }
            bool operator==(const placeholder *p) const { return val == dynamic_cast<const impl<T>*>(p)->val; }
            impl *clone() const { return new impl(val); }
            T val;
        };
        placeholder *ptr;
};

template<class S>
class base_t : public term_check {
    public:
        base_t() : bound(false) {}
        base_t(const base_t &o) { operator=(o); }
        base_t(base_t &&o) { operator=(forward<base_t>(o)); }
        base_t(const term_t &o) { *this = o; }
        base_t(term_t &&o) { *this = move(o); }
        base_t &operator=(const base_t &o) { value = o.value; bound = o.bound; }
        base_t &operator=(base_t &&o) { value = move(o.value); bound = o.bound; o.bound = false; }
        base_t &operator=(const term_t &o) { o.clone(*this); return *this; }
        base_t &operator=(term_t &&o) { o.swap(*this); o = term_t(); return *this; }
        base_t(const S &t) : value(t), bound(true) {}
        base_t(S &&t) : value(forward<S>(t)), bound(true) {}
        base_t &operator=(const S &t) { value = t; bound = true; return *this; }
        base_t &operator=(S &&t) { value = forward<S>(t); bound = true; return *this; }
        bool operator==(const base_t &o) const { if(!bound || !o.bound) throw unbound(); return value == o.value; }
        bool operator==(const S &t) const { if(!bound) throw unbound(); return value == t; }
        bool operator!=(const S &t) const { return !operator==(t); }
        bool operator<(const base_t &o) const { if(!bound || !o.bound) throw unbound(); return value < o.value; }
        bool operator<(const S &t) const { if(!bound) throw unbound(); return value < t; }
        bool operator>(const S &t) const { return bound && value > t; }
        bool operator<=(const S &t) const { return operator<(t) || operator==(t); }
        bool operator>=(const S &t) const { return operator>(t) || operator==(t); }
        operator const S& () const { if(!bound) throw unbound(); return value; }
        void swap(S &s) { if(!bound) throw unbound(); std::swap(value, s); }
        bool is_bound() const { return bound; }
    private:
        S value;
        bool bound;
};

typedef base_t<string> atom_t;
typedef base_t<int> integer_t;
typedef base_t<double> floating_t;

}
