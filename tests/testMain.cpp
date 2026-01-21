#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#define IMPORT_YYJSON_IMPL
#include "../yyjsonWrap/yyjsonWrap.hpp"
#undef IMPORT_YYJSON_IMPL

// Test structures for custom type conversion
struct Address
{
	std::string street;
	std::string city;
	std::string zipCode;

	bool operator==(const Address& other) const
	{
		return street == other.street && city == other.city && zipCode == other.zipCode;
	}
};

struct Person
{
	std::string name;
	int age;
	std::vector<std::string> hobbies;

	bool operator==(const Person& other) const { return name == other.name && age == other.age && hobbies == other.hobbies; }
};

// Custom type conversions
template <> void toJson(MutValueWrapper& value, const Address& a)
{
	value.set("street", a.street, "city", a.city, "zipCode", a.zipCode);
}

template <> void toJson(MutValueWrapper& value, const Person& p)
{
	value.set("name", p.name, "age", p.age, "hobbies", p.hobbies);
}

template <> Address fromJson(const ValueWrapper& doc) { return Address{doc["street"], doc["city"], doc["zipCode"]}; }

template <> Person fromJson(const ValueWrapper& doc) { return Person{doc["name"], doc["age"], doc["hobbies"]}; }

// ============================================================================
// Tests for DocWrapper (Reading JSON)
// ============================================================================

TEST_CASE("DocWrapper - Parse simple JSON object")
{
	std::string json = R"({"name":"Alice","age":25})";
	DocWrapper doc(json);
	ValueWrapper root = doc;

	CHECK(root.hasKey("name"));
	CHECK(root.hasKey("age"));
	CHECK_FALSE(root.hasKey("nonexistent"));
}

TEST_CASE("DocWrapper - Read primitive types")
{
	std::string json = R"({"intVal":42,"doubleVal":3.14,"boolVal":true,"stringVal":"hello"})";
	DocWrapper doc(json);
	ValueWrapper root = doc;

	CHECK(static_cast<int>(root["intVal"]) == 42);
	CHECK(static_cast<double>(root["doubleVal"]) == doctest::Approx(3.14));
	CHECK(static_cast<bool>(root["boolVal"]) == true);
	CHECK(static_cast<std::string>(root["stringVal"]) == "hello");
}

TEST_CASE("DocWrapper - Read arrays")
{
	std::string json = R"({"numbers":[1,2,3],"strings":["a","b","c"]})";
	DocWrapper doc(json);
	ValueWrapper root = doc;

	std::vector<int> numbers = root["numbers"];
	CHECK(numbers == std::vector<int>{1, 2, 3});

	std::vector<std::string> strings = root["strings"];
	CHECK(strings == std::vector<std::string>{"a", "b", "c"});
}

TEST_CASE("DocWrapper - Access array elements by index")
{
	std::string json = R"({"items":[10,20,30]})";
	DocWrapper doc(json);
	ValueWrapper root = doc;

	CHECK(static_cast<int>(root["items"][0]) == 10);
	CHECK(static_cast<int>(root["items"][1]) == 20);
	CHECK(static_cast<int>(root["items"][2]) == 30);
}

TEST_CASE("DocWrapper - Nested objects")
{
	std::string json = R"({"person":{"name":"Bob","age":30}})";
	DocWrapper doc(json);
	ValueWrapper root = doc;

	CHECK(static_cast<std::string>(root["person"]["name"]) == "Bob");
	CHECK(static_cast<int>(root["person"]["age"]) == 30);
}

TEST_CASE("DocWrapper - Custom type conversion")
{
	std::string json = R"({"street":"123 Main St","city":"New York","zipCode":"10001"})";
	DocWrapper doc(json);
	ValueWrapper root = doc;

	Address addr = root;
	CHECK(addr.street == "123 Main St");
	CHECK(addr.city == "New York");
	CHECK(addr.zipCode == "10001");
}

TEST_CASE("DocWrapper - toString roundtrip")
{
	std::string original = R"({"name":"Test","value":42})";
	DocWrapper doc(original);
	std::string serialized = doc.toString();

	// Parse again to verify
	DocWrapper doc2(serialized);
	ValueWrapper root = doc2;
	CHECK(static_cast<std::string>(root["name"]) == "Test");
	CHECK(static_cast<int>(root["value"]) == 42);
}

// ============================================================================
// Tests for MutDocWrapper (Writing JSON)
// ============================================================================

TEST_CASE("MutDocWrapper - Create empty document")
{
	MutDocWrapper mutDoc;
	MutValueWrapper root = mutDoc;
	std::string json = mutDoc.toString();
	CHECK(json.find('{') != std::string::npos); // Should be a valid JSON object
}

