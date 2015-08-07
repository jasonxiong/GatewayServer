#ifndef __CONFIG_ASSEMBLE_HPP__
#define __CONFIG_ASSEMBLE_HPP__

#include "ConfigDefine.hpp"
#include "LotusConfigMng.hpp"


class CConfigAssemble
{
public:
	CConfigAssemble();

public:
	int LoadConfig();
	int ReloadConfig();

	inline CLotusConfigMng& GetLotusConfigMng(){return m_stLotusConfigMng;};

private:

	CLotusConfigMng m_stLotusConfigMng;
};

#endif
