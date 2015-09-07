#ifndef __SESSION_MGR_H__
#define __SESSION_MGR_H__

#include "session.h"
#include <set>

namespace http
{
    namespace server
    {
        class sessionMgr
        {
        public:
            sessionMgr(const sessionMgr &) = delete;
            sessionMgr& operator=(const sessionMgr&) = delete;

            explicit sessionMgr();

            void start(session_ptr session_);

            void stop(session_ptr session_);

            void stop_all();
        protected:
        private:
            std::set<session_ptr> m_sessions_;
        };
    }
}

#endif

