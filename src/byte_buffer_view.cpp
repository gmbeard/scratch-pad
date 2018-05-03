#include "byte_buffer_view.hpp"

#include <vector>
#include <algorithm>

auto const_conversion(ByteBufferView<uint8_t const>) {
    (void)0;
}

template<typename C>
auto const_conversion_iter(ByteBufferViewIterator<C, true>) {
    (void)0;
}

auto test() -> int {
    std::vector<unsigned char> v(10);
    ByteBufferView<uint8_t> view{v};
    std::cerr << "Size: " << view.size() << "\n";

    for (auto& i : view) {
        i = 42;
    }

    std::copy(std::begin(view), std::end(view),
              std::ostream_iterator<uint8_t>{std::cerr, "\n"});

    auto it = view.begin();
    ByteBufferViewIterator<decltype(view), true> it2 = it;
    const_conversion(view);
//    const_conversion_iter(it);

    return 0;
}
