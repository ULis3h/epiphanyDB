#ifndef EPIPHANY_CONFIG_HPP_
#define EPIPHANY_CONFIG_HPP_

#include <string>

namespace epiphany {

// 内部配置实现类
class ConfigImpl {
public:
    ConfigImpl(const char* home, int mode);

    ~ConfigImpl();

    std::string home_;      // 数据库主目录
    int mode_; // 打开模式
    size_t cache_size_;     // 缓存大小
    size_t page_size_;      // 页面大小
    bool read_only_;        // 是否只读模式
};

} // namespace epiphany

#endif // EPIPHANY_CONFIG_HPP_
