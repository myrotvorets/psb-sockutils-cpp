#ifndef D2E11104_0197_4897_8423_C244B19354AA
#define D2E11104_0197_4897_8423_C244B19354AA

#ifdef PSB_SOCKUTILS_STATIC_DEFINE
#    define PSB_SOCKUTILS_EXPORT
#    define PSB_SOCKUTILS_NO_EXPORT
#else
#    ifdef psb_sockutils_EXPORTS
#        if defined _WIN32 || defined __CYGWIN__
#            define PSB_SOCKUTILS_EXPORT __declspec(dllexport)
#            define PSB_SOCKUTILS_NO_EXPORT
#        else
#            define PSB_SOCKUTILS_EXPORT    [[gnu::visibility("default")]]
#            define PSB_SOCKUTILS_NO_EXPORT [[gnu::visibility("hidden")]]
#        endif
#    else
#        if defined _WIN32 || defined __CYGWIN__
#            define PSB_SOCKUTILS_EXPORT __declspec(dllimport)
#            define PSB_SOCKUTILS_NO_EXPORT
#        else
#            define PSB_SOCKUTILS_EXPORT    [[gnu::visibility("default")]]
#            define PSB_SOCKUTILS_NO_EXPORT [[gnu::visibility("hidden")]]
#        endif
#    endif
#endif

#endif /* D2E11104_0197_4897_8423_C244B19354AA */
