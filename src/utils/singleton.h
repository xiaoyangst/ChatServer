#ifndef HV_SINGLETON_H_
#define HV_SINGLETON_H_

#include <mutex>

// 删除拷贝语义
#define DISABLE_COPY(Class) \
    Class(const Class&) = delete; \
    Class& operator=(const Class&) = delete;

// 放在需要单例的头文件中
#define SINGLETON_DECL(Class) \
    public: \
        static Class* instance(); \
        static void exitInstance(); \
    private: \
        DISABLE_COPY(Class) \
        static Class* s_pInstance; \
        static std::once_flag s_initFlag; \
        static std::mutex s_mutex;

// 放在需要单例的源文件中
#define SINGLETON_IMPL(Class) \
    Class* Class::s_pInstance = NULL; \
    std::once_flag Class::s_initFlag; \
    std::mutex Class::s_mutex; \
    Class* Class::instance() { \
        std::call_once(s_initFlag, []() {s_pInstance = new Class;}); \
	    return s_pInstance; \
    } \
    void Class::exitInstance() { \
        std::lock_guard<std::mutex> lock(s_mutex); \
        if (s_pInstance) { \
            delete s_pInstance; \
            s_pInstance = nullptr; \
        } \
    }

#endif // HV_SINGLETON_H_
