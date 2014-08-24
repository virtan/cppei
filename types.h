#include <exception>
#include <type_traits>
#include <algorithm>
#include <typeinfo>
#include <string>
#include <cassert>
#include <deque>

namespace cppei {

using namespace std;

template<const char Desc[]>
class local_exception : public std::exception {
    public:
        local_exception() {}
        const char *what() const throw() { return Desc; }
};

constexpr char le_unbound[] = "unbound variable";
typedef local_exception<le_unbound> unbound;
constexpr char le_incompatible[] = "incompatible type";
typedef local_exception<le_incompatible> incompatible;

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
        term_t &operator=(term_t &o) { delete ptr; ptr = o.ptr ? o.ptr->clone() : 0; return *this; }
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
        template<class T> T &cast() {
            typename enable_if<is_base_of<term_check, typename decay<T>::type>::value>::type *check;
            if(!is_bound()) throw unbound();
            impl<typename decay<T>::type> *p = dynamic_cast<impl<typename decay<T>::type>*>(ptr);
            if(p == 0) { // different types
                throw incompatible();
            } else {
                return forward<T&>(p->val);
            }
        }
        template<class T> operator T () { return cast<T>(); }
        
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

template<class S, int variant = 0>
class base_t : public term_check {
    public:
        // types
        enum empty_t { empty };
        // constructors
        base_t() : bound(false) {}
        base_t(empty_t) : value(S()), bound(true) {}
        base_t(const base_t &o) { operator=(o); }
        base_t(base_t &&o) { operator=(forward<base_t>(o)); }
        base_t(const term_t &o) { *this = o; }
        base_t(term_t &&o) { *this = move(o); }
        base_t(const S &t) : value(t), bound(true) {}
        base_t(S &&t) : value(forward<S>(t)), bound(true) {}
        // assign operators
        base_t &operator=(const base_t &o) { value = o.value; bound = o.bound; }
        base_t &operator=(base_t &&o) { value = move(o.value); bound = o.bound; o.bound = false; }
        base_t &operator=(const term_t &o) { o.clone(*this); return *this; }
        base_t &operator=(term_t &&o) { o.swap(*this); o = term_t(); return *this; }
        base_t &operator=(const S &t) { value = t; bound = true; return *this; }
        base_t &operator=(S &&t) { value = forward<S>(t); bound = true; return *this; }
        // comparison operators
        bool operator==(const base_t &o) const {
            if(!bound || !o.bound) throw unbound();
            return value == o.value;
        }
        bool operator!=(const base_t &o) const { return !operator==(o); }
        bool operator==(const S &t) const { if(!bound) throw unbound(); return value == t; }
        bool operator!=(const S &t) const { return !operator==(t); }
        bool operator<(const base_t &o) const { if(!bound || !o.bound) throw unbound(); return value < o.value; }
        bool operator<(const S &t) const { if(!bound) throw unbound(); return value < t; }
        bool operator>(const S &t) const { return bound && value > t; }
        bool operator<=(const S &t) const { return operator<(t) || operator==(t); }
        bool operator>=(const S &t) const { return operator>(t) || operator==(t); }
        // casting
        operator const S& () const { if(!bound) throw unbound(); return value; }
        // swap and is_bound
        void swap(base_t &o) { std::swap(bound, o.bound); std::swap(value, o.value); }
        void swap(S &s) { if(!bound) throw unbound(); std::swap(value, s); }
        bool is_bound() const { return bound; }

    protected:
        S value;
        bool bound;
};

template<int variant>
class base_t<deque<term_t>, variant> : public term_check {
    public:
        // types
        enum empty_t { empty };
        typedef deque<term_t>::iterator iterator;
        typedef deque<term_t>::const_iterator const_iterator;
        // constructors
        base_t() : bound(false) {}
        base_t(empty_t) : bound(true) {}
        base_t(const base_t &o) { operator=(o); }
        base_t(base_t &&o) { operator=(forward<base_t>(o)); }
        base_t(const term_t &o) { *this = o; }
        base_t(term_t &&o) { *this = move(o); }
        template<class... Types>
        base_t(Types... args...) {
            bound = true;
            [](...){}((value.push_front(args),1)...);
        }
        // assign operators
        base_t &operator=(const base_t &o) { value = o.value; bound = o.bound; }
        base_t &operator=(base_t &&o) { value = move(o.value); bound = o.bound; o.bound = false; }
        base_t &operator=(const term_t &o) { o.clone(*this); return *this; }
        base_t &operator=(term_t &&o) { o.swap(*this); o = term_t(); return *this; }
        // comparison operators
        bool operator==(const base_t &o) const {
            if(!bound || !o.bound) throw unbound();
            return value == o.value;
        }
        bool operator!=(const base_t &o) const { return !operator==(o); }
        // swap and is_bound
        void swap(base_t &o) { std::swap(bound, o.bound); std::swap(value, o.value); }
        bool is_bound() const { return bound; }
        // container-related methods
        iterator begin() { return value.begin(); }
        iterator end() { return value.end(); }
        const_iterator begin() const { return value.begin(); }
        const_iterator end() const { return value.end(); }
        size_t size() const { if(!bound) throw unbound(); return value.size(); }
        void push_front(const term_t &t) { bound = true; value.push_front(t); }
        void push_front(term_t &&t) { bound = true; value.emplace_front(t); }
        void push_back(const term_t &t) { bound = true; value.push_back(t); }
        void push_back(term_t &&t) { bound = true; value.emplace_back(t); }
        void pop_front() { if(!bound) throw unbound(); value.pop_front(); }
        void pop_back() { if(!bound) throw unbound(); value.pop_back(); }
        term_t &front() { if(!bound) throw unbound(); return value.front(); }
        const term_t &front() const { if(!bound) throw unbound(); return value.front(); }
        term_t &back() { if(!bound) throw unbound(); return value.back(); }
        const term_t &back() const { if(!bound) throw unbound(); return value.back(); }

    protected:
        deque<term_t> value;
        bool bound;
};

typedef base_t<string> atom_t;
typedef base_t<int> integer_t;
typedef base_t<double> floating_t;
typedef base_t<string> binary_t;

typedef base_t<deque<term_t>, 0> tuple_t;
typedef base_t<deque<term_t>, 1> list_t;

}
