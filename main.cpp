#include <QFlags>
#include <QVarLengthArray>
#include <QDebug>

enum Option {
    NoOptions = 0x0,
    ShowTabs = 0x1,
    ShowAll = 0x2,
    SqueezeBlank = 0x4,
    PreLast = 1 << 30,
    Last = 1 << 31
};
Q_DECLARE_FLAGS(Options, Option)
//Q_DECLARE_OPERATORS_FOR_FLAGS(Options)

enum MyEnum {bla=1 << 31};
//Q_DECLARE_FLAGS(MyEnums, MyEnum)


void shift(/*unsigned */int f = -1) {
    do qDebug() << f;
    while (f <<= 1);
}

template <typename T>
class qtvFlags : public QFlags<T>
{
public:
    qtvFlags(const QFlags<T> & other) : QFlags<T>(other) {qDebug("*this: %d", 1u > *this);}
    qtvFlags(T e) : QFlags<T>(e) {}
    qtvFlags(QFlag value) : QFlags<T>(value) {}
    qtvFlags() /*: QFlags() */{}

    const T next(unsigned int m) const {
        if (!m) m = 1;
        else {
            qDebug() << "   mask:" << (m << 1);
//            if ( (m <<= 1) > *this || !m ) return (T)0;
            m <<= 1;
        }
        if (m > *this || !m) return (T)0;   // check !m in case max enum flag set or *this==0
                                            // is it better to move !m check here also? done..
                                            // consider: 'm > *this | !m' to eliminate branching

        while ( !( *this & m ) ) m <<= 1;
        return (T)m;
    }

    class const_iterator : public std::iterator<std::forward_iterator_tag, T> {
        const qtvFlags<T>* /*const */i;     // const avoiding gives us copy assignable class by default
        unsigned int m;

    public:
        inline const_iterator(const qtvFlags<T>* const p) : i(p), m(0) {}
//        inline const_iterator(/*const */const_iterator&) : i() {} // is_copy_constructible: false

        // why only one of these always called?
        inline const T& operator*() const { /*qDebug("rvalue");*/ return static_cast<const T>(m); }
//        inline T& operator*() { qDebug("lvalue"); return reinterpret_cast<T&>(m); } // needed?

        inline bool operator==(const const_iterator &o) const { return m == o.m /*&& i == o.i*/; }
        inline bool operator!=(const const_iterator &o) const { return m != o.m /*|| i != o.i*/; }
        inline const_iterator& operator++() { m = i->next(m); return *this;}
    };

    const_iterator begin() const { return ++const_iterator(this);}
    const_iterator end() const { return const_iterator(this); }
};
Q_DECLARE_OPERATORS_FOR_FLAGS(qtvFlags<Option>)


Options myOpt(bla);
qtvFlags<Option> flags
(ShowTabs
// |SqueezeBlank
 |PreLast
// |Last
 )
;


int main()
{
    myOpt|=NoOptions|ShowTabs;
    qDebug() << flags << myOpt << '\n'
             << "is_copy_constructible:" << std::is_copy_constructible<qtvFlags<Option>::const_iterator>::value << '\n'
             << "is_copy_assignable:" << std::is_copy_assignable<qtvFlags<Option>::const_iterator>::value << '\n'
             << "is_destructible:" << std::is_destructible<qtvFlags<Option>::const_iterator>::value << '\n'
                ;

    for ( Option n = flags.next(0); n; n = flags.next(n) )
        qDebug() << n << '\n';

    qDebug("\nforeach");
    foreach (Option flag, flags) qDebug() << "value:" << flag << '\n';

    /* Tests
     */
    qtvFlags<Option>::const_iterator i(&flags), i1(i);
    i1 = i;
    i1 == i;
    i1 != i;
    // both of these call non-const '*' operator (lvalue) if provided
    *i;
//    *i = (Option)5;
    ++i;
    // empty() test
}
