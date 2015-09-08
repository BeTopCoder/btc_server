#include "PBHeader.hpp"

NAMESPACE_BEGIN(network)

/**
* @brief 构造函数
* @notes: 无
* @author: stone-jin
* @Time:   2015/09/08
* @Email:  1520006273@qq.com
*/
PBHeader::PBHeader()
    : pMsgHeader(new BYTE[1024], [](BYTE *pTmp) {delete[] pTmp;})
{

}

/**
* @brief 析构函数
* @notes: 无
* @author: stone-jin
* @Time:   2015/09/08
* @Email:  1520006273@qq.com
*/
PBHeader::~PBHeader()
{
}

NAMESPACE_END(network)
