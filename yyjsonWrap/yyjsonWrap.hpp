#pragma once

#include <string>
#include <vector>

#include "./yyjson/yyjson.h"

#ifdef IMPORT_YYJSON_IMPL
#include "./yyjson/yyjson.c"
#endif

#ifdef USE_WRAP_NAMESPACE
namespace wrap
{
#endif

	// Reading JSON
	class DocWrapper
	{
	public:
		class ValueWrapper
		{
		public:
			ValueWrapper() = default;
			ValueWrapper(yyjson_val* val, yyjson_doc* doc) : val_(val), doc_(doc) {}

			// Conversion operators
			operator int() const { return yyjson_get_int(val_); }
			operator int64_t() const { return yyjson_get_sint(val_); }
			operator uint64_t() const { return yyjson_get_uint(val_); }
			operator double() const { return yyjson_get_real(val_); }
			operator bool() const { return yyjson_get_bool(val_); }
			operator std::string() const { return yyjson_get_str(val_); }
			template <typename T> operator std::vector<T>() const
			{
				std::vector<T> res;
				if (yyjson_is_arr(val_))
				{
					size_t idx = 0;
					size_t max = 0;
					yyjson_val* val = nullptr;
					yyjson_arr_foreach(val_, idx, max, val)
					{
						ValueWrapper elem(val, doc_);
						res.push_back(static_cast<T>(elem));
					}
				}
				return res;
			}
			template <typename T> operator T() const; // Uses fromJson<T>

			// Access operators
			ValueWrapper operator[](const char* key) const { return {yyjson_obj_get(val_, key), doc_}; }
			ValueWrapper operator[](size_t index) const { return {yyjson_arr_get(val_, index), doc_}; }
			ValueWrapper operator[](int index) const { return {yyjson_arr_get(val_, static_cast<size_t>(index)), doc_}; }

			bool hasKey(const char* key) const { return yyjson_obj_get(val_, key) != nullptr; }

			yyjson_val* val_ = nullptr; // Direct access to underlying value
			yyjson_doc* doc_ = nullptr; // Direct access to underlying document
		};

		DocWrapper() = default;
		DocWrapper(const std::string& data) :
			doc(yyjson_read(data.c_str(), data.length(), 0)), root(yyjson_doc_get_root(doc)) {} // Parse JSON from string
		DocWrapper(const DocWrapper&) = delete;
		DocWrapper& operator=(const DocWrapper&) = delete;
		DocWrapper(DocWrapper&& other) noexcept : doc(other.doc), root(other.root)
		{
			other.doc = nullptr;
			other.root = nullptr;
		}
		DocWrapper& operator=(DocWrapper&& other) noexcept
		{
			if (this != &other)
			{
				if (doc) yyjson_doc_free(doc);
				doc = other.doc;
				root = other.root;
				other.doc = nullptr;
				other.root = nullptr;
			}
			return *this;
		}
		~DocWrapper()
		{
			if (doc) yyjson_doc_free(doc);
		}

		operator ValueWrapper() const { return {root, doc}; }
		std::string toString() const
		{
			char* json = yyjson_write(doc, 0, nullptr);
			std::string result(json);
			free(json);
			return result;
		}

		yyjson_doc* doc = nullptr;
		yyjson_val* root = nullptr;
	};

	// Writing JSON
	class MutDocWrapper
	{
	public:
		class MutValueWrapper
		{
		public:
			MutValueWrapper() = default;
			MutValueWrapper(yyjson_mut_val* val, yyjson_mut_doc* mutDoc) : val_(val), mutDoc_(mutDoc) {}

			MutValueWrapper& asObj()
			{
				if (!yyjson_mut_is_obj(val_)) yyjson_mut_set_obj(val_);
				return *this;
			}

			// Add object properties (variadic) (CAREFUL: it will NOT override existing properties)
			template <typename... Args> void set(Args&&... args)
			{
				asObj();
				setNoCheck(std::forward<Args>(args)...);
			}

			void setNoCheck() {}
			template <typename T, typename... Args> void setNoCheck(const char* key, const T& value, Args&&... args)
			{
				setNoCheck(key, value);
				setNoCheck(std::forward<Args>(args)...);
			}
			template <typename T> void setNoCheck(const char* key, const T& value)
			{
				yyjson_mut_val* val = createValue(value);
				yyjson_mut_obj_add_val(mutDoc_, val_, key, val);
			}

