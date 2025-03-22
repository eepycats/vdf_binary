#ifndef VDF_BINARY_HH
#define VDF_BINARY_HH

#include <istream>
#include <variant>
#include <memory>
#include <string>
#include <map>
namespace vdf_binary {
    namespace detail {
        template<typename T>
        T stream_read(std::istream& s){
            T x;
            s.read(reinterpret_cast<char*>(&x), sizeof(T)); // blehhh
            return x;
        }
    }
    
    struct key_value {
        struct kv_color{
            std::uint8_t r,g,b,a;
            std::string printf(){
                return std::format("rgba({},{},{},{})",r,g,b,a);
            }
        };
        
        using kv_variant = std::variant<
            std::shared_ptr<key_value>,
            std::string,
            std::int32_t,
            float,
            void*,
            std::wstring,
            kv_color,
            std::uint64_t
        >;

        enum type {
            TYPE_NONE,
            TYPE_STRING,
            TYPE_INT,
            TYPE_FLOAT,
            TYPE_PTR,
            TYPE_WSTRING,
            TYPE_COLOR,
            TYPE_UINT64,
            TYPE_NUMTYPES
        };


        void parse_from_stream(std::istream& in) {
            using namespace detail;
            for (;;) {
                type t = static_cast<type>(stream_read<std::uint8_t>(in));

                if (t == TYPE_NUMTYPES){
                    break;
                }
                std::string name;
                std::getline(in, name, '\0');
                kv_variant v;
                wchar_t c;
                switch (t){
                    case type::TYPE_NONE:
                        v = std::make_shared<key_value>();
                        std::get<std::shared_ptr<key_value>>(v)->parse_from_stream(in);
                        break;
                    case type::TYPE_STRING:
                        v = std::string();
                        std::getline(in, std::get<std::string>(v), '\0');
                        break;
                    case type::TYPE_INT:
                        v = stream_read<std::int32_t>(in);
                        break;
                    case type::TYPE_FLOAT:
                        v = stream_read<float>(in);
                        break;
                    case type::TYPE_PTR:
                        v = stream_read<void*>(in);
                        break;
                    case type::TYPE_WSTRING:
                        v = std::wstring();
                        c = stream_read<wchar_t>(in);
                        while (c != L'\0'){
                            std::get<std::wstring>(v) += c;
                            c = stream_read<wchar_t>(in);
                        }
                        break;
                    case type::TYPE_COLOR:
                        v = stream_read<kv_color>(in);
                    case type::TYPE_UINT64:
                        v = stream_read<std::uint64_t>(in);
                        break;
                    default:
                        throw std::exception("whatthefuck");
                }
                pairs[name] = v;
            }
        }
        std::map<std::string,kv_variant> pairs;
    };
}
#endif