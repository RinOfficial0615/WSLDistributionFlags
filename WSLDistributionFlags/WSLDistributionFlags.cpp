/*
 * Copyright Rin Rikka 2025 (@RinOfficial0615)
 */

#include <Windows.h>
#include <wslapi.h>

#include <iostream>
#include <exception>
#include <format>

// #pragma comment(lib, "Wslapi.lib")
// Microsoft where is your wslapi.lib??
class WSLApi
{
public:
	static WSLApi &GetInstance()
	{
		static WSLApi instance;
		return instance;
	}

	auto IsDistributionRegistered(
		_In_ PCWSTR distributionName
	) const
	{
		return IsDistributionRegistered_(distributionName);
	}

	auto ConfigureDistribution(
		_In_ PCWSTR distributionName,
		_In_ ULONG defaultUID,
		_In_ WSL_DISTRIBUTION_FLAGS wslDistributionFlags
	) const
	{
		return ConfigureDistribution_(distributionName, defaultUID, wslDistributionFlags);
	}

	auto GetDistributionConfiguration(
		_In_ PCWSTR distributionName,
		_Out_ ULONG* distributionVersion,
		_Out_ ULONG* defaultUID,
		_Out_ WSL_DISTRIBUTION_FLAGS* wslDistributionFlags,
		_Outptr_result_buffer_(*defaultEnvironmentVariableCount) PSTR** defaultEnvironmentVariables,
		_Out_ ULONG* defaultEnvironmentVariableCount
	) const
	{
		return GetDistributionConfiguration_(distributionName, distributionVersion, defaultUID, wslDistributionFlags, defaultEnvironmentVariables, defaultEnvironmentVariableCount);
	}

private:
	WSLApi()
	{
		auto wslapi_dll = LoadLibraryExW(L"wslapi.dll", NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);
		if (!wslapi_dll)
			throw std::runtime_error("Failed to load wslapi.dll!");

		auto get_func = [&wslapi_dll](const CHAR* func_name) -> auto
		{
			auto hFunc = GetProcAddress(wslapi_dll, func_name);
			if (!hFunc)
				throw std::runtime_error(std::format("Failed to resolve wslapi.dll!{}", func_name));
			return hFunc;
		};

		IsDistributionRegistered_ = reinterpret_cast<pIsDistributionRegistered_>(get_func("WslIsDistributionRegistered"));
		ConfigureDistribution_ = reinterpret_cast<pConfigureDistribution_>(get_func("WslConfigureDistribution"));
		GetDistributionConfiguration_ = reinterpret_cast<pGetDistributionConfiguration_>(get_func("WslGetDistributionConfiguration"));
	}

	typedef BOOL(*pIsDistributionRegistered_)(
		_In_ PCWSTR distributionName
		);
	pIsDistributionRegistered_ IsDistributionRegistered_ = nullptr;

	typedef HRESULT(*pConfigureDistribution_)(
			_In_ PCWSTR distributionName,
			_In_ ULONG defaultUID,
			_In_ WSL_DISTRIBUTION_FLAGS wslDistributionFlags
		);
	pConfigureDistribution_ ConfigureDistribution_ = nullptr;

	typedef HRESULT(*pGetDistributionConfiguration_)(
			_In_ PCWSTR distributionName,
			_Out_ ULONG* distributionVersion,
			_Out_ ULONG* defaultUID,
			_Out_ WSL_DISTRIBUTION_FLAGS* wslDistributionFlags,
			_Outptr_result_buffer_(*defaultEnvironmentVariableCount) PSTR** defaultEnvironmentVariables,
			_Out_ ULONG* defaultEnvironmentVariableCount
		);
	pGetDistributionConfiguration_ GetDistributionConfiguration_ = nullptr;
};

static auto GetWSLDistributionFlags(const PCWSTR distribution_name)
{
	auto &wsl_api = WSLApi::GetInstance();

	if (!wsl_api.IsDistributionRegistered(distribution_name))
		throw std::invalid_argument("Invalid distribution name!");
	
	ULONG distribution_version{};
	ULONG default_uid{};
	WSL_DISTRIBUTION_FLAGS wsl_distribution_flags{};
	PSTR *default_environment_variables{};
	ULONG default_environment_variable_count{};

	if (wsl_api.GetDistributionConfiguration(
		distribution_name,
		&distribution_version,
		&default_uid,
		&wsl_distribution_flags,
		&default_environment_variables,
		&default_environment_variable_count) != S_OK)
		throw std::runtime_error("Failed to get distribution configuration!");

	return std::make_pair<>(default_uid, wsl_distribution_flags);
}

