#include <iostream>
#include <fstream>
#include <map>
#include <set>
#include <string>
#include <vector>

class Object
{
    std::map<std::string, std::string> Data;
    
 public:
    Object() = default;
    Object(const Object&) = default;
    Object& operator=(const Object&) = default;

    void Add(const std::string& key, const std::string& value)
    {
        Data[key] = value;
    }

    std::string GetByKey(const std::string& key) const
    {
        auto it = Data.find(key);
        if (it == Data.end())
            throw std::runtime_error("key not found");

        return it->second;
    }

    std::set<std::string> Names() const
    {
        std::set<std::string> names;

        for (const auto& kv : Data)
        {
            names.insert(kv.first);
        }

        return names;
    }
};

class Container
{
    std::map<std::string, Object> Data;

    enum class EAction
    {
        FindScheme,
        ReadFields,
        ReadObject
    };

 public:
    Container() = default;

    const Object& GetObject(const std::string& key) const
    {
        auto it = Data.find(key);
        if (it == Data.end())
            throw std::runtime_error("key not found");

        return it->second;
    }

    std::set<std::string> Names() const
    {
        std::set<std::string> names;

        for (const auto& kv : Data)
        {
            names.insert(kv.first);
        }

        return names;
    }

    void Erase(const std::string& key)
    {
        auto it = Data.find(key);
        if (it == Data.end())
            return;
        Data.erase(it);
    }

    void Erase(const std::set<std::string>& keys)
    {
        for (const auto& key : keys)
            Erase(key);
    }

    void AddObject(const std::string& key, const Object& obj)
    {
        if (key == "Scheme")
            throw std::runtime_error("not valid key");

        if (Data.find(key) != Data.end())
            throw std::runtime_error("not valid key");

        if (!Data.empty())
        {
            auto names = obj.Names();
            auto exist = Data.cbegin()->second.Names();

            bool right = std::equal(names.begin(), names.end(), exist.begin());
            if (!right)
                throw std::runtime_error("not valid object");
        }
        Data[key] = obj;
    }

    void Save(const std::string& filename) const
    {
        std::ofstream ofs(filename);
        if (Data.empty())
            return;

        std::set<std::string> names = Data.begin()->second.Names();

        ofs << "#Scheme" << std::endl;
        for (const std::string& field : names)
        {
            ofs << "\"" << field << "\"" << std::endl;
        }

        for (const std::pair<std::string, Object>& object : Data)
        {
            ofs << "#" << object.first << std::endl;

            for (const auto& field : names)
            {
                ofs << "\"" << object.second.GetByKey(field)<< "\"" << std::endl;
            }
        }
    }

    explicit Container(const std::string& filename)
    {
        EAction action = EAction::FindScheme;

        std::ifstream ifs(filename);

        std::string line;
        std::vector<std::string> schemes;
        std::string currentName;
        Object obj;
        size_t index = 0;
        while (std::getline(ifs, line))
        {
            if (EAction::FindScheme == action
                && line == "#Scheme")
            {
                action = EAction::ReadFields;
            }

            else if (EAction::ReadFields == action
                && !line.empty()
                && line[0] != '#')
            {
                size_t f = line.find_first_of('"', 1);
                if (f == std::string::npos)
                    throw std::runtime_error("wrong format");

                std::string value(line.begin() + 1, line.begin() + f);
                schemes.push_back(value);
            }

            else if (EAction::ReadFields == action
                && !line.empty()
                && line[0] == '#')
            {
                action = EAction::ReadObject;
                currentName.assign(line.begin() + 1, line.end());
            }

            else if (EAction::ReadObject == action
                && !line.empty()
                && line[0] != '#')
            {
                if (index == schemes.size())
                    throw std::runtime_error("wrong format");

                size_t f = line.find_first_of('"', 1);
                if (f == std::string::npos)
                    throw std::runtime_error("wrong format");

                std::string value(line.begin() + 1, line.begin() + f);
                obj.Add(schemes[index], value);
                ++index;
            }

            else if (EAction::ReadObject == action
                     && !line.empty()
                     && line[0] == '#')
            {
                if (index != schemes.size())
                    throw std::runtime_error("wrong format");

                AddObject(currentName, obj);
                index = 0;
                currentName.assign(line.begin() + 1, line.end());
            }
        }

        if (EAction::ReadObject == action)
        {
            if (index != schemes.size())
                throw std::runtime_error("wrong format");
            AddObject(currentName, obj);
        }
    }
};

int main()
{
    Object o;
    o.Add("Country", "Great Britain");
    o.Add("Capital", "London");

    Container parser;
    parser.AddObject("Country1", o);
    parser.AddObject("Country2", o);
    parser.AddObject("Country3", o);

    parser.Save("test.txt");

    Container parser2("test.txt");
    Object o2 = parser2.GetObject("Country3");

    return 0;
}
