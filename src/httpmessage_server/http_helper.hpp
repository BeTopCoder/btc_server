#ifndef __HTTP_HELPER_HPP__
#define __HTTP_HELPER_HPP__

#include <string>
#include <boost/tuple/tuple.hpp>
#include <vector>
#include <boost/cstdint.hpp>
#include <boost/algorithm/string.hpp>
#include <sstream>
#include <boost/logic/tribool.hpp>

#ifndef atoi64
# ifdef _MSC_VER
#  define atoi64 _atoi64
#  define strncasecmp _strnicmp
# else
#  define atoi64(x) strtoll(x, (char**)NULL, 10)
# endif
#endif // atoi64

namespace httpmessage_server
{
    struct header
    {
        std::string name;
        std::string value;
    };

    struct request
    {
        std::string method;
        std::string uri;
        int http_version_major;
        int http_version_minor;
        std::vector<header> headers;

        //只有在调用了normalise后才能访问的成员
        boost::uint64_t content_length;
        bool keep_alive;

        std::string body;

        std::string operator[](const std::string &name) const
        {
            for (const header &hdr : headers)
            {
                if (0 == strncasecmp(name.c_str(), hdr.name.c_str(), name.length()))
                {
                    return hdr.value;
                }
            }
            return "";
        }

        /* 将一些标准头部从headers提取出来 */
        /* 必要的小写化 */
        void normalise()
        {
            boost::to_lower(method);
            boost::to_lower(uri);
            for (header &hdr : headers)
            {
                boost::to_lower(hdr.name);
            }
            auto contentlength = (*this)["content-length"];
            if (!contentlength.empty())
            {
                std::istringstream contentlength_string;
                contentlength_string.str(contentlength);
                contentlength_string.imbue(std::locale("C"));
                contentlength_string >> content_length;
            }
            keep_alive = boost::to_lower_copy((*this)["connection"]) == "keep-alive";
        }
    };

    class request_parser
    {
    public:
        request_parser()
            : m_state_(method_start)
        {

        }

        void reset()
        {
            m_state_ = method_start;
        }

        template<typename InputIterator>
        boost::tuple<boost::tribool, InputIterator>parse(request &req, InputIterator begin, InputIterator end)
        {
            boost::tribool result = boost::indeterminate;
            while (begin != end)
            {
                result = consume(req, *(begin++));
                if (result || !result)
                {
                    return boost::make_tuple(result, begin);
                }
            }
            return boost::make_tuple(result, begin);
        }

    protected:


