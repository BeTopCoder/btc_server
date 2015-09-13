#include <iostream>
#include <memory>
#include <string>
#include <map>
#include <set>
#include <list>
#include <sstream>
#include <boost/noncopyable.hpp>
#include <boost/enable_shared_from_this.hpp>
using namespace std;

#include "libpq-fe.h"

namespace dbproxy
{
    /**
    * @brief 数据库结果类
    * @notes: 无
    * @author: stone-jin
    * @Time:   2015/09/13
    */
    class CPGDBResult
        : public boost::enable_shared_from_this<CPGDBResult>
        , public boost::noncopyable
    {
    public:
        /**
        * @brief 构造函数
        * @notes: 无
        * @author: stone-jin
        * @Time:   2015/09/13
        * @Email:  1520006273@qq.com
        */
        explicit CPGDBResult(std::shared_ptr<PGresult> pResult_)
            : m_pResult_(pResult_)
            , iCurrentRow(0)
        {
            int nFiled = PQnfields(m_pResult_.get());
            for (int index = 0; index < nFiled; index++)
            {
                m_key_map_.insert(make_pair(PQfname(m_pResult_.get(), index), index));
            }
            iMaxRow = PQntuples(m_pResult_.get());
        }

        bool next()
        {
            iCurrentRow++;
            if (iCurrentRow >= iMaxRow)
            {
                return false;
            }
            return true;
        }

        int getInt(string &strKey)
        {
            map<string, int>::iterator iter = m_key_map_.find(strKey);
            if (m_key_map_.end() == iter)   /* 没有找到 */
            {
                throw std::exception("Fail to find key");
                return 0;
            }
            else    /* 找到了 */
            {
                return atoi(PQgetvalue(m_pResult_.get(), iCurrentRow, iter->second));
            }
        }

        string getStr(string &strKey)
        {
            map<string, int>::iterator iter = m_key_map_.find(strKey);
            if (m_key_map_.end() == iter)   /* 没有找到 */
            {
                throw std::exception("Fail to find key");
                return string("");
            }
            else
            {
                return string(PQgetvalue(m_pResult_.get(), iCurrentRow, iter->second));
            }
        }

        void print()
        {
            int nFiled = PQnfields(m_pResult_.get());
            for (int index = 0; index < nFiled; index++)
            {
                cout << PQfname(m_pResult_.get(), index) << "   ";
            }
            cout << endl;
            int nTuples = PQntuples(m_pResult_.get());
            for (int index = 0; index < nTuples; index++)
            {
                for (int j = 0; j < nFiled; j++)
                {
                    cout << PQgetvalue(m_pResult_.get(), index, j);
                }
                cout << endl;
            }
        }
    protected:
    private:
        std::shared_ptr<PGresult> m_pResult_;

        std::map<string, int> m_key_map_;
        int iCurrentRow;
        int iMaxRow;
    };

    typedef std::shared_ptr<CPGDBResult> CPGDBResult_Ptr;

    class CPGDBPool;
    class CPGDB
        : public boost::enable_shared_from_this<CPGDB>
        , public boost::noncopyable
    {
    public:
        /**
        * @brief 构造函数
        * @notes: 无
        * @author: stone-jin
        * @Time:   2015/09/13
        * @Email:  1520006273@qq.com
        */
        CPGDB(CPGDBPool *pPGDBPool_)
            : m_pPGDBPool_(pPGDBPool_)
        {
        }

        /**
        * @brief 初始化
        * @notes:
        * @author: stone-jin
        * @Time:   2015/09/13
        * @Email:  1520006273@qq.com
        */
        void Init()
        {
            PGconn_.reset(PQconnectdb("user=postgres password=passwd dbname=postgres"), [](PGconn *pTmp) {PQfinish(pTmp);});
            if (!PGconn_)
            {
                throw std::exception("PQconnectdb failed");
            }

            /* 检查后端连接成功建立 */
            if (CONNECTION_OK != PQstatus(PGconn_.get()))
            {
                throw std::exception("failed to connect db");
            }
        }

        /**
        * @brief 执行SQL语句
        * @notes: 无
        * @author: stone-jin
        * @Time:   2015/09/13
        * @Email:  1520006273@qq.com
        */
        void excuteSQL(const string &strSql)
        {
            std::shared_ptr<PGresult> pResult(PQexec(PGconn_.get(), strSql.c_str()), [](PGresult *pTmp) {PQclear(pTmp);});

            if (!pResult)
            {
                throw std::exception("Fail to PQexec.");
            }

            if (PGRES_COMMAND_OK != PQresultStatus(pResult.get()))
            {
                throw std::exception("Fail to exceteUpdate");
            }
        }

        /**
        * @brief 查询操作
        * @notes: 无
        * @author: stone-jin
        * @Time:   2015/09/13
        * @Email:  1520006273@qq.com
        */
        CPGDBResult_Ptr excuteQuery(const string& strSql)
        {
            std::shared_ptr<PGresult> pResult(PQexec(PGconn_.get(), strSql.c_str()), [](PGresult *pTmp) {PQclear(pTmp);});

            if (!pResult)
            {
                throw std::exception("Fail to PQexec");
            }

            if (PGRES_TUPLES_OK != PQresultStatus(pResult.get()))
            {
                throw std::exception("Fail to PQresultStatus");
            }

            std::shared_ptr<CPGDBResult> p(new CPGDBResult(pResult));

            return p;
        }
    protected:
    private:
        CPGDBPool *m_pPGDBPool_;

        std::shared_ptr<PGconn> PGconn_;    /* 数据库句柄的智能指针 */
    };

