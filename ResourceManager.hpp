#pragma once

#include "Resource.hpp"

class ResourceManager
{
    public:
    ResourceManager() {
        adress=new Resource;
    }
    ~ResourceManager() {
        delete adress; 
    }
    double get() {
        return resource->get();
    }
    private:
    Resource* adress;
};
