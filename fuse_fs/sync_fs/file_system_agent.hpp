#pragma once

class FileSystemAgent
{
private:
    
    
public:
    FileSystemAgent();
    virtual ~FileSystemAgent() = 0;
};

FileSystemAgent::FileSystemAgent()
{
}

FileSystemAgent::~FileSystemAgent()
{
}


class FileSystemServerAgent: public FileSystemAgent
{
private:
    
public:
    FileSystemServerAgent();
    ~FileSystemServerAgent();
};

FileSystemServerAgent::FileSystemServerAgent()
{
}

FileSystemServerAgent::~FileSystemServerAgent()
{
}


class FileSystemClientAgent: public FileSystemAgent
{
private:
    
public:
    FileSystemClientAgent();
    ~FileSystemClientAgent();
};

FileSystemClientAgent::FileSystemClientAgent()
{
}

FileSystemClientAgent::~FileSystemClientAgent()
{
}
