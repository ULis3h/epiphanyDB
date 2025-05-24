#ifndef EPIPHANY_FILE_MANAGER_H_
#define EPIPHANY_FILE_MANAGER_H_

#include <map>
#include <iostream>

#include "file.hpp"
#include "allocator.hpp"

namespace epiphany {

using os::EpiphanyFile;

typedef int FileID;

struct FileInfo {
    std::string file_path;
    EpiphanyFile* base;
};

class FileManager {
public:
    FileManager();
    ~FileManager();

    FileID registerFile(const std::string& file_path);
    bool unregisterFile(FileID file_id);

    bool openFile(FileID file_id, const std::string& mode);
    void closeFile(FileID file_id);
    size_t readFile(FileID file_id, char* buffer, size_t size);
    size_t writeFile(FileID file_id, const char* buffer, size_t size);
    bool isFileOpen(FileID file_id);
    int seekFile(FileID file_id, long offset, int origin);

private:
    std::map<FileID, FileInfo*, 
    epiphany::common::Allocator<std::pair<FileID, FileInfo*> > > file_map;
    
    FileID next_file_id;
    FileID generateFileId();
};





} /// epiphany.

#endif //! EPIPHANY_FILE_MANAGER_H_