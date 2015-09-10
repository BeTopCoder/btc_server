#ifndef __PB_HEADER__
#define __PB_HEADER__

#include "common.hpp"
#include <memory>

NAMESPACE_BEGIN(network)

const static UCHAR UC_MAIN_VERSION    = 0;    /* 当前主版本号 */
const static UCHAR UC_SUB_VERSION     = 0;    /* 当前次版本号 */

/**
 * @brief PBHeader消息头定义类
 * @notes:  final类
 * @author: stone-jin
 * @Time:   2015/09/08
 * @Email: 1520006273@qq.com
 */
class DLL_EXPORT PBHeader final
{
public:
    /**
     * @brief 构造函数
     * @notes: 无
     * @author: stone-jin
     * @Time:   2015/09/08
     * @Email:  1520006273@qq.com
     */
    PBHeader();

    /**
     * @brief 析构函数
     * @notes: 无
     * @author: stone-jin
     * @Time:   2015/09/08
     * @Email:  1520006273@qq.com
     */
    ~PBHeader();


public:

protected:
private:
    ULONG_32    m_ulMsgHeadLen = 0;                 /* 协议头字节长度，用于协议头的扩展 */
    ULONG_32    m_ulMsgBodyLen = 0;                 /* 协议内容长度 */
    UCHAR       m_ucMainVer    = UC_MAIN_VERSION;   /* 协议主版本号 */
    UCHAR       m_ucSubVer     = UC_SUB_VERSION;    /* 协议次版本号 */
    UCHAR       m_ucByteOrder  = 0;                 /* 字节序 */
    UCHAR       m_ucReverse    = 0;                 /* 保留 */
    ULONG_32    m_ulMsgType    = 0;                 /* 消息类型 */
    ULONG_32    m_ulMsgID      = 0;                 /* 消息ID */
    ULONG_32    m_ulMsgSeq     = 0;                 /* 消息序号 */

    std::shared_ptr<BYTE> pMsgHeader;
};

NAMESPACE_END(network)

#endif