    typedef std::shared_ptr<CPGDB> CPGDB_Ptr;

    /**
    * @brief 数据库连接池
    * @notes: 无
    * @author: stone-jin
    * @Time:   2015/09/13
    */
    class CPGDBPool
        : public boost::enable_shared_from_this<CPGDB>
        , public boost::noncopyable
    {
    public:
        /**
        * @brief 构造函数
        * @notes: 无
        * @author: stone-jin
        * @Time:   2015/09/13
        * @Email:  1520006273@qq.com
        */
        CPGDBPool(int num)
        {
            for (int index = 0; index < num; index++)
            {
                CPGDB_Ptr pgdb_(new CPGDB(this));
                m_pgdbs_.push_back(pgdb_);
            }
        }

        /**
        * @brief 获取数据库的智能指针
        * @notes: 无
        * @author: stone-jin
        * @Time:   2015/09/13
        * @Email:  1520006273@qq.com
        */
        CPGDB_Ptr getDB()
        {
            for (auto db_ : m_pgdbs_)
            {
                return db_;
            }
            return NULL;
        }
    protected:
    private:
        /* 管理数据库类 */
        std::list<CPGDB_Ptr> m_pgdbs_;
    };

    typedef std::shared_ptr<CPGDBPool> CPGDBPool_Ptr;

    /**
    * @brief 数据池管理类，一般一个写池，一个读池
    * @notes:
    * @author: stone-jin
    * @Time:   2015/09/13
    */
    class CPGDbPoolManager
        : public enable_shared_from_this<CPGDbPoolManager>
        , public boost::noncopyable
    {
    public:
        /**
        * @brief 初始化
        * @notes: 无
        * @author: stone-jin
        * @Time:   2015/09/13
        * @Email:  1520006273@qq.com
        */
        void init()
        {
            CPGDBPool_Ptr readPool_(new CPGDBPool(1));
            CPGDBPool_Ptr writePool_(new CPGDBPool(1));
            if ((!readPool_) || (!writePool_))
            {
                throw std::exception("Fail to init");
            }
            m_PGDBPools_.insert(std::make_pair("read", readPool_));
            m_PGDBPools_.insert(std::make_pair("write", writePool_));
        }

        /**
        * @brief 根据数据库连接池的名字从数据库连接池中获取数据库对象
        * @notes: 无
        * @author: stone-jin
        * @Time:   2015/09/13
        * @Email:  1520006273@qq.com
        */
        CPGDB_Ptr getDB(std::string &str_)
        {
            std::map<string, CPGDBPool_Ptr>::iterator iter = m_PGDBPools_.find(str_);
            if (m_PGDBPools_.end() == iter)
            {
                return NULL;
            }
            else
            {
                return iter->second->getDB();
            }
        }

    protected:
    private:

        /* 数据库连接池集合 */
        std::map<string, CPGDBPool_Ptr> m_PGDBPools_;
    };

    /**
    * @brief 获取数据库连接池管理器的全局变量
    * @notes: 无
    * @author: stone-jin
    * @Time:   2015/09/13
    * @Email:  1520006273@qq.com
    */
    template<class DBPoolManager>
    DBPoolManager& CDBPoolManager_single()
    {
        static DBPoolManager dbpoolManager_;
        return dbpoolManager_;
    }
}



int main(int argc, char** argv)
{
    using namespace dbproxy;
    try
    {
        CDBPoolManager_single<CPGDbPoolManager>().init();
        CDBPoolManager_single<CPGDbPoolManager>().getDB(string("read"))->Init();
        CDBPoolManager_single<CPGDbPoolManager>().getDB(string("write"))->Init();
        //CDBPoolManager_single<CPGDbPoolManager>().getDB(string("write"))->excuteSQL(string("CREATE TABLE C(C INT);"));
        CPGDBResult_Ptr t = CDBPoolManager_single<CPGDbPoolManager>().getDB(string("read"))->excuteQuery(string("SELECT * FROM C;"));
        t->print();
    }
    catch (std::exception &ec)
    {
        cout << ec.what() << endl;
    }
    catch (...)
    {
        cout << "sorry" << endl;
    }
    getchar();
    return 0;
}