static void SetWSLDistributionFlags(
	const PCWSTR distribution_name,
	const ULONG default_uid,
	const WSL_DISTRIBUTION_FLAGS dist_flags)
{
	auto &wsl_api = WSLApi::GetInstance();

	if (!wsl_api.IsDistributionRegistered(distribution_name)) // just in case
		throw std::invalid_argument("Invalid distribution name!");

	if (wsl_api.ConfigureDistribution(
		distribution_name,
		default_uid,
		dist_flags) != S_OK)
		throw std::runtime_error("Failed to set distribution configuration!");
}

int main()
{
	system("wsl -l --all");

	std::cout << "> WSL Distribution name?" << std::endl;
	std::wstring distribution_name{}; 
	//distribution_name = L"Ubuntu-24.04";
	std::wcin >> distribution_name;

	WSL_DISTRIBUTION_FLAGS wsl_dist_flags{};
	ULONG wsl_default_uid{};

	try
	{
		auto dist_flags = GetWSLDistributionFlags(distribution_name.c_str());
		wsl_default_uid = dist_flags.first;
		wsl_dist_flags = dist_flags.second;
	}
	catch (std::exception ex)
	{
		std::cout << ex.what() << std::endl;
		return -1;
	}

	std::cout << "WSL Distribution flags:" << std::endl;

	// bool wsl_flag_none = wsl_dist_flags == WSL_DISTRIBUTION_FLAGS_NONE;
	bool wsl_enable_interop = wsl_dist_flags & WSL_DISTRIBUTION_FLAGS_ENABLE_INTEROP;
	bool wsl_append_nt_path = wsl_dist_flags & WSL_DISTRIBUTION_FLAGS_APPEND_NT_PATH;
	bool wsl_enable_drive_mounting = wsl_dist_flags & WSL_DISTRIBUTION_FLAGS_ENABLE_DRIVE_MOUNTING;
	//if (wsl_flag_none)
	//{
	//	std::cout << "\t<NONE>" << std::endl;
	//}
	//else
	//{
	std::cout << "\tEnable interop: " << (wsl_enable_interop ? "true" : "false") << std::endl;
	std::cout << "\tAppend NT path: " << (wsl_append_nt_path ? "true" : "false") << std::endl;
	std::cout << "\tEnable drive mounting: " << (wsl_enable_drive_mounting ? "true" : "false") << std::endl;
	//}

	std::cout << "> Change \"enable interop\" to enable(1) / disable(0) ?" << std::endl;
	bool opt_enable_interop = false;
	std::cin >> opt_enable_interop;

	std::cout << "> Change \"append NT path\" to enable(1) / disable(0) ?" << std::endl;
	bool opt_append_nt_path = false;
	std::cin >> opt_append_nt_path;

	std::cout << "> Change \"enable drive mounting\" to enable(1) / disable(0) ?" << std::endl;
	bool opt_enable_drive_mounting = false;
	std::cin >> opt_enable_drive_mounting;

	// if (wsl_flag_none == false && (opt_enable_interop || opt_append_nt_path || opt_enable_drive_mounting) == false)
	//	wsl_dist_flags = WSL_DISTRIBUTION_FLAGS_NONE;

	auto change_flag_mask = [&wsl_dist_flags](bool wsl_val, bool opt_val, auto flag_mask) -> void
	{
		if (wsl_val == true && opt_val == false)
			wsl_dist_flags &= ~flag_mask; // remove flag mask
		else if (wsl_val == false && opt_val == true)
			wsl_dist_flags |= flag_mask; // add flag mask
	};

	change_flag_mask(wsl_enable_interop, opt_enable_interop, WSL_DISTRIBUTION_FLAGS_ENABLE_INTEROP);
	change_flag_mask(wsl_append_nt_path, opt_append_nt_path, WSL_DISTRIBUTION_FLAGS_APPEND_NT_PATH);
	change_flag_mask(wsl_enable_drive_mounting, opt_enable_drive_mounting, WSL_DISTRIBUTION_FLAGS_ENABLE_DRIVE_MOUNTING);

	try
	{
		SetWSLDistributionFlags(distribution_name.c_str(), wsl_default_uid, wsl_dist_flags);
	}
	catch (std::exception ex)
	{
		std::cout << ex.what() << std::endl;
		return -1;
	}

	std::cout << "Success!";
	return 0;
}