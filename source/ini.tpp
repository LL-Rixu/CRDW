static inline void defaults(std::unordered_map<std::string_view, std::variant<bool, int64_t, std::string_view>>& map)
{
    /* General */
    map["General|Logging"]        = true;
    map["General|LogFlush"]       = 2;
    map["General|ShowPopUp"]      = true;
    map["General|Mode"]           = "LOAD";
    map["General|CacheDirectory"] = "Data/SKSE/Plugins/";
    map["General|Debug"]          = false;

    /* Optimization */
    map["Optimization|IOBuffer"]     = 262144;
    map["Optimization|Experimental"] = false;
}

static inline std::pair<size_t, size_t> makepair(std::vector<char>& buffer, std::string_view& section, std::string_view& key)
{
    size_t begin = buffer.size();

    buffer.insert(buffer.end(), section.begin(), section.end());
    buffer.push_back('|');
    buffer.insert(buffer.end(), key.begin(), key.end());

    return { begin, buffer.size() };
}

static inline std::string_view makekey(std::vector<char>& buffer, std::pair<size_t, size_t> offsets)
{
    return { &buffer[offsets.first], &buffer[offsets.second] };
}

static inline bool getline(std::string_view& stream, std::string_view& line) 
{
    if (stream.empty()) { return false; }

    size_t comment = std::string_view::npos;
    size_t width = stream.find('\n');

    line = stream.substr(0, width);

    size_t step = width != std::string_view::npos ? width + 1 : stream.size();
    stream.remove_prefix(step);

    line = line.substr(0, line.find(';'));

    if(!line.empty() && line.back() == '\r') { line.remove_suffix(1); }

    return true;
}

template<FixedString Path>
INI<Path>::INI(): MMAP(Path.c_str())
{
    keys.reserve(128);

    defaults(map);

    if(!Open()) { return; }

    std::vector<std::pair<std::pair<size_t, size_t>, VariantType>> buffer;
    buffer.reserve(10);

    std::string_view section;

    std::string_view stream(Map(), Count());
    std::string_view line;
    while(getline(stream, line))
    {
        size_t equals = line.find('=');
        if(equals == std::string_view::npos)
        {
            size_t lbracket = line.find('['), rbracket = line.find(']'); 
            
            if(lbracket != std::string_view::npos && rbracket != std::string_view::npos)
            {
                section = line.substr(lbracket + 1, rbracket - lbracket - 1);
            }
        
            continue; 
        }

        std::string_view key   = line.substr(0, equals);
        std::string_view value = line.substr(equals + 1);

        size_t colon = value.find(':');
        if(colon == std::string_view::npos || colon == 0) { continue; }

        std::pair<size_t, size_t> inikey = makepair(keys, section, key);

        switch(value[0])
        {
        case 'b': buffer.emplace_back(inikey, (value.substr(colon + 1) == "true" ? true : false)); break;
        case 'i': 
        { 
            std::string_view number = value.substr(colon + 1);
            int64_t result = 0;
            auto [ptr, error] = std::from_chars(number.data(), number.data() + number.size(), result);
            if(error == std::errc{}) { buffer.emplace_back(inikey, result); } 
        }
        break;
        case 's': buffer.emplace_back(inikey, value.substr(colon + 1)); break;

        default: continue;
        }
    }

    for(auto [inikey, value] : buffer) { map[makekey(keys, inikey)] = value; }
}