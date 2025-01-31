#pragma once
#include "../arch.h"
#include <array>
#include <string_view>
#include <windef.h>

#if __has_include(<winrt/TranslucentTB.Xaml.Models.Primitives.h>)
#define HAS_WINRT_CONFIG
#include "../winrt.hpp"
#include <winrt/TranslucentTB.Xaml.Models.Primitives.h>
#endif

#if __has_include(<rapidjson/rapidjson.h>)
#define HAS_RAPIDJSON
#include <rapidjson/document.h>
#include <rapidjson/encodings.h>
#include "rapidjsonhelper.hpp"
#endif

#include "../undoc/user32.hpp"
#include "../util/color.hpp"
#include "../win32.hpp"

struct TaskbarAppearance {
	ACCENT_STATE Accent = ACCENT_NORMAL;
	Util::Color Color = { 0, 0, 0, 0 };
	bool ShowPeek = true;
	bool ShowLine = true;
	float BlurRadius = 9.0f;

	constexpr TaskbarAppearance() noexcept = default;
	constexpr TaskbarAppearance(ACCENT_STATE accent, Util::Color color, bool showPeek, bool showLine, float blurRadius) noexcept :
		Accent(accent),
		Color(color),
		ShowPeek(showPeek),
		ShowLine(showLine),
		BlurRadius(blurRadius)
	{ }

#ifdef HAS_RAPIDJSON
	template<class Writer>
	inline void Serialize(Writer &writer) const
	{
		rjh::Serialize(writer, Accent, ACCENT_KEY, ACCENT_MAP);
		rjh::Serialize(writer, Color.ToString(), COLOR_KEY);
		rjh::Serialize(writer, ShowPeek, SHOW_PEEK_KEY);
		rjh::Serialize(writer, ShowLine, SHOW_LINE_KEY);
		rjh::Serialize(writer, BlurRadius, RADIUS_KEY);
	}

	void Deserialize(const rjh::value_t &obj, void (*unknownKeyCallback)(std::wstring_view))
	{
		rjh::EnsureType(rj::Type::kObjectType, obj.GetType(), L"root node");

		for (auto it = obj.MemberBegin(); it != obj.MemberEnd(); ++it)
		{
			rjh::EnsureType(rj::Type::kStringType, it->name.GetType(), L"member name");

			InnerDeserialize(rjh::ValueToStringView(it->name), it->value, unknownKeyCallback);
		}
	}

protected:
	void InnerDeserialize(std::wstring_view key, const rjh::value_t &val, void (*unknownKeyCallback)(std::wstring_view))
	{
		if (key == ACCENT_KEY)
		{
			rjh::Deserialize(val, Accent, key, ACCENT_MAP);
		}
		else if (key == COLOR_KEY)
		{
			rjh::EnsureType(rj::Type::kStringType, val.GetType(), key);

			const auto colorStr = rjh::ValueToStringView(val);
			try
			{
				Color = Util::Color::FromString(colorStr);
			}
			catch (...)
			{
				throw rjh::DeserializationError {
					std::format(L"Found invalid string \"{}\" while deserializing {}", colorStr, key)
				};
			}
		}
		else if (key == SHOW_PEEK_KEY)
		{
			rjh::Deserialize(val, ShowPeek, key);
		}
		else if (key == SHOW_LINE_KEY)
		{
			rjh::Deserialize(val, ShowLine, key);
		}
		else if (key == RADIUS_KEY)
		{
			rjh::Deserialize(val, BlurRadius, key);

			// internal Direct2D limitation
			if (BlurRadius > 750)
			{
				BlurRadius = 750;
			}
		}
		else if (unknownKeyCallback)
		{
			unknownKeyCallback(key);
		}
	}

private:
	static constexpr std::array<std::wstring_view, 5> ACCENT_MAP = {
		L"normal",
		L"opaque",
		L"clear",
		L"blur",
		L"acrylic"
	};

	static constexpr std::wstring_view ACCENT_KEY = L"accent";
	static constexpr std::wstring_view COLOR_KEY = L"color";
	static constexpr std::wstring_view SHOW_PEEK_KEY = L"show_peek";
	static constexpr std::wstring_view SHOW_LINE_KEY = L"show_line";
	static constexpr std::wstring_view RADIUS_KEY = L"blur_radius";
#endif

#ifdef HAS_WINRT_CONFIG
public:
	TaskbarAppearance(const txmp::TaskbarAppearance &winrtObj) noexcept :
		Accent(static_cast<ACCENT_STATE>(winrtObj.Accent())),
		Color(winrtObj.Color()),
		ShowPeek(winrtObj.ShowPeek()),
		ShowLine(winrtObj.ShowLine()),
		BlurRadius(winrtObj.BlurRadius())
	{ }

	operator txmp::TaskbarAppearance() const
	{
		return { static_cast<txmp::AccentState>(Accent), Color, ShowPeek, ShowLine, BlurRadius };
	}
#endif
};
