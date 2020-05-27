#pragma once

class FileSystemAgent
{
private:
    
    
public:
    FileSystemAgent();
    virtual ~FileSystemAgent() = 0;
};



class FileSystemServerAgent: public FileSystemAgent
{
private:
    
public:
    FileSystemServerAgent();
    ~FileSystemServerAgent();
};


class FileSystemClientAgent: public FileSystemAgent
{
private:
    
public:
    FileSystemClientAgent();
    ~FileSystemClientAgent();
};

