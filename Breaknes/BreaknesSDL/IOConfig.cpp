#include "pch.h"

#include "IOConfig.h"

#include "../../Common/JsonLib/Json.h"
#include "../../IO/IO.h"

namespace BreaknesSDL
{
	namespace
	{
		const char* const kConfigDescr = "This JSON contains a list of IO devices and their Attach/Detach statuses with Motherboard and Bindings with Input API.";

		/// <summary>
		/// wchar_t -> UTF-8 (wchar_t is UTF-16 on Windows and UTF-32 on Linux).
		/// </summary>
		std::string WideToUTF8(const wchar_t* str)
		{
			std::string out;
			if (str == nullptr)
				return out;

			for (const wchar_t* p = str; *p; p++)
			{
				uint32_t cp = (uint32_t)*p;

				// UTF-16 surrogate pair (Windows)
				if (cp >= 0xD800 && cp <= 0xDBFF && p[1] >= 0xDC00 && p[1] <= 0xDFFF)
				{
					cp = 0x10000 + ((cp - 0xD800) << 10) + ((uint32_t)p[1] - 0xDC00);
					p++;
				}

				if (cp < 0x80)
				{
					out += (char)cp;
				}
				else if (cp < 0x800)
				{
					out += (char)(0xC0 | (cp >> 6));
					out += (char)(0x80 | (cp & 0x3F));
				}
				else if (cp < 0x10000)
				{
					out += (char)(0xE0 | (cp >> 12));
					out += (char)(0x80 | ((cp >> 6) & 0x3F));
					out += (char)(0x80 | (cp & 0x3F));
				}
				else
				{
					out += (char)(0xF0 | (cp >> 18));
					out += (char)(0x80 | ((cp >> 12) & 0x3F));
					out += (char)(0x80 | ((cp >> 6) & 0x3F));
					out += (char)(0x80 | (cp & 0x3F));
				}
			}
			return out;
		}

		/// <summary>
		/// UTF-8 -> wchar_t (UTF-16 with surrogate pairs on Windows, direct codepoints on Linux).
		/// </summary>
		std::wstring UTF8ToWide(const std::string& str)
		{
			std::wstring out;
			size_t i = 0;

			while (i < str.size())
			{
				uint8_t c = (uint8_t)str[i];
				uint32_t cp = 0;
				int extra = 0;

				if (c < 0x80)
				{
					cp = c;
				}
				else if ((c & 0xE0) == 0xC0)
				{
					cp = c & 0x1F;
					extra = 1;
				}
				else if ((c & 0xF0) == 0xE0)
				{
					cp = c & 0x0F;
					extra = 2;
				}
				else if ((c & 0xF8) == 0xF0)
				{
					cp = c & 0x07;
					extra = 3;
				}
				else
				{
					i++;	// invalid byte, skip
					continue;
				}

				i++;

				bool ok = true;
				for (int e = 0; e < extra; e++)
				{
					if (i >= str.size() || ((uint8_t)str[i] & 0xC0) != 0x80)
					{
						ok = false;
						break;
					}
					cp = (cp << 6) | ((uint8_t)str[i] & 0x3F);
					i++;
				}

				if (!ok)
					continue;

				if (cp >= 0x10000)
				{
					uint32_t v = cp - 0x10000;
					out += (wchar_t)(0xD800 + (v >> 10));
					out += (wchar_t)(0xDC00 + (v & 0x3FF));
				}
				else
				{
					out += (wchar_t)cp;
				}
			}
			return out;
		}

		/// <summary>
		/// Read the string value of a JSON member. Returns false if the member is missing
		/// or is not a string.
		/// </summary>
		bool GetStringValue(Json::Value* obj, const char* member_name, std::string& out)
		{
			if (obj == nullptr)
				return false;

			Json::Value* value = obj->ByName(member_name);
			if (value == nullptr || value->type != Json::ValueType::String)
				return false;

			out = WideToUTF8(value->value.AsString);
			return true;
		}

		/// <summary>
		/// Read the integer value of a JSON member. Returns false if the member is missing
		/// or is not an integer.
		/// </summary>
		bool GetIntValue(Json::Value* obj, const char* member_name, int& out)
		{
			if (obj == nullptr)
				return false;

			Json::Value* value = obj->ByName(member_name);
			if (value == nullptr || value->type != Json::ValueType::Int)
				return false;

			out = (int)value->value.AsInt;
			return true;
		}
	}