    private:
        boost::tribool consume(request &req, char input)
        {
            switch (m_state_)
            {
                case method_start:
                {
                    if (!is_char(input) || is_ctl(input) || is_tspecial(input))
                    {
                        return false;
                    }
                    else
                    {
                        m_state_ = method;
                        req.method.push_back(input);
                        return boost::indeterminate;
                    }
                }
                case method:
                {
                    if (input == ' ')
                    {
                        m_state_ = uri;
                        return boost::indeterminate;
                    }
                    else if (!is_char(input) || is_ctl(input) || is_tspecial(input))
                    {
                        return false;
                    }
                    else
                    {
                        req.method.push_back(input);
                        return boost::indeterminate;
                    }
                }
                case uri_start:
                {
                    if (is_ctl(input))
                    {
                        return false;
                    }
                    else
                    {
                        m_state_ = uri;
                        req.uri.push_back(input);
                        return boost::indeterminate;
                    }
                }
                case uri:
                {
                    if (input == ' ')
                    {
                        m_state_ = http_version_h;
                        return boost::indeterminate;
                    }
                    else if (is_ctl(input))
                    {
                        return false;
                    }
                    else
                    {
                        req.uri.push_back(input);
                        return boost::indeterminate;
                    }
                }
                case http_version_h:
                {
                    if (input == 'H')
                    {
                        m_state_ = http_version_t_1;
                        return boost::indeterminate;
                    }
                    else
                    {
                        return false;
                    }
                }
                case http_version_t_1:
                {
                    if (input == 'T')
                    {
                        m_state_ = http_version_t_2;
                        return boost::indeterminate;
                    }
                    else
                    {
                        return false;
                    }
                }
                case http_version_t_2:
                {
                    if (input == 'T')
                    {
                        m_state_ = http_version_p;
                        return boost::indeterminate;
                    }
                    else
                    {
                        return false;
                    }
                }
                case http_version_p:
                {
                    if (input == 'P')
                    {
                        m_state_ = http_version_slash;
                        return boost::indeterminate;
                    }
                    else
                    {
                        return false;
                    }
                }
                case http_version_slash:
                {
                    if (input == '/')
                    {
                        req.http_version_major = 0;
                        req.http_version_minor = 0;
                        m_state_ = http_version_major_start;
                        return boost::indeterminate;
                    }
                    else
                    {
                        return false;
                    }
                }
                case http_version_major_start:
                {
                    if (is_digit(input))
                    {
                        req.http_version_major = req.http_version_major * 10 + input - '0';
                        m_state_ = http_version_major;
                        return boost::indeterminate;
                    }
                    else
                    {
                        return false;
                    }
                }
                case http_version_major:
                {
                    if (input == '.')
                    {
                        m_state_ = http_version_minor_start;
                        return boost::indeterminate;
                    }
                    else if (is_digit(input))
                    {
                        req.http_version_major = req.http_version_major * 10 + input - '0';
                        return boost::indeterminate;
                    }
                    else
                    {
                        return false;
                    }
                }
                case http_version_minor_start:
                {
                    if (is_digit(input))
                    {
                        req.http_version_minor = req.http_version_minor * 10 + input - '0';
                        m_state_ = http_version_minor;
                        return boost::indeterminate;
                    }
                    else
                    {
                        return false;
                    }
                }
                case http_version_minor:
                {
                    if (input == '\r')
                    {
                        m_state_ = expecting_newline_1;
                        return boost::indeterminate;
                    }
                    else if (is_digit(input))
                    {
                        req.http_version_minor = req.http_version_minor * 10 + input - '0';
                        return boost::indeterminate;
                    }
                    else
                    {
                        return false;
                    }
                }
                case expecting_newline_1:
                {
                    if (input == '\n')
                    {
                        m_state_ = header_line_start;
                        return boost::indeterminate;
                    }
                    else
                    {
                        return false;
                    }
                }
                case header_line_start:
                {
                    if (input == '\r')
                    {
                        m_state_ = expecting_newline_3;
                        return boost::indeterminate;
                    }
                    else if (!req.headers.empty() && (input == ' ' || input == '\t'))
                    {
                        m_state_ = header_lws;
                        return boost::indeterminate;
                    }
                    else if (!is_char(input) || is_ctl(input) || is_tspecial(input))
                    {
                        return false;
                    }
                    else
                    {
                        req.headers.push_back(header());
                        req.headers.back().name.push_back(input);
                        m_state_ = header_name;
                        return boost::indeterminate;
                    }
                }
                case header_lws:
                {
                    if (input == '\r')
                    {
                        m_state_ = expecting_newline_2;
                        return boost::indeterminate;
                    }
                    else if (input == ' ' || input == '\t')
                    {
                        return boost::indeterminate;
                    }
                    else if (is_ctl(input))
                    {
                        return false;
                    }
                    else
                    {
                        m_state_ = header_value;
                        req.headers.back().value.push_back(input);
                        return boost::indeterminate;
                    }
                }
                case header_name:
                {
                    if (input == ':')
                    {
                        m_state_ = space_before_header_value;
                        return boost::indeterminate;
                    }
                    else if (!is_char(input) || is_ctl(input) || is_tspecial(input))
                    {
                        return false;
                    }
                    else
                    {
                        req.headers.back().name.push_back(input);
                        return boost::indeterminate;
                    }
                }
                case space_before_header_value:
                {
                    if (input == ' ')
                    {
                        m_state_ = header_value;
                        return boost::indeterminate;
                    }
                    else
                    {
                        return false;
                    }
                }
                case header_value:
                {
                    if (input == '\r')
                    {
                        m_state_ = expecting_newline_2;
                        return boost::indeterminate;
                    }
                    else if (is_ctl(input))
                    {
                        return false;
                    }
                    else
                    {
                        req.headers.back().value.push_back(input);
                        return boost::indeterminate;
                    }
                }
                case expecting_newline_2:
                {
                    if (input == '\n')
                    {
                        m_state_ = header_line_start;
                        return boost::indeterminate;
                    }
                    else
                    {
                        return false;
                    }
                }
                case expecting_newline_3:
                {
                    return (input == '\n');
                }
                default:
                {
                    return false;
                }
            }
        }

        /// Check if a byte is an HTTP character.
        static bool is_char(int c)
        {
            return c >= 0 && c <= 127;
        }

        /// Check if a byte is an HTTP control character.
        static bool is_ctl(int c)
        {
            return (c >= 0 && c <= 31) || (c == 127);
        }

        /// Check if a byte is defined as an HTTP tspecial character.
        static bool is_tspecial(int c)
        {
            switch (c)
            {
            case '(': case ')': case '<': case '>': case '@':
            case ',': case ';': case ':': case '\\': case '"':
            case '/': case '[': case ']': case '?': case '=':
            case '{': case '}': case ' ': case '\t':
                return true;
            default:
                return false;
            }
        }

        /// Check if a byte is a digit.
        static bool is_digit(int c)
        {
            return c >= '0' && c <= '9';
        }
    private:
        enum state
        {
            method_start,
            method,
            uri_start,
            uri,
            http_version_h,
            http_version_t_1,
            http_version_t_2,
            http_version_p,
            http_version_slash,
            http_version_major_start,
            http_version_major,
            http_version_minor_start,
            http_version_minor,
            expecting_newline_1,
            header_line_start,
            header_lws,
            header_name,
            space_before_header_value,
            header_value,
            expecting_newline_2,
            expecting_newline_3
        }m_state_;
    };
}

#endif
