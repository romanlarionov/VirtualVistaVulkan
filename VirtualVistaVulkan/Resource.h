
#ifndef VIRTUALVISTA_RESOURCE_H
#define VIRTUALVISTA_RESOURCE_H

#include <string>

namespace vv
{
    typedef std::string Handle;

    class Resource
    {
        friend class ResourceManager;

    public:
		Resource() {};
        virtual ~Resource() {}

    private:
    
    protected:
		Handle handle_;
		std::string file_path_;
		std::string file_name_;

    };
}

#endif // VIRTUALVISTA_RESOURCE_H