#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <tuple>
#include <vector>

namespace {

class Vector
{
    std::vector<std::tuple<std::string, std::size_t, std::size_t>> m_lines;

    const std::size_t field1;
    const std::size_t field2;
    const char separator;

    struct Comparator
    {
        bool operator()(std::tuple<std::string, std::size_t, std::size_t> a,
                        std::tuple<std::string, std::size_t, std::size_t> b)
        {
            int comparison = 0;
            if (std::get<0>(a).size() > std::get<1>(a) && std::get<0>(b).size() > std::get<1>(b)) {
                comparison = std::get<0>(a).compare(std::get<1>(a), std::get<2>(a), std::get<0>(b), std::get<1>(b), std::get<2>(b));
            }
            if (comparison == 0) {
                return (std::get<0>(a).compare(std::get<0>(b)) < 0);
            }
            return (comparison < 0);
        }
    };

public:
    Vector(const std::size_t size, const std::size_t field1, const std::size_t field2, const char separator)
        : field1(field1 - 1)
        , field2(field2)
        , separator(separator)
    {
        m_lines.reserve(size / 100);
    }

    void add(const std::string & line)
    {
        std::size_t first, last;
        std::size_t cnt = 0;
        auto current = line.begin();

        while (cnt < field1 && current != line.end()) {
            current = std::find(current, line.end(), separator);
            ++cnt;
            if (current != line.end()) {
                ++current;
            }
        }

        first = current - line.begin();

        if (field1 < field2) {
            while (cnt < field2 && current != line.end()) {
                current = std::find(current, line.end(), separator);
                ++cnt;
                if (current != line.end()) {
                    ++current;
                }
            }
            last = current - line.begin();
        }
        else {
            last = line.size();
        }

        last -= first;

        m_lines.emplace_back(line, first, last);
    }

    void sort()
    {
        std::sort(m_lines.begin(), m_lines.end(), Comparator());
    }

    using value_type = std::tuple<std::string, std::size_t, std::size_t>;
    using const_iterator = std::vector<value_type>::const_iterator;

    const_iterator begin() const
    {
        return m_lines.begin();
    }

    const_iterator end() const
    {
        return m_lines.end();
    }
};

using Lines = Vector;

template <class C>
void print_out(std::ostream & strm, const C & c)
{
    for (auto & element : c) {
        strm << std::get<0>(element) << std::endl;
    }
}

void sort_stream(std::istream & input, const std::size_t start, const std::size_t last, const char separator, const std::size_t size = 0)
{
    Lines lines(size, start, last, separator);

    std::string line;
    while (std::getline(input, line)) {
        lines.add(line);
    }

    lines.sort();

    print_out(std::cout, lines);
}

std::size_t calculate_size(std::istream & input)
{
    input.seekg(0, std::ios_base::end);
    const auto end_pos = input.tellg();
    input.seekg(0);
    return end_pos;
}

} // anonymous namespace

int main(int argc, char ** argv)
{
    std::size_t first = 1;
    std::size_t last = 0;
    char arg_type = '-';
    char separator = ' ';
    const char * input_name = nullptr;

    for (int i = 1; i < argc; ++i) {
        const std::size_t len = std::strlen(argv[i]);
        if (len == 0) {
            continue;
        }
        if (argv[i][0] == '-') {
            if (std::strncmp(argv[i], "--key", 5) == 0) {
                arg_type = 'k';
                if (len == 5) {
                    ++i;
                }
                else {
                    strcpy(argv[i], argv[i] + 6);
                }
            }
            else if (std::strncmp(argv[i], "--field-separator", 17) == 0) {
                arg_type = 't';
                if (len == 17) {
                    ++i;
                }
                else {
                    strcpy(argv[i], argv[i] + 18);
                }
            }
            else {
                arg_type = argv[i][1];
                ++i;
            }
        }

        switch (arg_type) {
        case '-':
            input_name = argv[i];
            break;
        case 'k':
            char * endptr;
            first = std::strtoul(argv[i], &endptr, 10);
            if (*endptr != '\x00') {
                ++endptr;
                last = std::strtoul(endptr, &endptr, 10);
            }
            break;
        case 't':
            separator = argv[i][0];
            break;
        }
        arg_type = '-';
    }

    if (input_name != nullptr) {
        std::ifstream f(input_name);
        sort_stream(f, first, last, separator, calculate_size(f));
    }
    else {
        sort_stream(std::cin, first, last, separator);
    }
}
