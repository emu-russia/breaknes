// Full unit test coverage of the JsonLib.
// https://github.com/emu-russia/breaknes/issues/500

#include "pch.h"

#include <cwchar>
#include <cstring>
#include <cmath>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest
{
	TEST_CLASS(JsonLibUnitTest)
	{
	public:

		/// <summary>
		/// Serialize -> Deserialize roundtrip of a complex Json document.
		/// </summary>
		TEST_METHOD(TestSerializeDeserializeRoundtrip)
		{
			Json json;

			Json::Value* rootObj = json.root.AddObject("root");
			rootObj->AddInt("intVal", 42);
			rootObj->AddUInt16("u16Val", 0xABCD);
			rootObj->AddUInt32("u32Val", 0xDEADBEEF);
			rootObj->AddUInt64("u64Val", 0x0123456789ABCDEFULL);
			rootObj->AddFloat("floatVal", 3.14f);
			rootObj->AddBool("boolTrue", true);
			rootObj->AddBool("boolFalse", false);
			rootObj->AddNull("nullVal");
			rootObj->AddString("strVal", L"hello \"world\" \\ test\n");
			rootObj->AddAnsiString("ansiVal", "ansi string");

			Json::Value* arr = rootObj->AddArray("arrVal");
			arr->AddInt(nullptr, 1);
			arr->AddInt(nullptr, 2);
			arr->AddInt(nullptr, 3);

			Json::Value* nested = rootObj->AddObject("nested");
			nested->AddInt("deep", 777);

			// Get serialized size

			size_t actualSize = 0;
			json.GetSerializedTextSize(nullptr, 0x100000, actualSize);
			Assert::IsTrue(actualSize > 0);

			// Serialize

			uint8_t* buf = new uint8_t[actualSize + 1];
			size_t serializedSize = 0;
			json.Serialize(buf, actualSize + 1, serializedSize);
			Assert::IsTrue(serializedSize == actualSize);
			buf[serializedSize] = 0;

			Logger::WriteMessage(("Serialized JSON:\n" + std::string((char*)buf) + "\n").c_str());

			// Deserialize into a new Json and check the values

			Json json2;
			json2.Deserialize(buf, serializedSize);
			delete[] buf;

			Json::Value* root2 = json2.root.children.front();

			Assert::IsTrue(root2->type == Json::ValueType::Object);

			Json::Value* v = root2->ByName("intVal");
			Assert::IsTrue(v != nullptr && v->type == Json::ValueType::Int && v->value.AsInt == 42);

			v = root2->ByName("u16Val");
			Assert::IsTrue(v != nullptr && v->type == Json::ValueType::Int && v->value.AsUint16 == 0xABCD);

			v = root2->ByName("u32Val");
			Assert::IsTrue(v != nullptr && v->type == Json::ValueType::Int && v->value.AsUint32 == 0xDEADBEEF);

			v = root2->ByName("u64Val");
			Assert::IsTrue(v != nullptr && v->type == Json::ValueType::Int && v->value.AsInt == 0x0123456789ABCDEFULL);

			v = root2->ByName("floatVal");
			Assert::IsTrue(v != nullptr && v->type == Json::ValueType::Float && fabsf(v->value.AsFloat - 3.14f) < 0.001f);

			v = root2->ByName("boolTrue");
			Assert::IsTrue(v != nullptr && v->type == Json::ValueType::Bool && v->value.AsBool == true);

			v = root2->ByName("boolFalse");
			Assert::IsTrue(v != nullptr && v->type == Json::ValueType::Bool && v->value.AsBool == false);

			v = root2->ByName("nullVal");
			Assert::IsTrue(v != nullptr && v->type == Json::ValueType::Null);

			v = root2->ByName("strVal");
			Assert::IsTrue(v != nullptr && v->type == Json::ValueType::String);
			Assert::IsTrue(wcscmp(v->value.AsString, L"hello \"world\" \\ test\n") == 0);

			v = root2->ByName("ansiVal");
			Assert::IsTrue(v != nullptr && v->type == Json::ValueType::String);
			Assert::IsTrue(wcscmp(v->value.AsString, L"ansi string") == 0);

			v = root2->ByName("arrVal");
			Assert::IsTrue(v != nullptr && v->type == Json::ValueType::Array && v->children.size() == 3);
			Assert::IsTrue(v->children.front()->value.AsInt == 1);
			Assert::IsTrue(v->children.back()->value.AsInt == 3);

			v = root2->ByName("nested");
			Assert::IsTrue(v != nullptr && v->type == Json::ValueType::Object);
			Json::Value* deep = v->ByName("deep");
			Assert::IsTrue(deep != nullptr && deep->value.AsInt == 777);
		}

		/// <summary>
		/// Deserialize a text document written by hand and check all its parts.
		/// </summary>
		TEST_METHOD(TestDeserialize)
		{
			const char* text = "{ \"a\" : 5, \"b\" : 2.5, \"s\" : \"text\", \"t\" : true, \"f\" : false, \"n\" : null, "
				"\"o\" : { \"x\" : 1, \"y\" : 2 }, \"arr\" : [ 10, 20, 30 ] }";

			Json json;
			json.Deserialize((void*)text, strlen(text));

			Assert::IsTrue(json.root.children.size() == 1);
			Json::Value* root = json.root.children.front();
			Assert::IsTrue(root->type == Json::ValueType::Object);

			Assert::IsTrue(root->ByName("a")->value.AsInt == 5);
			Assert::IsTrue(root->ByName("b")->type == Json::ValueType::Float);
			Assert::IsTrue(fabsf(root->ByName("b")->value.AsFloat - 2.5f) < 0.001f);
			Assert::IsTrue(root->ByName("s")->type == Json::ValueType::String);
			Assert::IsTrue(wcscmp(root->ByName("s")->value.AsString, L"text") == 0);
			Assert::IsTrue(root->ByName("t")->value.AsBool == true);
			Assert::IsTrue(root->ByName("f")->value.AsBool == false);
			Assert::IsTrue(root->ByName("n")->type == Json::ValueType::Null);

			Json::Value* o = root->ByName("o");
			Assert::IsTrue(o->type == Json::ValueType::Object);
			Assert::IsTrue(o->ByName("x")->value.AsInt == 1);
			Assert::IsTrue(o->ByName("y")->value.AsInt == 2);

			Json::Value* arr = root->ByName("arr");
			Assert::IsTrue(arr->type == Json::ValueType::Array);
			Assert::IsTrue(arr->children.size() == 3);
			Assert::IsTrue(arr->children.front()->value.AsInt == 10);
			Assert::IsTrue(arr->children.back()->value.AsInt == 30);
		}

		/// <summary>
		/// All the Add* methods.
		/// </summary>
		TEST_METHOD(TestAddMethods)
		{
			Json json;

			Json::Value* root = json.root.AddObject("root");
			Assert::IsTrue(root->name != nullptr && strcmp(root->name, "root") == 0);
			Assert::IsTrue(root->type == Json::ValueType::Object);
			Assert::IsTrue(root->parent == &json.root);

			Json::Value* i = root->AddInt("i", 100);
			Assert::IsTrue(i->type == Json::ValueType::Int && i->value.AsInt == 100);
			Assert::IsTrue(i->parent == root);

			Json::Value* u16 = root->AddUInt16("u16", 0x1234);
			Assert::IsTrue(u16->type == Json::ValueType::Int && u16->value.AsUint16 == 0x1234);

			Json::Value* u32 = root->AddUInt32("u32", 0x89ABCDEF);
			Assert::IsTrue(u32->type == Json::ValueType::Int && u32->value.AsUint32 == 0x89ABCDEF);

			Json::Value* u64 = root->AddUInt64("u64", 0x123456789ABCDEF0ULL);
			Assert::IsTrue(u64->type == Json::ValueType::Int && u64->value.AsInt == 0x123456789ABCDEF0ULL);

			Json::Value* f = root->AddFloat("f", 1.5f);
			Assert::IsTrue(f->type == Json::ValueType::Float && f->value.AsFloat == 1.5f);

			Json::Value* n = root->AddNull("n");
			Assert::IsTrue(n->type == Json::ValueType::Null);

			Json::Value* t = root->AddBool("t", true);
			Assert::IsTrue(t->type == Json::ValueType::Bool && t->value.AsBool == true);

			Json::Value* s = root->AddString("s", L"wide");
			Assert::IsTrue(s->type == Json::ValueType::String && wcscmp(s->value.AsString, L"wide") == 0);

			Json::Value* as = root->AddAnsiString("as", "ansi");
			Assert::IsTrue(as->type == Json::ValueType::String && wcscmp(as->value.AsString, L"ansi") == 0);

			Json::Value* obj = root->AddObject("obj");
			Assert::IsTrue(obj->type == Json::ValueType::Object);

			Json::Value* arr = root->AddArray("arr");
			Assert::IsTrue(arr->type == Json::ValueType::Array);

			// ByName lookups
			Assert::IsTrue(root->ByName("i") == i);
			Assert::IsTrue(root->ByName("u16") == u16);
			Assert::IsTrue(root->ByName("u32") == u32);
			Assert::IsTrue(root->ByName("u64") == u64);
			Assert::IsTrue(root->ByName("f") == f);
			Assert::IsTrue(root->ByName("n") == n);
			Assert::IsTrue(root->ByName("t") == t);
			Assert::IsTrue(root->ByName("s") == s);
			Assert::IsTrue(root->ByName("as") == as);
			Assert::IsTrue(root->ByName("obj") == obj);
			Assert::IsTrue(root->ByName("arr") == arr);

			// Missing name
			Assert::IsTrue(root->ByName("missing") == nullptr);

			// ByType
			Assert::IsTrue(root->ByType(Json::ValueType::Int) == i);
			Assert::IsTrue(root->ByType(Json::ValueType::Float) == f);
			Assert::IsTrue(root->ByType(Json::ValueType::Null) == n);
			Assert::IsTrue(root->ByType(Json::ValueType::Bool) == t);
			Assert::IsTrue(root->ByType(Json::ValueType::String) == s);
			Assert::IsTrue(root->ByType(Json::ValueType::Object) == obj);
			Assert::IsTrue(root->ByType(Json::ValueType::Array) == arr);
			Assert::IsTrue(root->ByType(Json::ValueType::Unknown) == nullptr);
		}

		/// <summary>
		/// SetName / ReplaceString / AddValue / Add / Replace.
		/// </summary>
		TEST_METHOD(TestValueModification)
		{
			Json json;
			Json::Value* root = json.root.AddObject("root");

			// SetName
			Json::Value* v = root->AddInt("old", 1);
			v->SetName("new");
			Assert::IsTrue(v->name != nullptr && strcmp(v->name, "new") == 0);
			Assert::IsTrue(root->ByName("old") == nullptr);
			Assert::IsTrue(root->ByName("new") == v);

			// ReplaceString
			Json::Value* s = root->AddString("s", L"old text");
			s->ReplaceString(L"new text");
			Assert::IsTrue(wcscmp(s->value.AsString, L"new text") == 0);

			// AddValue (add an existing value under a new name)
			Json::Value* detached = new Json::Value();
			detached->type = Json::ValueType::Int;
			detached->value.AsInt = 999;
			Json::Value* added = root->AddValue("detached", detached);
			Assert::IsTrue(added == detached);
			Assert::IsTrue(root->ByName("detached") == detached);
			Assert::IsTrue(detached->value.AsInt == 999);

			// Add (deep copy of a value with children)
			Json src;
			Json::Value* srcRoot = src.root.AddObject("srcRoot");
			srcRoot->AddInt("a", 11);
			Json::Value* srcArr = srcRoot->AddArray("b");
			srcArr->AddInt(nullptr, 5);

			Json::Value* copied = root->Add(root, srcRoot);
			Assert::IsTrue(copied->type == Json::ValueType::Object);
			Assert::IsTrue(copied != srcRoot);
			root->children.push_back(copied);		// Adopt so the destructor cleans it up
			Json::Value* copiedA = copied->ByName("a");
			Assert::IsTrue(copiedA != nullptr && copiedA->value.AsInt == 11);
			Json::Value* copiedB = copied->ByName("b");
			Assert::IsTrue(copiedB != nullptr && copiedB->type == Json::ValueType::Array && copiedB->children.size() == 1);
			Assert::IsTrue(copiedB->children.front()->value.AsInt == 5);

			// Replace: value with the same name is updated in place, a new one is added
			Json::Value* other = srcRoot->ByName("a");
			Json::Value* replaced1 = root->Replace(root, other);
			Assert::IsTrue(replaced1 != nullptr);	// New child "a" created
			root->children.push_back(replaced1);
			Assert::IsTrue(root->ByName("a")->value.AsInt == 11);

			Json::Value* replaced2 = root->Replace(root, other);
			Assert::IsTrue(replaced2 == nullptr);	// Existing child "a" updated in place
			Assert::IsTrue(root->ByName("a")->value.AsInt == 11);

			Json::Value* other2 = srcRoot->ByName("b");
			Json::Value* added2 = root->Replace(root, other2);
			Assert::IsTrue(added2 != nullptr);	// New child
			root->children.push_back(added2);
			Assert::IsTrue(root->ByName("b")->type == Json::ValueType::Array);
		}

		/// <summary>
		/// Json::Clone and Json::Merge.
		/// </summary>
		TEST_METHOD(TestCloneAndMerge)
		{
			// Clone

			Json src;
			Json::Value* srcRoot = src.root.AddObject("srcRoot");
			srcRoot->AddInt("a", 1);
			srcRoot->AddInt("b", 2);

			Json dst;
			dst.Clone(&src);
			Assert::IsTrue(dst.root.children.size() == 1);
			Json::Value* dstRoot = dst.root.children.front();
			Assert::IsTrue(dstRoot != srcRoot);
			Assert::IsTrue(dstRoot->ByName("a")->value.AsInt == 1);
			Assert::IsTrue(dstRoot->ByName("b")->value.AsInt == 2);

			// Modifying the clone must not affect the source
			dstRoot->ByName("a")->value.AsInt = 100;
			Assert::IsTrue(srcRoot->ByName("a")->value.AsInt == 1);

			// Merge: other root is merged into this root.
			// The keys "a" exists -> replaced, "c" is new -> added.

			Json mergeSrc;
			Json::Value* mRoot = mergeSrc.root.AddObject("mergeRoot");
			mRoot->AddInt("a", 50);
			mRoot->AddInt("c", 60);

			Json target;
			Json::Value* tRoot = target.root.AddObject("mergeRoot");
			tRoot->AddInt("a", 1);
			tRoot->AddInt("b", 2);

			target.Merge(&mergeSrc);

			Json::Value* tRoot2 = target.root.children.front();
			Assert::IsTrue(tRoot2->ByName("a")->value.AsInt == 50);
			Assert::IsTrue(tRoot2->ByName("b")->value.AsInt == 2);
			Assert::IsTrue(tRoot2->ByName("c")->value.AsInt == 60);
		}

		/// <summary>
		/// Deserialize malformed text must throw an exception.
		/// </summary>
		TEST_METHOD(TestDeserializeErrors)
		{
			// Wrong syntax
			Json json;
			bool thrown = false;
			try
			{
				const char* text = "{ \"a\" : }";
				json.Deserialize((void*)text, strlen(text));
			}
			catch (...)
			{
				thrown = true;
			}
			Assert::IsTrue(thrown);

			// Unknown token
			Json json2;
			thrown = false;
			try
			{
				const char* text = "{ \"a\" : @ }";
				json2.Deserialize((void*)text, strlen(text));
			}
			catch (...)
			{
				thrown = true;
			}
			Assert::IsTrue(thrown);
		}

		/// <summary>
		/// Serializing an empty Json produces an empty text.
		/// </summary>
		TEST_METHOD(TestSerializeEmpty)
		{
			Json json;
			size_t actualSize = 0;
			json.GetSerializedTextSize(nullptr, 0x1000, actualSize);
			Assert::IsTrue(actualSize == 0);

			uint8_t buf[0x1000]{};
			json.Serialize(buf, sizeof(buf), actualSize);
			Assert::IsTrue(actualSize == 0);
		}
	};
}