TEST_CASE("MutDocWrapper - Set primitive values")
{
	MutDocWrapper mutDoc;
	MutValueWrapper root = mutDoc;

	root.set("intVal", 42, "doubleVal", 3.14, "boolVal", true, "stringVal", "hello");

	std::string json = mutDoc.toString();
	DocWrapper doc(json);
	ValueWrapper readRoot = doc;

	CHECK(static_cast<int>(readRoot["intVal"]) == 42);
	CHECK(static_cast<double>(readRoot["doubleVal"]) == doctest::Approx(3.14));
	CHECK(static_cast<bool>(readRoot["boolVal"]) == true);
	CHECK(static_cast<std::string>(readRoot["stringVal"]) == "hello");
}

TEST_CASE("MutDocWrapper - Add array elements")
{
	MutDocWrapper mutDoc;
	MutValueWrapper root = mutDoc;

	// Create array directly using set with vector
	std::vector<int> itemsVec{1, 2, 3};
	root.set("items", itemsVec);

	std::string json = mutDoc.toString();
	DocWrapper doc(json);
	ValueWrapper readRoot = doc;

	std::vector<int> items = readRoot["items"];
	CHECK(items == std::vector<int>{1, 2, 3});
}

TEST_CASE("MutDocWrapper - Add vector to array")
{
	MutDocWrapper mutDoc;
	MutValueWrapper root = mutDoc;

	std::vector<std::string> hobbies = {"reading", "coding", "gaming"};
	root.set("hobbies", hobbies);

	std::string json = mutDoc.toString();
	DocWrapper doc(json);
	ValueWrapper readRoot = doc;

	std::vector<std::string> result = readRoot["hobbies"];
	CHECK(result == hobbies);
}

TEST_CASE("MutDocWrapper - Nested objects")
{
	MutDocWrapper mutDoc;
	MutValueWrapper root = mutDoc;

	// Create nested object using Address struct
	Address addr{"123 Main St", "New York", "10001"};
	root.set("address", addr);

	std::string json = mutDoc.toString();
	DocWrapper doc(json);
	ValueWrapper readRoot = doc;

	// Verify nested object structure
	CHECK(static_cast<std::string>(readRoot["address"]["street"]) == "123 Main St");
	CHECK(static_cast<std::string>(readRoot["address"]["city"]) == "New York");
	CHECK(static_cast<std::string>(readRoot["address"]["zipCode"]) == "10001");
}

TEST_CASE("MutDocWrapper - Custom type serialization")
{
	MutDocWrapper mutDoc;
	MutValueWrapper root = mutDoc;

	Person p{"Alice", 25, {"reading", "coding"}};
	toJson(root, p);

	std::string json = mutDoc.toString();
	DocWrapper doc(json);
	ValueWrapper readRoot = doc;

	Person result = readRoot;
	CHECK(result == p);
}

// ============================================================================
// Integration Tests
// ============================================================================

TEST_CASE("Roundtrip - Write then read")
{
	// Write JSON
	MutDocWrapper mutDoc;
	MutValueWrapper root = mutDoc;
	root.set("name", "Charlie", "age", 35, "active", true);

	std::string json = mutDoc.toString();

	// Read JSON
	DocWrapper doc(json);
	ValueWrapper readRoot = doc;

	CHECK(static_cast<std::string>(readRoot["name"]) == "Charlie");
	CHECK(static_cast<int>(readRoot["age"]) == 35);
	CHECK(static_cast<bool>(readRoot["active"]) == true);
}

TEST_CASE("Roundtrip - Custom types")
{
	// Write
	Address addr{"456 Oak Ave", "Boston", "02101"};
	MutDocWrapper mutDoc;
	MutValueWrapper root = mutDoc;
	toJson(root, addr);

	std::string json = mutDoc.toString();

	// Read
	DocWrapper doc(json);
	ValueWrapper readRoot = doc;
	Address result = readRoot;

	CHECK(result == addr);
}

TEST_CASE("Complex nested structure")
{
	std::string json = R"({
		"users": [
			{"name": "Alice", "age": 25},
			{"name": "Bob", "age": 30}
		],
		"metadata": {
			"count": 2,
			"active": true
		}
	})";

	DocWrapper doc(json);
	ValueWrapper root = doc;

	// Access nested array
	CHECK(static_cast<std::string>(root["users"][0]["name"]) == "Alice");
	CHECK(static_cast<int>(root["users"][0]["age"]) == 25);
	CHECK(static_cast<std::string>(root["users"][1]["name"]) == "Bob");
	CHECK(static_cast<int>(root["users"][1]["age"]) == 30);

	// Access nested object
	CHECK(static_cast<int>(root["metadata"]["count"]) == 2);
	CHECK(static_cast<bool>(root["metadata"]["active"]) == true);
}

TEST_CASE("Large integers")
{
	std::string json = R"({"bigInt":9223372036854775807,"smallInt":-9223372036854775807})";
	DocWrapper doc(json);
	ValueWrapper root = doc;

	CHECK(static_cast<int64_t>(root["bigInt"]) == 9223372036854775807LL);
	CHECK(static_cast<int64_t>(root["smallInt"]) == -9223372036854775807LL);
}
