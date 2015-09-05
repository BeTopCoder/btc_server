#ifndef __REPLY_H__
#define __REPLY_H__

namespace http
{
    namespace server
    {
        class reply
        {
        public:
            reply(const reply&) = delete;
            reply& operator=(const reply&) = delete;

            reply();
        protected:
        private:

        };
    }
}

#endif
