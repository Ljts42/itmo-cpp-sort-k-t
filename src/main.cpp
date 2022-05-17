#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <string_view>
#include <vector>

namespace {

struct Comparator
{
    bool operator()(const std::vector<std::string_view> & a,
                    const std::vector<std::string_view> & b) const
    {
        for (std::size_t i = 1; i < std::min(a.size(), b.size()); ++i) {
            if(int comp = a[i].compare(b[i]); comp != 0) {
                return comp < 0;
            }
        }
        return (a[0].compare(b[0]) < 0);
    }
};

class VectorView
{
    std::vector<char> m_data;
    std::vector<std::vector<std::string_view>> m_lines;

    const std::size_t start_pos;
    const std::size_t end_pos;
    const char separator;

    std::size_t find(const std::string & line, std::size_t current)
    {
        while (current < line.size()) {
            if ((separator == '\x00' && isspace(line[current])) || line[current] == separator) {
                return current + 1;
            }
            ++current;
        }
        return current;
    }

public:
    VectorView(const std::size_t size, const std::size_t start_pos, const std::size_t end_pos, const char separator)
        : start_pos(start_pos - 1)
        , end_pos(end_pos)
        , separator(separator)
    {
        m_data.reserve(size);
        m_lines.reserve(size / 100);
    }

    void add(const std::string & line)
    {
        const std::size_t old_size = m_data.size();
        m_data.resize(old_size + line.size());
        std::memmove(&m_data[old_size], line.c_str(), line.size());

        m_lines.push_back(std::vector<std::string_view>());
        m_lines.back().emplace_back(&m_data[old_size], line.size());

        if (line.size() < start_pos) {
            return;
        }

        std::size_t cnt = 0;
        std::size_t first = 0;
        std::size_t last = find(line, first);

        while (cnt < start_pos && last < line.size()) {
            first = last;
            last = find(line, last);
            if (last - first != 1 || last == line.size()) {
                ++cnt;
            }
        }

        if (cnt < start_pos) {
            return;
        }

        while (cnt < end_pos && last < line.size()) {
            m_lines.back().emplace_back(&m_data[old_size + first], last - first - 1);

            first = last;
            last = find(line, last);
            if (last - first != 1 || last == line.size()) {
                ++cnt;
            }
        }

        if (last == line.size() && cnt < end_pos) {
            m_lines.back().emplace_back(&m_data[old_size + first], last - first);
        }
    }

    void sort()
    {
        std::sort(m_lines.begin(), m_lines.end(), Comparator());
    }

    using value_type = std::vector<std::string_view>;
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

using Lines = VectorView;

template <class C>
void print_out(std::ostream & strm, const C & c)
{
    for (auto & element : c) {
        strm << element[0] << std::endl;
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
    std::size_t start_pos = 0;
    std::size_t end_pos = -1;
    char separator = '\x00';

    char arg_type = '-';
    std::vector<char *> input_names;

    for (int i = 1; i < argc; ++i) {
        const std::size_t len = std::strlen(argv[i]);
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
            else if (len > 1 && (argv[i][1] == 'k' || argv[i][1] == 't')) {
                arg_type = argv[i][1];
                if (len == 2) {
                    ++i;
                }
                else {
                    strcpy(argv[i], argv[i] + 3);
                }
            }
        }

        switch (arg_type) {
        case '-':
            input_names.push_back(argv[i]);
            break;
        case 'k':
            char * endptr;
            start_pos = std::strtoul(argv[i], &endptr, 10);
            if (*endptr != '\x00') {
                ++endptr;
                end_pos = std::strtoul(endptr, &endptr, 10);
            }
            break;
        case 't':
            separator = argv[i][0];
            break;
        }
        arg_type = '-';
    }

    for (const auto name : input_names) {
        if (*name != '-') {
            std::ifstream f(name);
            sort_stream(f, start_pos, end_pos, separator, calculate_size(f));
        }
        else {
            sort_stream(std::cin, start_pos, end_pos, separator);
        }
    }
    if (input_names.empty()) {
        sort_stream(std::cin, start_pos, end_pos, separator);
    }
}
