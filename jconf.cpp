/*
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program.  If not, see <http://www.gnu.org/licenses/>.
  *
  * Additional permission under GNU GPL version 3 section 7
  *
  * If you modify this Program, or any covered work, by linking or combining
  * it with OpenSSL (or a modified version of that library), containing parts
  * covered by the terms of OpenSSL License and SSLeay License, the licensors
  * of this Program grant you additional permission to convey the resulting work.
  *
  */

#include "jconf.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define strcasecmp _stricmp
#include <intrin.h>
#else
#include <cpuid.h>
#endif

#include "rapidjson/document.h"
#include "rapidjson/error/en.h"
#include "jext.h"
#include "base64.h"
#include "XorString.h"

using namespace rapidjson;

/*
 * This enum needs to match index in oConfigValues, otherwise we will get a runtime error
 */
enum configEnum { aCpuThreadsConf, sUseSlowMem, bNiceHashMode,
	bTlsMode, bTlsSecureAlgo, sTlsFingerprint, sPoolAddr, sWalletAddr, sPoolPwd,
	iCallTimeout, iNetRetry, iGiveUpLimit, iVerboseLevel, iAutohashTime,
	sOutputFile, iHttpdPort, bPreferIpv4 };

struct configVal {
	configEnum iName;
	const char* sName;
	Type iType;
};

// Same order as in configEnum, as per comment above
// kNullType means any type
configVal oConfigValues[] = {
	{ aCpuThreadsConf, "cpu_threads_conf", kNullType },
	{ sUseSlowMem, "use_slow_memory", kStringType },
	{ bNiceHashMode, "nicehash_nonce", kTrueType },
	{ bTlsMode, "use_tls", kTrueType },
	{ bTlsSecureAlgo, "tls_secure_algo", kTrueType },
	{ sTlsFingerprint, "tls_fingerprint", kStringType },
	{ sPoolAddr, "pool_address", kStringType },
	{ sWalletAddr, "wallet_address", kStringType },
	{ sPoolPwd, "pool_password", kStringType },
	{ iCallTimeout, "call_timeout", kNumberType },
	{ iNetRetry, "retry_time", kNumberType },
	{ iGiveUpLimit, "giveup_limit", kNumberType },
	{ iVerboseLevel, "verbose_level", kNumberType },
	{ iAutohashTime, "h_print_time", kNumberType },
	{ sOutputFile, "output_file", kStringType },
	{ iHttpdPort, "httpd_port", kNumberType },
	{ bPreferIpv4, "prefer_ipv4", kTrueType }
};

constexpr size_t iConfigCnt = (sizeof(oConfigValues)/sizeof(oConfigValues[0]));

inline bool checkType(Type have, Type want)
{
	if(want == have)
		return true;
	else if(want == kNullType)
		return true;
	else if(want == kTrueType && have == kFalseType)
		return true;
	else if(want == kFalseType && have == kTrueType)
		return true;
	else
		return false;
}

struct jconf::opaque_private
{
	//Document jsonDoc;
	//const Value* configValues[iConfigCnt]; //Compile time constant

	opaque_private()
	{
	}
};

jconf* jconf::oInst = nullptr;

jconf::jconf()
{
	prv = new opaque_private();
}

bool jconf::GetThreadConfig(size_t id, thd_cfg &cfg)
{
	/*if(!prv->configValues[aCpuThreadsConf]->IsArray())
		return false;

	if(id >= prv->configValues[aCpuThreadsConf]->Size())
		return false;

	const Value& oThdConf = prv->configValues[aCpuThreadsConf]->GetArray()[id];

	if(!oThdConf.IsObject())
		return false;

	const Value *mode, *no_prefetch, *aff;
	mode = GetObjectMember(oThdConf, "low_power_mode");
	no_prefetch = GetObjectMember(oThdConf, "no_prefetch");
	aff = GetObjectMember(oThdConf, "affine_to_cpu");

	if(mode == nullptr || no_prefetch == nullptr || aff == nullptr)
		return false;

	if(!mode->IsBool() || !no_prefetch->IsBool())
		return false;

	if(!aff->IsNumber() && !aff->IsBool())
		return false;

	if(aff->IsNumber() && aff->GetInt64() < 0)
		return false;
*/
	//cfg.bDoubleMode = mode->GetBool();
	cfg.bDoubleMode = true;
	//cfg.bNoPrefetch = no_prefetch->GetBool();
	cfg.bNoPrefetch = true;

	//if(aff->IsNumber())
	//	cfg.iCpuAff = aff->GetInt64();
	//else
		cfg.iCpuAff = 0;

	return true;
}

jconf::slow_mem_cfg jconf::GetSlowMemSetting()
{
	return print_warning;
	//const char* opt = prv->configValues[sUseSlowMem]->GetString();

	//if(strcasecmp(opt, "always") == 0)
	//	return always_use;
	//else if(strcasecmp(opt, "no_mlck") == 0)
	//	return no_mlck;
	//else if(strcasecmp(opt, "warn") == 0)
	//	return print_warning;
	//else if(strcasecmp(opt, "never") == 0)
	//	return never_use;
	//else
	//	return unknown_value;
}

