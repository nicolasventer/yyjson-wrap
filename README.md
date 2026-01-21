# yyjsonWrap

# Description

yyjsonWrap is a **c++ library** that allows you to **read and write json data** with an easy way to convert to structured data. It is a wrapper around [yyjson](https://github.com/ibireme/yyjson).

# Features

- read and write json data
- convert structured data from and to json data
- easy to use (header only)

Note: This library is both fast and easy to use since it is a wrapper around yyjson.

# Installation

## Header only

Include the [`yyjsonWrap.hpp`](yyjsonWrap/yyjsonWrap.hpp) anywhere you want to use it.

Download the `yyjsonWrap` folder using this [download link](https://download-directory.github.io/?url=https%3A%2F%2Fgithub.com%2Fnicolasventer%2Fyyjson-wrap%2Ftree%2Fmain%2FyyjsonWrap).  
Alternatively, you can download the latest version of `yyjson source files` directly from the [yyjson repository](https://github.com/ibireme/yyjson/tree/master/src) using this [download link](https://download-directory.github.io/?url=https%3A%2F%2Fgithub.com%2Fibireme%2Fyyjson%2Ftree%2Fmaster%2Fsrc).

To use the library, you need to define `IMPORT_YYJSON_IMPL` before including the header to include the implementation:

```cpp
#define IMPORT_YYJSON_IMPL
#include "yyjsonWrap/yyjsonWrap.hpp"
#undef IMPORT_YYJSON_IMPL
```

### Requirements

c++17 or later required for compilation.  
No external dependencies (yyjson is included in the repository).

# Example

Content of [example.cpp](example.cpp)

```cpp
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#define IMPORT_YYJSON_IMPL
#include "yyjsonWrap/yyjsonWrap.hpp"
#undef IMPORT_YYJSON_IMPL

struct Address
{
	std::string street;
	std::string city;
	std::string zipCode;
};

struct Person
{
	std::string name;
	int age;
	std::optional<Address> address;
	std::vector<std::string> hobbies;
};

template <> void toJson(MutValueWrapper& value, const Address& a)
{
	value.set("street", a.street, "city", a.city, "zipCode", a.zipCode);
}

template <> void toJson(MutValueWrapper& value, const Person& p)
{
	value.set("name", p.name, "age", p.age, "hobbies", p.hobbies);
	if (p.address.has_value()) value.set("address", p.address.value());
}

template <> Address fromJson(const ValueWrapper& doc) { return Address{doc["street"], doc["city"], doc["zipCode"]}; }

template <> Person fromJson(const ValueWrapper& doc)
{
	Person res{doc["name"], doc["age"], {}, doc["hobbies"]};
	if (doc.hasKey("address")) res.address = Address(doc["address"]);
	return res;
}

int main()
{
	std::string json = R"(
    {
        "name": "Alice",
        "age": 25,
        "hobbies": ["reading", "coding"]
    }
    )";
	DocWrapper doc(json);
	ValueWrapper value = doc;
	Person p = value;
	p.address = Address{"123 Main St", "New York", "10001"};
	MutDocWrapper mutDoc;
	MutValueWrapper root = mutDoc;
	toJson(root, p);
	std::string serialized = mutDoc.toString();
	std::cout << serialized << "\n";
}

```

Output:

```
{"name":"Alice","age":25,"hobbies":["reading","coding"],"address":{"street":"123 Main St","city":"New York","zipCode":"10001"}}
```

# Usage

_The usage is not exhaustive._

## Reading JSON

```cpp
// Parse JSON from string
DocWrapper doc(jsonString);

// Access values
ValueWrapper root = doc;
ValueWrapper name = root["name"];
std::string nameStr = name;  // Conversion operator
int age = root["age"];       // Conversion operator
std::vector<std::string> hobbies = root["hobbies"];  // Vector conversion

// Check if key exists
if (root.hasKey("address")) {
    // Access nested object
    Address addr = root["address"];
}

// Access array elements
ValueWrapper firstHobby = root["hobbies"][0];
```

## Writing JSON

```cpp
// Create a mutable document
MutDocWrapper mutDoc;
MutValueWrapper root = mutDoc;

// Set object properties
root.set("name", "Alice", "age", 25);

// Add to arrays
root.asArr().add("item1", "item2", "item3");

// Or add vectors
std::vector<std::string> hobbies = {"reading", "coding"};
root.set("hobbies", hobbies);

// Convert to string
std::string json = mutDoc.toString();
```

## Custom Types

```cpp
// Implement toJson for writing custom types
template <> void toJson(MutValueWrapper& value, const MyType& obj)
{
    value.set("field1", obj.field1, "field2", obj.field2);
}

// Implement fromJson for reading custom types
template <> MyType fromJson(const ValueWrapper& doc)
{
    return MyType{doc["field1"], doc["field2"]};
}
```

## Main API

```cpp
// Reading JSON
class DocWrapper
{
public:
    DocWrapper(const std::string& data);  // Parse JSON from string
    operator ValueWrapper() const;
    std::string toString() const;

    class ValueWrapper
    {
    public:
        // Conversion operators
        operator int() const;
        operator int64_t() const;
        operator uint64_t() const;
        operator double() const;
        operator bool() const;
        operator std::string() const;
        template <typename T> operator std::vector<T>() const;
        template <typename T> operator T() const;  // Uses fromJson<T>

        // Access operators
        ValueWrapper operator[](const char* key) const;
        ValueWrapper operator[](size_t index) const;
        ValueWrapper operator[](int index) const;

        bool hasKey(const char* key) const;

        yyjson_val* val_;  // Direct access to underlying value
        yyjson_doc* doc_;  // Direct access to underlying document
    };
};

// Writing JSON
class MutDocWrapper
{
public:
    MutDocWrapper();  // Create empty document
    operator MutValueWrapper() const;
    std::string toString() const;

    class MutValueWrapper
    {
        // Add object properties (variadic) (CAREFUL: it will NOT override existing properties)
        template <typename... Args> void set(Args&&... args);

        // Add array elements (variadic)
        template <typename... Args> void add(Args&&... args);

        // Add vector to array
        template <typename T> void addVector(const std::vector<T>& valueList);

        yyjson_mut_val* val_;     // Direct access to underlying value
        yyjson_mut_doc* mutDoc_;  // Direct access to underlying document
    };
};

// Custom type conversion
template <typename T> T fromJson(const ValueWrapper& doc);
template <typename T> void toJson(MutValueWrapper& value, const T& obj);

using ValueWrapper = DocWrapper::ValueWrapper;
using MutValueWrapper = MutDocWrapper::MutValueWrapper;
```

**Note:** you should never manually call `fromJson` or `toJson`.

# License

MIT License. See [LICENSE file](LICENSE).
Please refer me with:

    Copyright (c) Nicolas VENTER All rights reserved.