			MutValueWrapper& asArr()
			{
				if (!yyjson_mut_is_arr(val_)) yyjson_mut_set_arr(val_);
				return *this;
			}
			// Add array elements (variadic)
			template <typename... Args> void add(Args&&... args)
			{
				asArr();
				addNoCheck(std::forward<Args>(args)...);
			}
			// Add vector to array
			template <typename T> void addVector(const std::vector<T>& valueList)
			{
				asArr();
				addVectorNoCheck(valueList);
			}

			void addNoCheck() {}
			template <typename T> void addNoCheck(const T& value)
			{
				yyjson_mut_val* val = createValue(value);
				yyjson_mut_arr_append(val_, val);
			}
			template <typename T, typename... Args> void addNoCheck(const T& value, Args&&... args)
			{
				addNoCheck(value);
				addNoCheck(std::forward<Args>(args)...);
			}

			template <typename T> void addVectorNoCheck(const std::vector<T>& valueList)
			{
				for (const auto& value : valueList) addNoCheck(value);
			}

			yyjson_mut_val* val_ = nullptr;	   // Direct access to underlying value
			yyjson_mut_doc* mutDoc_ = nullptr; // Direct access to underlying document
		private:
			yyjson_mut_val* createValue(int value) { return yyjson_mut_int(mutDoc_, value); }
			yyjson_mut_val* createValue(int64_t value) { return yyjson_mut_sint(mutDoc_, value); }
			yyjson_mut_val* createValue(uint64_t value) { return yyjson_mut_uint(mutDoc_, value); }
			yyjson_mut_val* createValue(double value) { return yyjson_mut_real(mutDoc_, value); }
			yyjson_mut_val* createValue(bool value) { return yyjson_mut_bool(mutDoc_, value); }
			yyjson_mut_val* createValue(const std::string& value) { return yyjson_mut_strcpy(mutDoc_, value.c_str()); }
			yyjson_mut_val* createValue(const char* value) { return yyjson_mut_strcpy(mutDoc_, value); }
			template <typename T> yyjson_mut_val* createValue(const std::vector<T>& value)
			{
				yyjson_mut_val* arr = yyjson_mut_arr(mutDoc_);
				for (const auto& v : value)
				{
					yyjson_mut_val* val = createValue(v);
					yyjson_mut_arr_append(arr, val);
				}
				return arr;
			}
			template <typename T> yyjson_mut_val* createValue(const T& value)
			{
				yyjson_mut_val* obj = yyjson_mut_obj(mutDoc_);
				MutValueWrapper wrapper(obj, mutDoc_);
				toJson(wrapper, value);
				return obj;
			}
		};

		MutDocWrapper() : mutDoc(yyjson_mut_doc_new(nullptr)) // Create empty document
		{
			if (mutDoc) yyjson_mut_doc_set_root(mutDoc, yyjson_mut_obj(mutDoc));
		}
		MutDocWrapper(const MutDocWrapper&) = delete;
		MutDocWrapper& operator=(const MutDocWrapper&) = delete;
		MutDocWrapper(MutDocWrapper&& other) noexcept : mutDoc(other.mutDoc) { other.mutDoc = nullptr; }
		MutDocWrapper& operator=(MutDocWrapper&& other) noexcept
		{
			if (this != &other)
			{
				if (mutDoc) yyjson_mut_doc_free(mutDoc);
				mutDoc = other.mutDoc;
				other.mutDoc = nullptr;
			}
			return *this;
		}
		~MutDocWrapper()
		{
			if (mutDoc) yyjson_mut_doc_free(mutDoc);
		}

		operator MutValueWrapper() const { return {yyjson_mut_doc_get_root(mutDoc), mutDoc}; }
		std::string toString() const
		{
			char* json = yyjson_mut_write(mutDoc, 0, nullptr);
			std::string result(json);
			free(json);
			return result;
		}

		yyjson_mut_doc* mutDoc = nullptr;
	};

	// Custom type conversion
	template <typename T> T fromJson(const DocWrapper::ValueWrapper&)
	{
		static_assert(sizeof(T) == 0, "fromJson not implemented for this type");
		return T();
	}

	// Implementation of ValueWrapper::operator T()
	template <typename T> DocWrapper::ValueWrapper::operator T() const { return fromJson<T>(*this); }

	template <typename T> void toJson(MutDocWrapper::MutValueWrapper&, const T&)
	{
		static_assert(sizeof(T) == 0, "toJson not implemented for this type");
	}

	using ValueWrapper = DocWrapper::ValueWrapper;
	using MutValueWrapper = MutDocWrapper::MutValueWrapper;

#ifdef USE_WRAP_NAMESPACE
} // namespace wrap
#endif
