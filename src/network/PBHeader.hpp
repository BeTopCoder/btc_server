#ifndef __PB_HEADER__
#define __PB_HEADER__

#include "common.hpp"
#include <memory>

NAMESPACE_BEGIN(network)

const static UCHAR UC_MAIN_VERSION    = 0;    /* ��ǰ���汾�� */
const static UCHAR UC_SUB_VERSION     = 0;    /* ��ǰ�ΰ汾�� */

/**
 * @brief PBHeader��Ϣͷ������
 * @notes:  final��
 * @author: stone-jin
 * @Time:   2015/09/08
 * @Email: 1520006273@qq.com
 */
class DLL_EXPORT PBHeader final
{
public:
    /**
     * @brief ���캯��
     * @notes: ��
     * @author: stone-jin
     * @Time:   2015/09/08
     * @Email:  1520006273@qq.com
     */
    PBHeader();

    /**
     * @brief ��������
     * @notes: ��
     * @author: stone-jin
     * @Time:   2015/09/08
     * @Email:  1520006273@qq.com
     */
    ~PBHeader();


public:

protected:
private:
    ULONG_32    m_ulMsgHeadLen = 0;                 /* Э��ͷ�ֽڳ��ȣ�����Э��ͷ����չ */
    ULONG_32    m_ulMsgBodyLen = 0;                 /* Э�����ݳ��� */
    UCHAR       m_ucMainVer    = UC_MAIN_VERSION;   /* Э�����汾�� */
    UCHAR       m_ucSubVer     = UC_SUB_VERSION;    /* Э��ΰ汾�� */
    UCHAR       m_ucByteOrder  = 0;                 /* �ֽ��� */
    UCHAR       m_ucReverse    = 0;                 /* ���� */
    ULONG_32    m_ulMsgType    = 0;                 /* ��Ϣ���� */
    ULONG_32    m_ulMsgID      = 0;                 /* ��ϢID */
    ULONG_32    m_ulMsgSeq     = 0;                 /* ��Ϣ��� */

    std::shared_ptr<BYTE> pMsgHeader;
};

NAMESPACE_END(network)

#endif
