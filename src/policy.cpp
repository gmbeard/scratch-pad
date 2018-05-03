#include <iostream>
#include <vector>
#include <type_traits>
#include <iterator>
#include <forward_list>
#include <array>

template<typename Container>
constexpr bool IsContiguousBytes =
    std::is_convertible_v<
        typename std::iterator_traits<
            decltype(std::declval<Container>().begin())>::iterator_category,
        std::random_access_iterator_tag>
    &&
    std::is_convertible_v<
        std::remove_const_t<decltype(*std::declval<Container>().begin())>,
        uint8_t&>;

template<typename Container>
constexpr auto check_contiguous() {
    static_assert(
        IsContiguousBytes<Container>,
        "Container type must be ContiguousBytes"); 
}

#define CHECK_CONTIGUOUS(T) static_cast<void*>(check_contiguous<T>)

template<typename Parent>
struct Read {
    template<typename Container>
    auto read(Container& c) -> size_t {
        CHECK_CONTIGUOUS(Container);
        return write(*static_cast<Parent*>(this), c);
    }
};

template<typename Parent>
struct Write {
    template<typename Container>
    auto write(Container const& c) -> size_t {
        CHECK_CONTIGUOUS(Container);
        return read(*static_cast<Parent*>(this), c);
    }
};

struct A :  
    Read<A>
,   Write<A>
{
    template<typename Container>
    friend auto read(A& a, Container& c) -> size_t {
        return c.size();
    }
   
    template<typename Container>
    friend auto write(A& a, Container const& c) -> size_t {
        return c.size() / 2;
    }
};

int main(int, char const**) {
    A a{};

//    std::vector<uint8_t> v(10);
    std::forward_list<uint8_t> v;
//    std::array<uint8_t, 10> v;

    std::cerr << "Read : " << a.read(v) << "\n";
    std::cerr << "Write: " << a.write(v) << "\n";
    return 0;
}
