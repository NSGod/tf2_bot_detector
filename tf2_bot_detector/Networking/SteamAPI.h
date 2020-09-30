#pragma once

#include "Bitmap.h"
#include "Clock.h"
#include "SteamID.h"

#include <mh/error/error_code_exception.hpp>
#include <nlohmann/json_fwd.hpp>

#include <future>
#include <optional>
#include <string>
#include <unordered_set>
#include <variant>
#include <vector>

namespace tf2_bot_detector
{
	class HTTPClient;
}

namespace tf2_bot_detector::SteamAPI
{
	enum class ErrorCode
	{
		Success,

		EmptyState,
		InfoPrivate,
		GameNotOwned,
		UnexpectedDataFormat,
		GenericHttpError,
		JSONParseError,
		InvalidSteamID,
		EmptyAPIKey,
	};

	struct SteamAPIError : mh::error_condition_exception, std::nested_exception
	{
		SteamAPIError(const mh::source_location& location, std::error_condition code,
			const std::string_view& detail = {});

		mh::source_location m_SourceLocation;
	};

	std::error_condition make_error_condition(tf2_bot_detector::SteamAPI::ErrorCode e);
}

namespace std
{
	template<> struct is_error_condition_enum<tf2_bot_detector::SteamAPI::ErrorCode> : std::bool_constant<true> {};
}

namespace tf2_bot_detector::SteamAPI
{
	enum class CommunityVisibilityState
	{
		Private = 1,
		FriendsOnly = 2,
		Public = 3,
	};

	enum class PersonaState
	{
		Offline = 0,
		Online = 1,
		Busy = 2,
		Away = 3,
		Snooze = 4,
		LookingToTrade = 5,
		LookingToPlay = 6,
	};

	enum class AvatarQuality
	{
		Small,
		Medium,
		Large,
	};

	struct PlayerSummary
	{
		SteamID m_SteamID;
		std::string m_RealName;
		std::string m_Nickname;
		std::string m_AvatarHash;
		std::string m_ProfileURL;
		PersonaState m_Status{};
		CommunityVisibilityState m_Visibility{};
		bool m_ProfileConfigured = false;
		bool m_CommentPermissions = false;
		std::optional<time_point_t> m_CreationTime;
		std::optional<time_point_t> m_LastLogOff;

		std::optional<duration_t> GetAccountAge() const;

		std::string GetAvatarURL(AvatarQuality quality = AvatarQuality::Large) const;
		std::shared_future<Bitmap> GetAvatarBitmap(const HTTPClient* client,
			AvatarQuality quality = AvatarQuality::Large) const;

		std::string_view GetVanityURL() const;
	};
	void from_json(const nlohmann::json& j, PlayerSummary& d);

	std::future<std::vector<PlayerSummary>> GetPlayerSummariesAsync(
		const std::string_view& apikey, const std::vector<SteamID>& steamIDs, const HTTPClient& client);

	enum class PlayerEconomyBan
	{
		None,
		Probation,
		Banned,

		Unknown,
	};

	struct PlayerBans
	{
		SteamID m_SteamID;
		bool m_CommunityBanned = false;
		PlayerEconomyBan m_EconomyBan{};
		unsigned m_VACBanCount = 0;
		unsigned m_GameBanCount = 0;
		duration_t m_TimeSinceLastBan{};
	};
	void from_json(const nlohmann::json& j, PlayerBans& d);

	std::shared_future<std::vector<PlayerBans>> GetPlayerBansAsync(
		const std::string_view& apikey, const std::vector<SteamID>& steamIDs, const HTTPClient& client);

	std::future<duration_t> GetTF2PlaytimeAsync(const std::string_view& apikey,
		const SteamID& steamID, const HTTPClient& client);

	std::future<std::unordered_set<SteamID>> GetFriendList(const std::string_view& apikey,
		const SteamID& steamID, const HTTPClient& client);
}