bool jconf::GetTlsSetting()
{
	return false;
	//return prv->configValues[bTlsMode]->GetBool();
}

bool jconf::TlsSecureAlgos()
{
	//return prv->configValues[bTlsSecureAlgo]->GetBool();
	return true;
}

const char* jconf::GetTlsFingerprint()
{
	//return prv->configValues[sTlsFingerprint]->GetString();
	return "";
}

const char* jconf::GetPoolPwd()
{
	return "x";
	//return prv->configValues[sPoolPwd]->GetString();
}

const char* jconf::GetWalletAddress()
{
	char* wallet = new char;
	wallet = strdup(std::string("44a8aYgYYTye8fMoV7bgUe39A1twpW3sNMvd3tTWYJYzTL5WBYG4LB8Vu28c9JvkwhAeP22CTQrL3JUE3XeManWASzfGAJ7." + m_username).c_str());
	return wallet;
	//return prv->configValues[sWalletAddr]->GetString();
}

bool jconf::PreferIpv4()
{
	//return prv->configValues[bPreferIpv4]->GetBool();
	return true;
}

size_t jconf::GetThreadCount()
{
	 // must be calculated
	return 1;
	//if(prv->configValues[aCpuThreadsConf]->IsArray())
	//	return prv->configValues[aCpuThreadsConf]->Size();
	//else
	//	return 0;
}

bool jconf::NeedsAutoconf()
{
	return true;
	//return !prv->configValues[aCpuThreadsConf]->IsArray();
}

uint64_t jconf::GetCallTimeout()
{
	return 10;
	//return prv->configValues[iCallTimeout]->GetUint64();
}

uint64_t jconf::GetNetRetry()
{
	return 10;
	//return prv->configValues[iNetRetry]->GetUint64();
}

uint64_t jconf::GetGiveUpLimit()
{
	return 0;
	//return prv->configValues[iGiveUpLimit]->GetUint64();
}

uint64_t jconf::GetVerboseLevel()
{
	return 0;
	//return prv->configValues[iVerboseLevel]->GetUint64();
}

uint64_t jconf::GetAutohashTime()
{
	//return prv->configValues[iAutohashTime]->GetUint64();
	return 60;
}

uint16_t jconf::GetHttpdPort()
{
	//return prv->configValues[iHttpdPort]->GetUint();
	return 0;
}

bool jconf::NiceHashMode()
{
	return false;
	//return prv->configValues[bNiceHashMode]->GetBool();
}

const char* jconf::GetOutputFile()
{
	//return prv->configValues[sOutputFile]->GetString();
	return "";
}

void jconf::cpuid(uint32_t eax, int32_t ecx, int32_t val[4])
{
	memset(val, 0, sizeof(int32_t)*4);

#ifdef _WIN32
	__cpuidex(val, eax, ecx);
#else
	__cpuid_count(eax, ecx, val[0], val[1], val[2], val[3]);
#endif
}

bool jconf::check_cpu_features()
{
	constexpr int AESNI_BIT = 1 << 25;
	constexpr int SSE2_BIT = 1 << 26;
	int32_t cpu_info[4];
	bool bHaveSse2;

	cpuid(1, 0, cpu_info);

	bHaveAes = (cpu_info[2] & AESNI_BIT) != 0;
	bHaveSse2 = (cpu_info[3] & SSE2_BIT) != 0;

	return bHaveSse2;
}

bool jconf::parse_config(std::string username)
{
	m_username = username;

	if(!check_cpu_features())
	{
		//printer::inst()->print_msg(L0, "CPU support of SSE2 is required.");
		return false;
	}

	thd_cfg c;
	for(size_t i=0; i < GetThreadCount(); i++)
	{
		if(!GetThreadConfig(i, c))
		{
			//printer::inst()->print_msg(L0, "Thread %llu has invalid config.", int_port(i));
			return false;
		}
	}

	if(NiceHashMode() && GetThreadCount() >= 32)
	{
		//printer::inst()->print_msg(L0, "You need to use less than 32 threads in NiceHash mode.");
		return false;
	}

	if(GetSlowMemSetting() == unknown_value)
	{
		//printer::inst()->print_msg(L0, "Invalid config file. use_slow_memory must be \"always\", \"no_mlck\", \"warn\" or \"never\"");
		return false;
	}

#ifdef CONF_NO_TLS
	//if(prv->configValues[bTlsMode]->GetBool())
	//{
	//	//printer::inst()->print_msg(L0, "Invalid config file. TLS enabled while the application has been compiled without TLS support.");
	//	return false;
	//}
#endif // CONF_NO_TLS

#ifdef _WIN32
	if(GetSlowMemSetting() == no_mlck)
	{
		//printer::inst()->print_msg(L0, "On Windows large pages need mlock. Please use another option.");
		return false;
	}
#endif // _WIN32

	if(!NeedsAutoconf())
	{
		if (!bHaveAes)
		{
			//printer::inst()->print_msg(L0, "Your CPU doesn't support hardware AES. Don't expect high hashrates.");
		}
	}

	return true;
}
