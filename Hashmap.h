#pragma once
#include <unordered_map>
#include <tuple>

template <class T, class R,
    class hashFunction = std::hash<T>,
    class keyEqual = std::equal_to<T>,
    class valueEqual = std::equal_to<R>>
    class Hashmap
{
private:
    size_t map_size;
    size_t allocated_size;
    float max_loading_factor;
    std::tuple<size_t, T, R>* hash_map;

    size_t hash(const std::pair<T, R>& entry, std::tuple<size_t, T, R>* current_map, size_t current_size) const
    {
        size_t hash1 = hashFunction()(entry.first);
        size_t hash2 = 2 * (hash1 / current_size) + 1;
        size_t t = 0;
        size_t position;

        while (t < current_size)
        {
            position = (hash1 + t * hash2) % current_size;

            if (std::get<0>(current_map[position]) >= ULLONG_MAX - 1)
                return position;

            if (std::get<0>(current_map[position]) < ULLONG_MAX - 1 && keyEqual()(entry.first, std::get<1>(current_map[position])))
                return ULLONG_MAX - 1;

            t++;
        }

        return ULLONG_MAX;
    }
    void resize()
    {
        size_t new_size = 2 * this->allocated_size;
        std::tuple<size_t, T, R>* new_table = new std::tuple<size_t, T, R>[new_size];

        for (size_t i = 0; i < new_size; i++)
            std::get<0>(new_table[i]) = ULLONG_MAX;

        for (size_t i = 0; i < this->allocated_size; i++)
            if (std::get<0>(this->hash_map[i]) < ULONG_MAX - 1)
            {
                T key = std::get<1>(this->hash_map[i]);
                R value = std::get<2>(this->hash_map[i]);
                size_t key_hash = hash({ key,value }, new_table, new_size);

                if (key_hash < ULLONG_MAX - 1)
                    new_table[key_hash] = std::make_tuple(key_hash, key, value);
            }

        delete[] this->hash_map;
        this->hash_map = new_table;
        this->allocated_size = new_size;
    }
public:
    Hashmap<T, R, hashFunction, keyEqual, valueEqual>(float max_loading_factor = 0.7)
    {
        this->allocated_size = 8;
        this->map_size = 0;
        this->max_loading_factor = max_loading_factor;
        this->hash_map = new std::tuple<size_t, T, R>[this->allocated_size];
        for (size_t i = 0; i < this->allocated_size; i++)
            std::get<0>(this->hash_map[i]) = ULLONG_MAX;
    }
    ~Hashmap<T, R, hashFunction, keyEqual, valueEqual>()
    {
        delete[] this->hash_map;
    }

    void insert(const std::pair<T, R>& entry)
    {
        float loading_factor = 1.0 * this->map_size / this->allocated_size;

        if (loading_factor >= this->max_loading_factor)
            resize();

        size_t key_hash = hash(entry, this->hash_map, this->allocated_size);

        if (key_hash >= ULONG_MAX - 1)
            return;

        this->hash_map[key_hash] = std::make_tuple(key_hash, entry.first, entry.second);
        this->map_size++;
    }
    std::tuple<size_t, T, R>* find(const T& key) const
    {
        size_t hash1 = hashFunction()(key);
        size_t hash2 = 2 * (hash1 / allocated_size) + 1;
        size_t t = 0;
        size_t position;

        while (t < map_size)
        {
            position = (hash1 + t * hash2) % allocated_size;
            if (std::get<0>(hash_map[position]) == ULLONG_MAX)
                return nullptr;

            if (std::get<0>(hash_map[position]) < ULLONG_MAX - 1 && keyEqual()(std::get<1>(hash_map[position]), key))
                return &(hash_map[position]);

            t++;
        }

        return nullptr;
    }
    void erase(const T& key)
    {
        auto ptr = find(key);

        if (ptr == nullptr)
            return;

        std::get<0>(*ptr) = ULLONG_MAX - 1;
    }

    size_t size() const
    {
        return map_size;
    }
    size_t alloc_size() const
    {
        return allocated_size;
    }

    R at(const T& key) const
    {
        auto ptr = find(key);

        if (ptr == nullptr)
            return R();

        return std::get<2>(*ptr);
    }
    R& operator[](const T& key)
    {
        auto ptr = find(key);

        if (ptr == nullptr)
            insert({ key,R() });

        ptr = find(key);

        return std::get<2>(*ptr);
    }
};
