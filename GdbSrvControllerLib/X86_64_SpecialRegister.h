#pragma once
#include <string>
#include <map>
#include "GdbSrvControllerLib.h" 
 
	using namespace GdbSrvControllerLib;
	void QRcmdRegistor(GdbSrvController* ctx, const std::string& input, std::map<std::string, std::string>& maps);

	//void QRcmdRegistor2(GdbSrvController* ctx, const std::string& input, std::map<std::string, std::string>& maps);
	void QuerySpecialRegistor(GdbSrvController* ctx, std::map<std::string, std::string>& maps);
	auto QRcmdRegistorSegment(GdbSrvController* ctx, const std::string& input)
		-> std::pair<uint64_t, uint64_t>;
 