# yyjsonWrap

## Description

yyjsonWrap is a **C++ library** that lets you **read and write JSON** and convert between JSON and your own types. It wraps [yyjson](https://github.com/ibireme/yyjson).

## Features

- Read and write JSON (parse from string, build mutable documents, serialize to string)
- Convert between JSON and structured data via `fromJson` / `toJson`
- Header-only; define `IMPORT_YYJSON_IMPL` in one translation unit to pull in the implementation
- Optional namespace: define `USE_WRAP_NAMESPACE` to use the `wrap::` namespace
- Optional extra primitive conversions: define `USE_PRIMITIVE_CONVERSION` for `unsigned int`, `signed char`, `unsigned char`, `unsigned short`, `float` (via `fromJson`/`toJson`)
- Fast and dependency-free (yyjson is included in the repo)

Note: Performance comes from yyjson; the wrapper adds a small, type-safe C++ layer on top.

## Installation

### Header-only

1. Add the `yyjsonWrap` folder to your include path.
2. Include [`yyjsonWrap.hpp`](yyjsonWrap/yyjsonWrap.hpp) where you use the library.
3. In **exactly one** translation unit, define `IMPORT_YYJSON_IMPL` before the include so the yyjson implementation is compiled:

```cpp
#define IMPORT_YYJSON_IMPL
#include "yyjsonWrap/yyjsonWrap.hpp"
#undef IMPORT_YYJSON_IMPL
```

To use the optional `wrap` namespace (e.g. `wrap::DocWrapper`), define `USE_WRAP_NAMESPACE` before including the header.

**Getting the code:** Clone this repo or download the `yyjsonWrap` folder via [this link](https://download-directory.github.io/?url=https%3A%2F%2Fgithub.com%2Fnicolasventer%2Fyyjson-wrap%2Ftree%2Fmain%2FyyjsonWrap). The yyjson sources are included under `yyjsonWrap/yyjson/`; you can also fetch the latest from the [yyjson repo](https://github.com/ibireme/yyjson/tree/master/src) if needed.

### Requirements

- **C++17** or later
- No external dependencies (yyjson is vendored in the repository)

## Example

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
	root = p;
	std::string serialized = mutDoc.toString();
	std::cout << serialized << "\n";
}

```

**Output:**

```
{"name":"Alice","age":25,"hobbies":["reading","coding"],"address":{"street":"123 Main St","city":"New York","zipCode":"10001"}}
```

## Testing

Run the tests from the project root (see `tests/` and `tests/exec_test.bat` for your setup).

## Usage

_The usage is not exhaustive._

### Reading JSON

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

### Writing JSON

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

### Custom Types

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

### Main API

```cpp
// Reading JSON
class DocWrapper
{
public:
    // Parse JSON from string or buffer
    DocWrapper(const char* data, size_t len);
    DocWrapper(const std::string& data);
    operator ValueWrapper() const;
    std::string toString() const;
    // Move-only; doc/root are internal.

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
        template <typename T, size_t N> operator std::array<T, N>() const;
        template <typename T, size_t N> void fillArray(T (&arr)[N]) const;
        template <typename T> operator T() const;  // Uses fromJson<T>
        template <typename TTo, typename TFrom> TTo as() const;  // Explicit conversion: second type is the intermediate type

        // Access
        ValueWrapper operator[](const char* key) const;
        ValueWrapper operator[](size_t index) const;
        ValueWrapper operator[](int index) const;
        bool hasKey(const char* key) const;

        std::string toString() const;

        yyjson_val* val_;
        yyjson_doc* doc_;
    };
};

// Writing JSON
class MutDocWrapper
{
public:
    MutDocWrapper();  // Empty document with root object
    operator MutValueWrapper() const;
    std::string toString() const;

    class MutValueWrapper
    {
    public:
        MutValueWrapper& asObj();   // Ensure value is an object
        MutValueWrapper& asArr();   // Ensure value is an array

        // Object: set key-value pairs. Does NOT override existing keys.
        template <typename... Args> void set(Args&&... args);
        template <typename T> void setNoCheck(const char* key, const T& value);

        // Array: add elements
        template <typename... Args> void add(Args&&... args);
        template <typename T> void addArray(const std::vector<T>& valueList);
        template <typename T, size_t N> void addArray(const std::array<T, N>& valueList);
        template <typename T, size_t N> void addArray(const T (&valueList)[N]);

        // Assignment: set this value to a primitive, vector, array, or custom type
        MutValueWrapper& operator=(const int& value);
        MutValueWrapper& operator=(const int64_t& value);
        MutValueWrapper& operator=(const uint64_t& value);
        MutValueWrapper& operator=(const double& value);
        MutValueWrapper& operator=(const bool& value);
        MutValueWrapper& operator=(const std::string& value);
        MutValueWrapper& operator=(const char* value);
        template <typename T> MutValueWrapper& operator=(const std::vector<T>& value);
        template <typename T, size_t N> MutValueWrapper& operator=(const std::array<T, N>& value);
        template <typename T, size_t N> MutValueWrapper& operator=(const T (&value)[N]);
        template <typename T> MutValueWrapper& operator=(const T& value);

        std::string toString() const;

        yyjson_mut_val* val_;     // Direct access to underlying value
        yyjson_mut_doc* mutDoc_;  // Direct access to underlying document
    };
};

// Custom type conversion (specialize these for your types)
template <typename T> T fromJson(const ValueWrapper& doc);
template <typename T> void toJson(MutValueWrapper& value, const T& obj);

using ValueWrapper = DocWrapper::ValueWrapper;
using MutValueWrapper = MutDocWrapper::MutValueWrapper;
```

**Note:** You should never manually call `fromJson` or `toJson`. Use implicit conversion (e.g. `Person p = value;`) or `static_cast<T>(value)` instead.

## License

MIT License. See [LICENSE file](LICENSE).  
Please refer me with:

    Copyright (c) Nicolas VENTER. All rights reserved.