	bool IOConfigManager::Load(const char* path, IOConfig& config)
	{
		config = IOConfig();
		config.descr = kConfigDescr;

		FILE* f = fopen(path, "rb");
		if (f == nullptr)
			return false;

		fseek(f, 0, SEEK_END);
		long file_size = ftell(f);
		fseek(f, 0, SEEK_SET);

		if (file_size <= 0)
		{
			fclose(f);
			return false;
		}

		std::string text;
		text.resize((size_t)file_size);

		size_t readed = fread(&text[0], 1, (size_t)file_size, f);
		fclose(f);

		if (readed != (size_t)file_size)
			return false;

		try
		{
			Json json;
			json.Deserialize((void*)text.c_str(), text.size());

			if (json.root.children.empty())
				return false;

			Json::Value* root = json.root.children.front();

			// The description lives in the "info" member, like in the managed application.
			Json::Value* info_value = root->ByName("info");
			GetStringValue(info_value, "descr", config.descr);

			Json::Value* devices_value = root->ByName("devices");
			if (devices_value != nullptr && devices_value->type == Json::ValueType::Array)
			{
				for (auto it = devices_value->children.begin(); it != devices_value->children.end(); ++it)
				{
					Json::Value* dev = *it;
					IOConfigDevice device;

					int device_id = 0;
					if (GetIntValue(dev, "device_id", device_id))
					{
						device.device_id = (uint32_t)device_id;
					}

					GetStringValue(dev, "name", device.name);

					Json::Value* attached_value = dev->ByName("attached");
					if (attached_value != nullptr && attached_value->type == Json::ValueType::Array)
					{
						for (auto pit = attached_value->children.begin(); pit != attached_value->children.end(); ++pit)
						{
							Json::Value* port_value = *pit;
							IOConfigPort port;

							GetStringValue(port_value, "board", port.board);
							GetIntValue(port_value, "port", port.port);

							device.attached.push_back(port);
						}
					}

					Json::Value* bindings_value = dev->ByName("bindings");
					if (bindings_value != nullptr && bindings_value->type == Json::ValueType::Array)
					{
						for (auto bit = bindings_value->children.begin(); bit != bindings_value->children.end(); ++bit)
						{
							Json::Value* binding_value = *bit;
							IOConfigBinding binding;

							GetIntValue(binding_value, "actuator_id", binding.actuator_id);
							GetStringValue(binding_value, "binding", binding.binding);

							device.bindings.push_back(binding);
						}
					}

					config.devices.push_back(device);
				}
			}
		}
		catch (...)
		{
			return false;
		}

		return true;
	}

	bool IOConfigManager::Save(const IOConfig& config, const char* path)
	{
		Json json;
		Json::Value* root = json.root.AddObject(nullptr);

		Json::Value* info = root->AddObject("info");
		info->AddString("descr", UTF8ToWide(config.descr.empty() ? kConfigDescr : config.descr).c_str());

		Json::Value* devices_value = root->AddArray("devices");

		for (auto it = config.devices.begin(); it != config.devices.end(); ++it)
		{
			const IOConfigDevice& device = *it;

			Json::Value* dev = devices_value->AddObject(nullptr);
			dev->AddUInt32("device_id", device.device_id);
			dev->AddString("name", UTF8ToWide(device.name).c_str());

			Json::Value* attached_value = dev->AddArray("attached");
			for (auto pit = device.attached.begin(); pit != device.attached.end(); ++pit)
			{
				const IOConfigPort& port = *pit;

				Json::Value* port_value = attached_value->AddObject(nullptr);
				port_value->AddString("board", UTF8ToWide(port.board).c_str());
				port_value->AddInt("port", port.port);
			}

			Json::Value* bindings_value = dev->AddArray("bindings");
			for (auto bit = device.bindings.begin(); bit != device.bindings.end(); ++bit)
			{
				const IOConfigBinding& binding = *bit;

				Json::Value* binding_value = bindings_value->AddObject(nullptr);
				binding_value->AddInt("actuator_id", binding.actuator_id);
				binding_value->AddString("binding", UTF8ToWide(binding.binding).c_str());
			}
		}

		try
		{
			// Two-pass serialization: first compute the size, then emit the text.
			// JsonLib asserts that the passed max size is larger than the result,
			// so the sizing pass gets a generous cap (the settings file is small).

			const size_t MaxJsonSize = 0x100000;	// 1 MB

			size_t text_size = 0;
			json.GetSerializedTextSize(nullptr, MaxJsonSize, text_size);

			std::string text;
			text.resize(text_size + 1, 0);

			size_t actual_size = 0;
			json.Serialize(&text[0], MaxJsonSize, actual_size);

			FILE* f = fopen(path, "wb");
			if (f == nullptr)
				return false;

			size_t written = fwrite(text.c_str(), 1, actual_size, f);
			fclose(f);

			return written == actual_size;
		}
		catch (...)
		{
			return false;
		}
	}

	std::string IOConfigManager::DeviceIDToString(uint32_t device_id)
	{
		switch ((IO::DeviceID)device_id)
		{
			case IO::DeviceID::FamiController_1: return "Famicom Controller (Port1)";
			case IO::DeviceID::FamiController_2: return "Famicom Controller (Port2)";
			case IO::DeviceID::NESController: return "NES Controller";
			case IO::DeviceID::DendyController: return "Dendy Turbo Controller";
			case IO::DeviceID::VirtualFamiController_1: return "Virtual Famicom Controller (Port1)";
			case IO::DeviceID::VirtualFamiController_2: return "Virtual Famicom Controller (Port2)";
			case IO::DeviceID::VirtualNESController: return "Virtual NES Controller";
			case IO::DeviceID::VirtualDendyController: return "Virtual Dendy Turbo Controller";
		}
		return "Unknown";
	}

	std::string IOConfigManager::GetDefaultDeviceName(uint32_t device_id)
	{
		return DeviceIDToString(device_id);
	}
}
