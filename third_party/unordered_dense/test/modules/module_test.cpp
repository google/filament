import ankerl.unordered_dense;

#include <cassert>
#include <string>

int main() {
    ankerl::unordered_dense::map<std::string, int> m;
    m["24535"] = 4;
    assert(m.size() == 1);

    auto h_int = ankerl::unordered_dense::hash<int>();
    assert(h_int(123) != 123);

    auto h_str = ankerl::unordered_dense::hash<std::string>();
    assert(h_str("123") != 123);

    auto h_ptr = ankerl::unordered_dense::hash<int*>();
    int i = 0;
    assert(h_ptr(&i) != 0);
}
