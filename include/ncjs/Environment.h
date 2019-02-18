
/***************************************************************
 * Name:      Environment.h
 * Purpose:   Defines Node-CEF Environment Class
 * Author:    Joshua GPBeta (studiocghibli@gmail.com)
 * Created:   2016-05-26
 * Copyright: Studio GPBeta (www.gpbeta.com)
 * License:
 **************************************************************/
 
#ifndef NCJS_ENVIRONMENT_H
#define NCJS_ENVIRONMENT_H

typedef struct uv_loop_s uv_loop_t;

/// ----------------------------------------------------------------------------
/// Headers
/// ----------------------------------------------------------------------------

#include <include/cef_v8.h>

namespace ncjs {

class EventLoop;

/// ----------------------------------------------------------------------------
/// \class Environment
/// ----------------------------------------------------------------------------
class Environment : public CefBaseRefCounted {

    friend class Core;

    struct Array {
        CefRefPtr<CefV8Value> module_load_list;
        CefRefPtr<CefV8Value> array_buffer_flags;

        Array() : module_load_list(CefV8Value::CreateArray(0)),
                  array_buffer_flags(CefV8Value::CreateArray(0)) {}
    } m_array;

    struct Function {
        CefRefPtr<CefV8Value> op_new;
        CefRefPtr<CefV8Value> op_throw;
        CefRefPtr<CefV8Value> new_error;
        CefRefPtr<CefV8Value> ctor_fs_stats;

        Function() {}
    } m_function;

    struct Object {
        CefRefPtr<CefV8Value> process;
        CefRefPtr<CefV8Value> binding_cache;
        CefRefPtr<CefV8Value> ptype_buffer;

        Object() : binding_cache(CefV8Value::CreateObject(NULL, NULL)) {}
    } m_object;

public:

	class Listener : public CefBaseRefCounted {
    public:
        virtual void OnContextReleased(CefRefPtr<CefV8Context> context) = 0;
    };

    class BufferObjectInfo { // original: ArrayBufferAllocatorInfo
        friend class Environment;
        enum { NO_ZERO_FILL, FIELDS_COUNT };
    public:
        unsigned* Fields() { return m_fields; }
        int FieldsCount() const { return FIELDS_COUNT; };
        bool NoZeroFill() const { return m_fields[NO_ZERO_FILL] != 0; }
        void ResetFillFlag() { m_fields[NO_ZERO_FILL] = 0; }
    private:
        BufferObjectInfo();
        unsigned m_fields[FIELDS_COUNT];
    };

    Array&       GetArray() { return m_array; }
    Function& GetFunction() { return m_function; }
    Object&     GetObject() { return m_object; }

    const CefString& GetFrameUrl() const { return m_urlFrame; }
    const CefString& GetPagePath() const { return m_pathPage; }
    const CefString& GetExecPath() const { return m_pathExec; }

    BufferObjectInfo& GetBufferObjectInfo() { return m_infoBufferObject; }

    CefRefPtr<CefV8Value> New(const CefRefPtr<CefV8Value>& obj,
                              const CefV8ValueList& args)
    {
        return m_function.op_new->ExecuteFunction(obj, args);
    }

    bool AddListener(const CefRefPtr<Listener>& listener)
    {
        m_listener.push_back(listener);

        return true;
    }

    bool RemoveListener(const CefRefPtr<Listener>& listener)
    {
        for (ListenerList::iterator it = m_listener.begin(); it != m_listener.end(); ++it) {
            if (*it == listener) {
                m_listener.erase(it);
                return true;
            }
        }
        return false;
    }

    void Setup(const CefString& execPath, const CefString& pagePath,
               const CefString& frameUrl, CefRefPtr<CefV8Value> process);

    /// Static Functions
    /// --------------------------------------------------------------

    enum Endianness { LITTLE_ENDIAN, BIG_ENDIAN };

    static bool IsLE() { return GetEndianness() == LITTLE_ENDIAN; }
    static bool IsBE() { return GetEndianness() == BIG_ENDIAN; }

    static enum Endianness GetEndianness() {
#ifndef CEF_INCLUDE_BASE_CEF_BUILD_H_
#error cef_build.h required
#endif
#if ARCH_CPU_LITTLE_ENDIAN
        return LITTLE_ENDIAN;
#else
        return BIG_ENDIAN;
#endif
    }

    static EventLoop& GetAsyncLoop() { return s_loopAsync; }
    static uv_loop_t*  GetSyncLoop() { return s_loopSync; }

    static double GetProcessStartTime() { return s_startTime; }

    template <unsigned N>
    static void ErrorException(const cef_char_t (&msg)[N], CefString& except)
    {
        except.FromString(msg, N, false);
    }

    static void ErrorException(int err, const char* syscall,
                               const char* msg, CefString& except);

    template <unsigned N>
    static void RangeException(const cef_char_t (&msg)[N], CefString& except)
    {
        except.FromString(msg, N, false);
    }

    template <unsigned N>
    static void TypeException(const cef_char_t (&msg)[N], CefString& except)
    {
        except.FromString(msg, N, false);
    }
    
    static void UvException(int err, const char* syscall, CefString& except)
    {
        UvException(err, syscall, NULL, NULL, NULL, except);
    }

    static void UvException(int err, const char* syscall, const char* msg,
                            const char* path, const char* dest, CefString& except);

    static Environment* Get(const CefRefPtr<CefV8Context>& context);
    static Environment* Create(CefRefPtr<CefV8Context> context);
    static void InvalidateContext(const CefRefPtr<CefV8Context>& context);

private:

    typedef std::vector< CefRefPtr<Listener> > ListenerList;
    typedef std::map< CefRefPtr<CefV8Context>, CefRefPtr<Environment> > EnvMap;

    static bool Initialize();
    static void Shutdown();

    /// Utilities Functions
    /// --------------------------------------------------------------

    static bool FindEnvironment(const CefRefPtr<CefV8Context>& context, EnvMap::iterator& it);

    /// Constructors & Destructor
    /// --------------------------------------------------------------

    Environment();
    ~Environment();

    /// Declarations
    /// -----------------

    ListenerList m_listener;

    BufferObjectInfo m_infoBufferObject;
    
    CefString m_pathExec;
    CefString m_pathPage;
    CefString m_urlFrame;

    static uv_loop_t* s_loopSync;
    static EventLoop s_loopAsync;

    static const double s_startTime;

    static EnvMap s_map;

    IMPLEMENT_REFCOUNTING(Environment);
};


} // ncjs

#endif // NCJS_ENVIRONMENT_H
