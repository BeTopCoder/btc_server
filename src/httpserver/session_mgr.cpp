#include "session_mgr.h"

namespace http
{
    namespace server
    {
        sessionMgr::sessionMgr()
        {
        }

        void sessionMgr::start(session_ptr session_)
        {
            m_sessions_.insert(session_);
            session_->start();
            return;
        }

        void sessionMgr::stop(session_ptr session_)
        {
            session_->stop();
            m_sessions_.erase(session_);
        }

        void sessionMgr::stop_all()
        {
            for (auto c : m_sessions_)
            {
                c->stop();
            }
            m_sessions_.clear();
        }

    }
}
