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
