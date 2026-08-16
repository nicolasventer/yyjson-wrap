#include <iostream>
#include <map>
#include <optional>
#include <string>
#include <vector>

#define IMPORT_YYJSON_IMPL
#include "yyjsonWrap/yyjsonWrap.hpp"
#undef IMPORT_YYJSON_IMPL

namespace
{
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
		std::map<std::string, std::string> metadata;
	};
} // namespace

template <> void toJson(MutValueWrapper& value, const Address& a)
{
	value.set("street", a.street, "city", a.city, "zipCode", a.zipCode);
}

template <> void toJson(MutValueWrapper& value, const Person& p)
{
	value.set("name", p.name, "age", p.age, "address", p.address, "hobbies", p.hobbies, "metadata", p.metadata);
}

template <> Address fromJson(const ValueWrapper& doc) { return Address{doc["street"], doc["city"], doc["zipCode"]}; }

template <> Person fromJson(const ValueWrapper& doc)
{
	return Person{doc["name"], doc["age"], doc["address"].asOptional<Address>(), doc["hobbies"], doc["metadata"]};
}

int main()
{
	const std::string json = R"(
    {
        "name": "Alice",
        "age": 25,
        "hobbies": ["reading", "coding"],
        "metadata": {
            "department": "Engineering",
            "level": "Senior"
        }
    }
    )";
	const DocWrapper doc(json);
	const ValueWrapper value = doc;
	Person p = value;
	p.address = Address{"123 Main St", "New York", "10001"};
	p.metadata["team"] = "Platform";
	const MutDocWrapper mutDoc;
	MutValueWrapper root = mutDoc;
	root = p;
	const std::string serialized = mutDoc.toString(EPrettyPrint::Pretty);
	std::cout << serialized << "\n";
}
